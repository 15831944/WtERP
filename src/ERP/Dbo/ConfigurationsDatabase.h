#ifndef WW_DBODATABASE_CONFIGURATION_H
#define WW_DBODATABASE_CONFIGURATION_H

#include "Common.h"
#include "Dbo/Configuration.h"

#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>

namespace WW
{
	using namespace std::chrono;

	class ConfigurationsDatabase
	{
	protected:
		typedef std::unordered_map< std::string, Ddo::ptr<Ddo::ConfigurationBool> > BoolMap;
		typedef std::unordered_map< std::string, Ddo::ptr<Ddo::ConfigurationDouble> > DoubleMap;
		typedef std::unordered_map< std::string, Ddo::ptr<Ddo::ConfigurationEnum> > EnumMap;
		typedef std::unordered_map< std::string, Ddo::ptr<Ddo::ConfigurationFloat> > FloatMap;
		typedef std::unordered_map< std::string, Ddo::ptr<Ddo::ConfigurationInt> > IntMap;
		typedef std::unordered_map< std::string, Ddo::ptr<Ddo::ConfigurationLongInt> > LongIntMap;
		typedef std::unordered_map< std::string, Ddo::ptr<Ddo::ConfigurationString> > StringMap;

	public:
		ConfigurationsDatabase(Wt::Dbo::Session &session);
		void reload() { fetchAll(); }

		Ddo::cPtr<Ddo::ConfigurationBool> getBoolPtr(const std::string &name) const;
		Ddo::cPtr<Ddo::ConfigurationDouble> getDoublePtr(const std::string &name) const;
		Ddo::cPtr<Ddo::ConfigurationEnum> getEnumPtr(const std::string &name) const;
		Ddo::cPtr<Ddo::ConfigurationFloat> getFloatPtr(const std::string &name) const;
		Ddo::cPtr<Ddo::ConfigurationInt> getIntPtr(const std::string &name) const;
		Ddo::cPtr<Ddo::ConfigurationLongInt> getLongIntPtr(const std::string &name) const;
		Ddo::cPtr<Ddo::ConfigurationString> getStringPtr(const std::string &name) const;

		bool getBool(const std::string &name, bool defaultValue) const;
		double getDouble(const std::string &name, double defaultValue) const;
		int getEnum(const std::string &name, int defaultValue) const;
		float getFloat(const std::string &name, float defaultValue) const;
		int getInt(const std::string &name, int defaultValue) const;
		long long getLongInt(const std::string &name, long long defaultValue) const;
		std::string getStr(const std::string &name, std::string defaultValue = "") const;

		Ddo::cPtr<Ddo::ConfigurationLongInt> addLongInt(const std::string &name, long long value, Wt::Dbo::Session *alternateSession = nullptr);

		long long getLoadDurationinMS() const;
		std::size_t configurationsCount() const;

	protected:
		void fetchAll();

		BoolMap _boolMap;
		DoubleMap _doubleMap;
		EnumMap _enumMap;
		FloatMap _floatMap;
		IntMap _intMap;
		LongIntMap _longIntMap;
		StringMap _stringMap;

		milliseconds _loadDuration;
		std::size_t _count = 0;
		Wt::Dbo::Session &dboSession;

	private:
		mutable boost::shared_mutex _mutex;
	};
}

#endif