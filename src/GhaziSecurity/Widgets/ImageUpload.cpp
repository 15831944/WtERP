#include "Widgets/ImageUpload.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"

#include <Wt/WFileUpload.h>
#include <Wt/WImage.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WDialog.h>

#include <Wt/WMemoryResource.h>
#include <Wt/WFileResource.h>
#include <web/Configuration.h>

#include <boost/filesystem.hpp>
#include <Magick++.h>

namespace GS
{

	UploadedImage::UploadedImage(Dbo::ptr<UploadedFile> ptr)
		: temporary(false), filePtr(ptr), fileName(ptr->pathToFile()), mimeType(ptr->mimeType), extension(ptr->extension)
	{ }

	ImageUpload::ImageUpload(Wt::WString actionUpload, Wt::WString actionChange)
		: Wt::WTemplate(tr("GS.ImageUpload")), _actionUpload(actionUpload), _actionChange(actionChange)
	{
		setStyleClass("image-upload");
		_fileUpload = bindNew<Wt::WFileUpload>("input");
		bindEmpty("image");
		bindNew<Wt::WText>("action", _actionUpload);

		bool enabled = isEnabled();
		setCondition("is-enabled", enabled);
		if(enabled)
			bindString("label-for", "in" + _fileUpload->id());
		else
			bindEmpty("label-for");

		_tempImage = make_unique<Wt::WImage>(Wt::WLink());
		_image = _tempImage.get();

		_fileUpload->setFilters("image/*");
		_fileUpload->changed().connect(this, &ImageUpload::handleChanged);
		_fileUpload->uploaded().connect(this, &ImageUpload::handleUploaded);
		_fileUpload->fileTooLarge().connect(this, &ImageUpload::handleFileTooLarge);

		auto btn = bindNew<Wt::WPushButton>("button");
		btn->setStyleClass("fa fa-external-link");
		btn->clicked().connect(this, &ImageUpload::viewImage);
	}

	void ImageUpload::handleUploaded()
	{
		if(isEnabled())
			bindString("label-for", "in" + _fileUpload->id());

		if(_fileUpload->uploadedFiles().empty())
			return;

		auto &fileInfo = _fileUpload->uploadedFiles().back();
		boost::filesystem::path filePath(fileInfo.clientFileName());
		if(!filePath.has_extension())
		{
			resolve<Wt::WText*>("action")->setText(tr("NoExtension"));
			return;
		}

		try
		{
			Magick::Blob blob;
			createThumbnail(fileInfo.spoolFileName(), &blob);

			auto memoryResource = make_shared<Wt::WMemoryResource>("image/jpeg");
			memoryResource->setData((const unsigned char*)blob.data(), static_cast<int>(blob.length()));
			_image->setImageLink(Wt::WLink(memoryResource));
			_thumbnailResource = memoryResource;

			_imageInfo.fileName = fileInfo.spoolFileName();
			_imageInfo.mimeType = fileInfo.contentType();
			_imageInfo.extension = filePath.extension().string();
			_imageInfo.temporary = true;
			_imageInfo.filePtr = nullptr;

			resolve<Wt::WText*>("action")->setText(_actionChange);
			setCondition("has-image", true);
			lazyBindImage();
		}
		catch(const Magick::Exception &e)
		{
			Wt::log("warning") << "ImageUpload::handleUploaded(): Error in Magick block:" << e.what();
			WApplication::instance()->showErrorDialog(tr("GMThumbnailError"));
		}
	}

	void ImageUpload::handleFileTooLarge()
	{
		if(isEnabled())
			bindString("label-for", "in" + _fileUpload->id());

		resolve<Wt::WText*>("action")->setText(tr("FileTooLarge").arg(std::round(static_cast<double>(SERVER->configuration().maxRequestSize()) * 100 / 1024 / 1024) / 100));
	}

	void ImageUpload::handleChanged()
	{
		if(!_fileUpload->canUpload())
			return;

		resolve<Wt::WText*>("action")->setText("Uploading...");

		_fileUpload->upload();
		bindEmpty("label-for");
	}

	void ImageUpload::setPlaceholderImageLink(const Wt::WLink &link)
	{
		if(resolveWidget("image"))
			return;

		_image->setImageLink(link);
		bindWidget("image", move(_tempImage));
	}

	void ImageUpload::lazyBindImage()
	{
		if(!resolveWidget("image"))
			bindWidget("image", move(_tempImage));
	}

	void ImageUpload::viewImage()
	{
		if(_imageInfo.fileName.empty())
			return;
		
		if(!_dialog)
		{
			_dialog = addChild(make_unique<Wt::WDialog>());
			_dialog->setClosable(true);
			_dialog->resize(Wt::WLength(95,  Wt::LengthUnit::Percentage), Wt::WLength(95,  Wt::LengthUnit::Percentage));
			_dialog->rejectWhenEscapePressed(true);
			_dialog->setTransient(true);
			_dialog->contents()->setOverflow(Wt::Overflow::Auto);

			if(!_imageResource)
				_imageResource = make_shared<Wt::WFileResource>();
			_imageResource->setFileName(_imageInfo.fileName);
			_imageResource->setMimeType(_imageInfo.mimeType);

			auto img = _dialog->contents()->addNew<Wt::WImage>();
			img->setImageLink(Wt::WLink(_imageResource));
		}

		_dialog->show();
	}

