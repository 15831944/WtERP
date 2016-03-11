#include "Dbos.h"
#include "Application/WApplication.h"
#include <boost/filesystem/path.hpp>

std::string GS::UploadedFile::pathToFile() const
{
	boost::filesystem::path result(pathToDirectory());
	result /= boost::lexical_cast<std::string>(id()) + extension;
	return result.string();
}

std::string GS::UploadedFile::pathToDirectory() const
{
	auto result = boost::filesystem::path(APP->appRoot()) / "uploads" / boost::lexical_cast<std::string>(entityPtr.id());
	return result.string();
}
