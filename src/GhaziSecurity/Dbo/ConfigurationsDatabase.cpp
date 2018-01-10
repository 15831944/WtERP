#include "Dbo/ConfigurationsDatabase.h"
#include <Wt/WLogger.h>
#include "Application/WServer.h"

namespace WW
{
	using namespace GS;

	ConfigurationsDatabase::ConfigurationsDatabase(Wt::Dbo::Session &session)
		: dboSession(session)
	{
		fetchAll();
	}

	void ConfigurationsDatabase::fetchAll()
	{
		std::lock_guard<std::shared_mutex> lock(_mutex);
		//Time at start
		steady_clock::time_point tpStart = steady_clock::now();

		//Insert into temporary objects first
		BoolMap boolmap;
		DoubleMap doublemap;
		EnumMap enummap;
		FloatMap floatmap;
		IntMap intmap;
		LongIntMap longintmap;
		StringMap stringmap;
		std::size_t count = 0;

		Dbo::BoolCollection boolCollection;
		Dbo::DoubleCollection doubleCollection;
		Dbo::EnumCollection enumCollection;
		Dbo::FloatCollection floatCollection;
		Dbo::IntCollection intCollection;
		Dbo::LongIntCollection longIntCollection;
		Dbo::StringCollection stringCollection;

		//Fetch em all
		Wt::Dbo::Transaction transaction(dboSession);
		boolCollection = dboSession.find<Dbo::ConfigurationBool>();
		doubleCollection = dboSession.find<Dbo::ConfigurationDouble>();
		enumCollection = dboSession.find<Dbo::ConfigurationEnum>();
		floatCollection = dboSession.find<Dbo::ConfigurationFloat>();
		intCollection = dboSession.find<Dbo::ConfigurationInt>();
		longIntCollection = dboSession.find<Dbo::ConfigurationLongInt>();
		stringCollection = dboSession.find<Dbo::ConfigurationString>();

		//Bool
		for(Dbo::ptr<Dbo::ConfigurationBool> &ptr : boolCollection)
		{
			boolmap[ptr.id()->name()] = std::make_shared<Ddo::ConfigurationBool>(ptr);
			_count++;
		}

		//Double
		for(Dbo::ptr<Dbo::ConfigurationDouble> &ptr : doubleCollection)
		{
			doublemap[ptr.id()->name()] = std::make_shared<Ddo::ConfigurationDouble>(ptr);
			_count++;
		}

		//Enum
		for(Dbo::ptr<Dbo::ConfigurationEnum> &ptr : enumCollection)
		{
			enummap[ptr.id()->name()] = std::make_shared<Ddo::ConfigurationEnum>(ptr);
			_count++;
		}

		//Float
		for(Dbo::ptr<Dbo::ConfigurationFloat> &ptr : floatCollection)
		{
			floatmap[ptr.id()->name()] = std::make_shared<Ddo::ConfigurationFloat>(ptr);
			_count++;
		}

		//Int
		for(Dbo::ptr<Dbo::ConfigurationInt> &ptr : intCollection)
		{
			intmap[ptr.id()->name()] = std::make_shared<Ddo::ConfigurationInt>(ptr);
			_count++;
		}

		//LongInt
		for(Dbo::ptr<Dbo::ConfigurationLongInt> &ptr : longIntCollection)
		{
			longintmap[ptr.id()->name()] = std::make_shared<Ddo::ConfigurationLongInt>(ptr);
			_count++;
		}

		//String
		for(Dbo::ptr<Dbo::ConfigurationString> &ptr : stringCollection)
		{
			stringmap[ptr.id()->name()] = std::make_shared<Ddo::ConfigurationString>(ptr);
			_count++;
		}

		transaction.commit();
		_boolMap.swap(boolmap);
		_doubleMap.swap(doublemap);
		_enumMap.swap(enummap);
		_floatMap.swap(floatmap);
		_intMap.swap(intmap);
		_longIntMap.swap(longintmap);
		_stringMap.swap(stringmap);
		std::swap(_count, count);

		//Time at end
		steady_clock::time_point tpEnd = steady_clock::now();
		_loadDuration = duration_cast<milliseconds>(tpEnd - tpStart);

		Wt::log("gs-info") << "ConfigurationsDatabase: " << _count << " entries successfully loaded in " << _loadDuration.count() << " ms";
	}