	void ImageUpload::setImageInfo(const UploadedImage &newImageInfo)
	{
		if(_imageInfo.temporary || newImageInfo.temporary)
			return;
		if(_imageInfo.fileName == newImageInfo.fileName && _imageInfo.mimeType == newImageInfo.mimeType && _imageInfo.extension == newImageInfo.extension)
			return;

		_imageInfo = newImageInfo;
		if(_imageInfo.fileName.empty())
		{
			_image->setImageLink(_placeholderLink);
			setCondition("has-image", false);
			return;
		}

		auto thumbFileResource = dynamic_pointer_cast<Wt::WFileResource>(_thumbnailResource);
		if(!thumbFileResource)
			_thumbnailResource = thumbFileResource = make_shared<Wt::WFileResource>();

		boost::filesystem::path path(_imageInfo.fileName);
		boost::filesystem::path thumbnailPath = path.parent_path() / path.stem().concat("_thumb.jpg");

		thumbFileResource->setFileName(thumbnailPath.string());
		thumbFileResource->setMimeType(_imageInfo.mimeType);
		_image->setImageLink(Wt::WLink(thumbFileResource));
		setCondition("has-image", true);
		resolve<Wt::WText*>("action")->setText(_actionChange);
		lazyBindImage();
	}

	bool ImageUpload::saveAndRelocate(Dbo::ptr<Entity> entityPtr, const std::string &description)
	{
		if(!_imageInfo.temporary || !entityPtr || _fileUpload->uploadedFiles().empty())
			return false;

		//Save
		WApplication *app = WApplication::instance();
		TRANSACTION(app);

		if(!_imageInfo.filePtr)
			_imageInfo.filePtr = app->dboSession().addNew<UploadedFile>();

		_imageInfo.filePtr.modify()->description = description;
		_imageInfo.filePtr.modify()->extension = _imageInfo.extension;
		_imageInfo.filePtr.modify()->mimeType = _imageInfo.mimeType;
		_imageInfo.filePtr.modify()->entityPtr = entityPtr;
		_imageInfo.filePtr.flush();
		t.commit();

		//Relocate
		boost::filesystem::path newPath = boost::filesystem::path(_imageInfo.filePtr->pathToDirectory());
		boost::filesystem::path thumbnailFilePath = newPath / (boost::lexical_cast<std::string>(_imageInfo.filePtr.id()) + "_thumb.jpg");
		boost::filesystem::path newFilePath = newPath / (boost::lexical_cast<std::string>(_imageInfo.filePtr.id()) + _imageInfo.extension);

		try
		{
			boost::filesystem::create_directories(newPath);
			boost::filesystem::copy_file(_imageInfo.fileName, newFilePath, boost::filesystem::copy_option::overwrite_if_exists);
			boost::filesystem::remove(_imageInfo.fileName);
			_fileUpload->uploadedFiles().back().stealSpoolFile();

			auto thumbMemoryResource = static_pointer_cast<Wt::WMemoryResource>(_thumbnailResource);
			std::ofstream thumbnailStream(thumbnailFilePath.string(), std::ofstream::binary | std::ofstream::trunc);
			auto thumbnailData = thumbMemoryResource->data();
			thumbnailStream.write((const char*)&thumbnailData[0], thumbnailData.size() * sizeof(unsigned char));
			thumbnailStream.close();

			_imageInfo.temporary = false;
			_imageInfo.fileName = newFilePath.string();

			if(_imageResource)
				_imageResource->setFileName(newFilePath.string());

			shared_ptr<Wt::WFileResource> thumbFileResource;
			_thumbnailResource = thumbFileResource = make_shared<Wt::WFileResource>();
			thumbFileResource->setFileName(thumbnailFilePath.string());
			thumbFileResource->setMimeType("image/jpeg");
			_image->setImageLink(Wt::WLink(_thumbnailResource));
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "Error relocating file " << _imageInfo.fileName << ": " << e.what();
			_imageInfo.filePtr.remove();
			_imageInfo = UploadedImage();
			_image->setImageLink(_placeholderLink);
			app->showErrorDialog(tr("ImageRelocationError"));
			return false;
		}

		return true;
	}

	void ImageUpload::createThumbnail(const std::string &path, Magick::Blob *blob)
	{
		Magick::Image img;
		img.quiet(true);
		img.read(path);
		img.backgroundColor(Magick::Color("white"));
		auto imgSize = img.size();
		img.thumbnail(Magick::Geometry(static_cast<unsigned int>(std::round((double)imgSize.width() / imgSize.height() * _thumbnailHeight)), _thumbnailHeight));
		img.quality(90);
		img.write(blob, "JPEG");
	}

	void ImageUpload::propagateSetEnabled(bool enabled)
	{
		setCondition("is-enabled", enabled);
		if(enabled)
			bindString("label-for", "in" + _fileUpload->id());
		else
			bindEmpty("label-for");
	}

}
