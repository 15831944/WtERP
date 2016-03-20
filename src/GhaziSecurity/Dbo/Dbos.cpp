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

boost::posix_time::ptime GS::addCycleInterval(boost::posix_time::ptime pTime, CycleInterval interval, int nIntervals)
{
	if(interval == DailyInterval)
		pTime += boost::gregorian::days(nIntervals);
	else if(interval == WeeklyInterval)
		pTime += boost::gregorian::weeks(nIntervals);
	else if(interval == MonthlyInterval)
		pTime += boost::gregorian::months(nIntervals);
	else if(interval == YearlyInterval)
		pTime += boost::gregorian::years(nIntervals);

	return pTime;
}
