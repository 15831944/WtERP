#ifndef GS_IMAGEUPLOAD_WIDGET_H
#define GS_IMAGEUPLOAD_WIDGET_H

#include "Dbo/Dbos.h"
#include <Wt/WTemplate>
#include <Wt/WLink>

namespace Magick
{
	class Blob;
}

namespace GS
{

	struct UploadedImage
	{
		UploadedImage() = default;
		UploadedImage(Wt::Dbo::ptr<UploadedFile> ptr);
		bool temporary = false;
		std::string fileName, mimeType, extension;
		Wt::Dbo::ptr<UploadedFile> filePtr;
	};

	class ImageUpload : public Wt::WTemplate
	{
	public:
		ImageUpload(Wt::WString actionUpload, Wt::WString actionChange, Wt::WContainerWidget *parent = nullptr);
		void setPlaceholderImageLink(const Wt::WLink &link);
		void setThumbnailHeight(unsigned int height) { _thumbnailHeight = height; }
		void handleUploaded();
		void handleFileTooLarge();
		void handleChanged();
		void viewImage();
		Wt::WFileUpload *fileUpload() const { return _fileUpload; }
		Wt::WImage *image() const { return _image; }
		const UploadedImage &imageInfo() const { return _imageInfo; }
		void setImageInfo(const UploadedImage &imageInfo);
		bool saveAndRelocate(Wt::Dbo::ptr<Entity> entityPtr, const std::string &description = ""); //throws Dbo::Exception

	protected:
		void lazyBindImage();
		void createThumbnail(const std::string &path, Magick::Blob *blob);

		Wt::WString _actionUpload;
		Wt::WString _actionChange;
		Wt::WFileUpload *_fileUpload = nullptr;
		Wt::WImage *_image = nullptr;
		Wt::WFileResource *_imageResource = nullptr;
		Wt::WResource *_thumbnailResource = nullptr;
		Wt::WDialog *_dialog = nullptr;
		unsigned int _thumbnailHeight = 128;
		UploadedImage _imageInfo;
		Wt::WLink _placeholderLink;
	};

}

#endif