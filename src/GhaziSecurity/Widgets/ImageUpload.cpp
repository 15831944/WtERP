#include "Widgets/ImageUpload.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"

#include <Wt/WFileUpload>
#include <Wt/WImage>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WDialog>

#include <Wt/WMemoryResource>
#include <Wt/WFileResource>
#include <web/Configuration.h>

#include <boost/filesystem.hpp>
#include <Magick++.h>

namespace GS
{

	UploadedImage::UploadedImage(Wt::Dbo::ptr<UploadedFile> ptr)
		: filePtr(ptr), temporary(false), fileName(ptr->pathToFile()), mimeType(ptr->mimeType), extension(ptr->extension)
	{ }

	ImageUpload::ImageUpload(Wt::WString actionUpload, Wt::WString actionChange, Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplate(tr("GS.ImageUpload"), parent), _actionUpload(actionUpload), _actionChange(actionChange)
	{
		addFunction("id", &Functions::id);
		setStyleClass("image-upload");
		bindWidget("input", _fileUpload = new Wt::WFileUpload());
		bindEmpty("image");
		bindWidget("action", new Wt::WText(_actionUpload));
		bindString("label-for", "in" + _fileUpload->id());

		_image = new Wt::WImage(Wt::WLink());

		_fileUpload->setFilters("image/*");
		_fileUpload->changed().connect(this, &ImageUpload::handleChanged);
		_fileUpload->uploaded().connect(this, &ImageUpload::handleUploaded);
		_fileUpload->fileTooLarge().connect(this, &ImageUpload::handleFileTooLarge);

		auto btn = new Wt::WPushButton();
		btn->setStyleClass("fa fa-external-link");
		btn->clicked().connect(this, &ImageUpload::viewImage);
		bindWidget("button", btn);
	}

	void ImageUpload::handleUploaded()
	{
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

			Wt::WMemoryResource *thumnailResource = new Wt::WMemoryResource("image/jpeg", this);
			thumnailResource->setData((const unsigned char*)blob.data(), static_cast<int>(blob.length()));
			_image->setImageLink(Wt::WLink(thumnailResource));
			delete _thumbnailResource;
			_thumbnailResource = thumnailResource;

			_imageInfo.fileName = fileInfo.spoolFileName();
			_imageInfo.mimeType = fileInfo.contentType();
			_imageInfo.extension = filePath.extension().string();
			_imageInfo.temporary = true;
			_imageInfo.filePtr = Wt::Dbo::ptr<UploadedFile>();

			resolve<Wt::WText*>("action")->setText(_actionChange);
			setCondition("has-image", true);
			lazyBindImage();
		}
		catch(const Magick::Exception &e)
		{
			Wt::log("warn") << "ImageUpload::handleUploaded(): Error in Magick block:" << e.what();
			WApplication::instance()->showErrorDialog(tr("GMThumbnailError"));
		}
	}

	void ImageUpload::handleFileTooLarge()
	{
		resolve<Wt::WText*>("action")->setText(tr("FileTooLarge").arg(std::round(static_cast<double>(SERVER->configuration().maxRequestSize()) * 100 / 1024 / 1024) / 100));
	}

	void ImageUpload::handleChanged()
	{
		if(!_fileUpload->canUpload())
			return;

		resolve<Wt::WText*>("action")->setText("Uploading...");

		bindEmpty("label-for");
		_fileUpload->upload();
	}

	void ImageUpload::setPlaceholderImageLink(const Wt::WLink &link)
	{
		if(resolveWidget("image"))
			return;

		_image->setImageLink(link);
		bindWidget("image", _image);
	}

	void ImageUpload::lazyBindImage()
	{
		if(!resolveWidget("image"))
			bindWidget("image", _image);
	}

	void ImageUpload::viewImage()
	{
		if(_imageInfo.fileName.empty())
			return;
		
		if(!_dialog)
		{
			_dialog = new Wt::WDialog(this);
			_dialog->setClosable(true);
			_dialog->resize(Wt::WLength(95, Wt::WLength::Percentage), Wt::WLength(95, Wt::WLength::Percentage));
			_dialog->rejectWhenEscapePressed(true);
			_dialog->setTransient(true);
			_dialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);

			if(!_imageResource)
				_imageResource = new Wt::WFileResource(this);
			_imageResource->setFileName(_imageInfo.fileName);
			_imageResource->setMimeType(_imageInfo.mimeType);

			Wt::WImage *img = new Wt::WImage(_dialog->contents());
			img->setImageLink(_imageResource);
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

		auto thumbFileResource = dynamic_cast<Wt::WFileResource*>(_thumbnailResource);
		if(!thumbFileResource)
		{
			delete _thumbnailResource;
			_thumbnailResource = thumbFileResource = new Wt::WFileResource(this);
		}

		boost::filesystem::path path(_imageInfo.fileName);
		boost::filesystem::path thumbnailPath = path.parent_path() / path.stem().concat("_thumb.jpg");

		thumbFileResource->setFileName(thumbnailPath.string());
		thumbFileResource->setMimeType(_imageInfo.mimeType);
		_image->setImageLink(thumbFileResource);
		setCondition("has-image", true);
		resolve<Wt::WText*>("action")->setText(_actionChange);
		lazyBindImage();
	}

	bool ImageUpload::saveAndRelocate(Wt::Dbo::ptr<Entity> entityPtr, const std::string &description /*= ""*/)
	{
		if(!_imageInfo.temporary || !entityPtr || _fileUpload->uploadedFiles().empty())
			return false;

		//Save
		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		if(!_imageInfo.filePtr)
			_imageInfo.filePtr = app->session().add(new UploadedFile);

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

			auto thumbnailResource = dynamic_cast<Wt::WMemoryResource*>(_thumbnailResource);
			std::ofstream thumbnailStream(thumbnailFilePath.string(), std::ofstream::binary | std::ofstream::trunc);
			auto thumbnailData = thumbnailResource->data();
			thumbnailStream.write((const char*)&thumbnailData[0], thumbnailData.size() * sizeof(unsigned char));
			thumbnailStream.close();

			_imageInfo.temporary = false;
			_imageInfo.fileName = newFilePath.string();

			if(_imageResource)
				_imageResource->setFileName(newFilePath.string());

			delete _thumbnailResource;
			Wt::WFileResource *thumbFileResource;
			_thumbnailResource = thumbFileResource = new Wt::WFileResource(this);
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
			APP->showErrorDialog(tr("ImageRelocationError"));
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

}
