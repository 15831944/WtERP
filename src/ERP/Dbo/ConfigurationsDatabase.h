#ifndef ERP_DBODATABASE_CONFIGURATION_H
#define ERP_DBODATABASE_CONFIGURATION_H

#include "Common.h"
#include "Dbo/Configuration.h"

#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>

namespace ERP
{
	class ConfigurationsDatabase
	{
	protected:
		typedef std::unordered_map< std::string, shared_ptr<Ddo::ConfigurationBool> > BoolMap;
		typedef std::unordered_map< std::string, shared_ptr<Ddo::ConfigurationDouble> > DoubleMap;
		typedef std::unordered_map< std::string, shared_ptr<Ddo::ConfigurationEnum> > EnumMap;
		typedef std::unordered_map< std::string, shared_ptr<Ddo::ConfigurationFloat> > FloatMap;
		typedef std::unordered_map< std::string, shared_ptr<Ddo::ConfigurationInt> > IntMap;
		typedef std::unordered_map< std::string, shared_ptr<Ddo::ConfigurationLongInt> > LongIntMap;
		typedef std::unordered_map< std::string, shared_ptr<Ddo::ConfigurationString> > StringMap;

	public:
		ConfigurationsDatabase(DboSession &session);
		void reload() { fetchAll(); }

		shared_ptr<const Ddo::ConfigurationBool> getBoolPtr(const std::string &name) const;
		shared_ptr<const Ddo::ConfigurationDouble> getDoublePtr(const std::string &name) const;
		shared_ptr<const Ddo::ConfigurationEnum> getEnumPtr(const std::string &name) const;
		shared_ptr<const Ddo::ConfigurationFloat> getFloatPtr(const std::string &name) const;
		shared_ptr<const Ddo::ConfigurationInt> getIntPtr(const std::string &name) const;
		shared_ptr<const Ddo::ConfigurationLongInt> getLongIntPtr(const std::string &name) const;
		shared_ptr<const Ddo::ConfigurationString> getStringPtr(const std::string &name) const;

		bool getBool(const std::string &name, bool defaultValue) const;
		double getDouble(const std::string &name, double defaultValue) const;
		int getEnum(const std::string &name, int defaultValue) const;
		float getFloat(const std::string &name, float defaultValue) const;
		int getInt(const std::string &name, int defaultValue) const;
		long long getLongInt(const std::string &name, long long defaultValue) const;
		std::string getStr(const std::string &name, std::string defaultValue = "") const;

		shared_ptr<const Ddo::ConfigurationLongInt> addLongInt(const std::string &name, long long value, DboSession *alternateSession = nullptr);

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
		DboSession &dboSession;

	private:
		mutable boost::shared_mutex _mutex;
	};
}

#endif