	Ddo::cPtr<Ddo::ConfigurationBool> ConfigurationsDatabase::getBoolPtr(const std::string &name) const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		BoolMap::const_iterator itr = _boolMap.find(name);
		if(itr == _boolMap.end())
			return Ddo::cPtr<Ddo::ConfigurationBool>();

		return itr->second;
	}

	Ddo::cPtr<Ddo::ConfigurationDouble> ConfigurationsDatabase::getDoublePtr(const std::string &name) const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		DoubleMap::const_iterator itr = _doubleMap.find(name);
		if(itr == _doubleMap.end())
			return Ddo::cPtr<Ddo::ConfigurationDouble>();

		return itr->second;
	}

	Ddo::cPtr<Ddo::ConfigurationEnum> ConfigurationsDatabase::getEnumPtr(const std::string &name) const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		EnumMap::const_iterator itr = _enumMap.find(name);
		if(itr == _enumMap.end())
			return Ddo::cPtr<Ddo::ConfigurationEnum>();

		return itr->second;
	}

	Ddo::cPtr<Ddo::ConfigurationFloat> ConfigurationsDatabase::getFloatPtr(const std::string &name) const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		FloatMap::const_iterator itr = _floatMap.find(name);
		if(itr == _floatMap.end())
			return Ddo::cPtr<Ddo::ConfigurationFloat>();

		return itr->second;
	}

	Ddo::cPtr<Ddo::ConfigurationInt> ConfigurationsDatabase::getIntPtr(const std::string &name) const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		IntMap::const_iterator itr = _intMap.find(name);
		if(itr == _intMap.end())
			return Ddo::cPtr<Ddo::ConfigurationInt>();

		return itr->second;
	}

	Ddo::cPtr<Ddo::ConfigurationLongInt> ConfigurationsDatabase::getLongIntPtr(const std::string &name) const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		LongIntMap::const_iterator itr = _longIntMap.find(name);
		if(itr == _longIntMap.end())
			return Ddo::cPtr<Ddo::ConfigurationLongInt>();

		return itr->second;
	}

	Ddo::cPtr<Ddo::ConfigurationString> ConfigurationsDatabase::getStringPtr(const std::string &name) const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		StringMap::const_iterator itr = _stringMap.find(name);
		if(itr == _stringMap.end())
			return Ddo::cPtr<Ddo::ConfigurationString>();

		return itr->second;
	}

	//Boolean getter
	bool ConfigurationsDatabase::getBool(const std::string &name, bool defaultValue) const
	{
		Ddo::cPtr<Ddo::ConfigurationBool> boolPtr = getBoolPtr(name);
		if(!boolPtr)
		{
			Wt::log("warning") << "BoolPtr not found in ConfigurationsDatabase in GetBool(...). Name: " << name << ", Default Value: " << defaultValue;
			return defaultValue;
		}
		return boolPtr->value;
	}

	//Double getter
	double ConfigurationsDatabase::getDouble(const std::string &name, double defaultValue) const
	{
		Ddo::cPtr<Ddo::ConfigurationDouble> doublePtr = getDoublePtr(name);
		if(!doublePtr)
		{
			Wt::log("warning") << "DoublePtr not found in ConfigurationsDatabase in GetDouble(...). Name: " << name << ", Default Value: " << defaultValue;
			return defaultValue;
		}
		return doublePtr->value;
	}

	//Enum getter
	int ConfigurationsDatabase::getEnum(const std::string &name, int defaultValue) const
	{
		Ddo::cPtr<Ddo::ConfigurationEnum> enumPtr = getEnumPtr(name);
		if(!enumPtr)
		{
			Wt::log("warning") << "EnumPtr not found in ConfigurationsDatabase in GetEnum(...). Name: " << name << ", Default Value: " << defaultValue;
			return defaultValue;
		}
		return enumPtr->value;
	}

	//Float getter
	float ConfigurationsDatabase::getFloat(const std::string &name, float defaultValue) const
	{
		Ddo::cPtr<Ddo::ConfigurationFloat> floatPtr = getFloatPtr(name);
		if(!floatPtr)
		{
			Wt::log("warning") << "FloatPtr not found in ConfigurationsDatabase in GetFloat(...). Name: " << name << ", Default Value: " << defaultValue;
			return defaultValue;
		}
		return floatPtr->value;
	}

	//Integer getter
	int ConfigurationsDatabase::getInt(const std::string &name, int defaultValue) const
	{
		Ddo::cPtr<Ddo::ConfigurationInt> intPtr = getIntPtr(name);
		if(!intPtr)
		{
			Wt::log("warning") << "IntPtr not found in ConfigurationsDatabase in GetInt(...). Name: " << name << ", Default Value: " << defaultValue;
			return defaultValue;
		}
		return intPtr->value;
	}

	//Long integer getter
	long long ConfigurationsDatabase::getLongInt(const std::string &name, long long defaultValue) const
	{
		Ddo::cPtr<Ddo::ConfigurationLongInt> longIntPtr = getLongIntPtr(name);
		if(!longIntPtr)
		{
			Wt::log("warning") << "LongIntPtr not found in ConfigurationsDatabase in GetLongInt(...). Name: " << name << ", Default Value: " << defaultValue;
			return defaultValue;
		}
		return longIntPtr->value;
	}

	//String getter
	std::string ConfigurationsDatabase::getStr(const std::string &name, std::string defaultValue) const
	{
		Ddo::cPtr<Ddo::ConfigurationString> stringPtr = getStringPtr(name);
		if(!stringPtr)
		{
			Wt::log("warning") << "StringPtr not found in ConfigurationsDatabase in GetString(...). Name: " << name << ", Default Value: " << defaultValue;
			return defaultValue;
		}
		return stringPtr->value;
	}

	Ddo::cPtr<Ddo::ConfigurationLongInt> ConfigurationsDatabase::addLongInt(const std::string &name, long long value, Wt::Dbo::Session *alternateSession /*= nullptr*/)
	{
		std::lock_guard<std::shared_mutex> lock(_mutex);

		Dbo::ptr<Dbo::Configuration> configPtr;
		Dbo::ptr<Dbo::ConfigurationLongInt> configValPtr;
		
		//Use server's session if not provided as argument
		if(!alternateSession)
			alternateSession = &dboSession;

		Wt::Dbo::Transaction t(*alternateSession);

		configPtr = dboSession.find<Dbo::Configuration>().where("name = ? AND type = ?").bind(name).bind(Dbo::Configuration::LongInt);
		if(!configPtr)
			configPtr = dboSession.addNew<Dbo::Configuration>(name, Dbo::Configuration::LongInt);

		configValPtr = configPtr->longIntPtr;
		if(!configValPtr)
			configValPtr = dboSession.addNew<Dbo::ConfigurationLongInt>(configPtr);
		configValPtr.modify()->value = value;

		configPtr.flush();
		configValPtr.flush();
		t.commit();

		Ddo::ptr<Ddo::ConfigurationLongInt> result = std::make_shared<Ddo::ConfigurationLongInt>(configValPtr);
		_longIntMap[configPtr->name()] = result;
		return result;
	}

	long long ConfigurationsDatabase::getLoadDurationinMS() const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		return _loadDuration.count();
	}

	std::size_t ConfigurationsDatabase::configurationsCount() const
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		return _count;
	}

}
