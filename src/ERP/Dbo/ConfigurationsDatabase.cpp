#include "Dbo/ConfigurationsDatabase.h"
#include "Application/WServer.h"

namespace ERP
{
	ConfigurationsDatabase::ConfigurationsDatabase(Wt::Dbo::Session &session)
		: dboSession(session)
	{
		fetchAll();
	}

	void ConfigurationsDatabase::fetchAll()
	{
		boost::lock_guard<boost::shared_mutex> lock(_mutex);
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

		//Fetch em all
		Wt::Dbo::Transaction transaction(dboSession);
		ConfigurationBoolCollection boolCollection = dboSession.find<ConfigurationBool>();
		ConfigurationDoubleCollection doubleCollection = dboSession.find<ConfigurationDouble>();
		ConfigurationEnumCollection enumCollection = dboSession.find<ConfigurationEnum>();
		ConfigurationFloatCollection floatCollection = dboSession.find<ConfigurationFloat>();
		ConfigurationIntCollection intCollection = dboSession.find<ConfigurationInt>();
		ConfigurationLongIntCollection longIntCollection = dboSession.find<ConfigurationLongInt>();
		ConfigurationStringCollection stringCollection = dboSession.find<ConfigurationString>();

		//Bool
		for(Dbo::ptr<ConfigurationBool> &ptr : boolCollection)
		{
			boolmap[ptr.id()->name()] = make_shared<Ddo::ConfigurationBool>(ptr);
			++_count;
		}

		//Double
		for(Dbo::ptr<ConfigurationDouble> &ptr : doubleCollection)
		{
			doublemap[ptr.id()->name()] = make_shared<Ddo::ConfigurationDouble>(ptr);
			++_count;
		}

		//Enum
		for(Dbo::ptr<ConfigurationEnum> &ptr : enumCollection)
		{
			enummap[ptr.id()->name()] = make_shared<Ddo::ConfigurationEnum>(ptr);
			++_count;
		}

		//Float
		for(Dbo::ptr<ConfigurationFloat> &ptr : floatCollection)
		{
			floatmap[ptr.id()->name()] = make_shared<Ddo::ConfigurationFloat>(ptr);
			++_count;
		}

		//Int
		for(Dbo::ptr<ConfigurationInt> &ptr : intCollection)
		{
			intmap[ptr.id()->name()] = make_shared<Ddo::ConfigurationInt>(ptr);
			++_count;
		}

		//LongInt
		for(Dbo::ptr<ConfigurationLongInt> &ptr : longIntCollection)
		{
			longintmap[ptr.id()->name()] = make_shared<Ddo::ConfigurationLongInt>(ptr);
			++_count;
		}

		//String
		for(Dbo::ptr<ConfigurationString> &ptr : stringCollection)
		{
			stringmap[ptr.id()->name()] = make_shared<Ddo::ConfigurationString>(ptr);
			++_count;
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

		Wt::log("erp-info") << "ConfigurationsDatabase: " << _count << " entries successfully loaded in " << _loadDuration.count() << " ms";
	}

	shared_ptr<const Ddo::ConfigurationBool> ConfigurationsDatabase::getBoolPtr(const std::string &name) const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		auto itr = _boolMap.find(name);
		if(itr == _boolMap.end())
			return nullptr;

		return itr->second;
	}

	shared_ptr<const Ddo::ConfigurationDouble> ConfigurationsDatabase::getDoublePtr(const std::string &name) const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		auto itr = _doubleMap.find(name);
		if(itr == _doubleMap.end())
			return nullptr;

		return itr->second;
	}

	shared_ptr<const Ddo::ConfigurationEnum> ConfigurationsDatabase::getEnumPtr(const std::string &name) const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		auto itr = _enumMap.find(name);
		if(itr == _enumMap.end())
			return nullptr;

		return itr->second;
	}

	shared_ptr<const Ddo::ConfigurationFloat> ConfigurationsDatabase::getFloatPtr(const std::string &name) const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		auto itr = _floatMap.find(name);
		if(itr == _floatMap.end())
			return nullptr;

		return itr->second;
	}

	shared_ptr<const Ddo::ConfigurationInt> ConfigurationsDatabase::getIntPtr(const std::string &name) const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		auto itr = _intMap.find(name);
		if(itr == _intMap.end())
			return nullptr;

		return itr->second;
	}

	shared_ptr<const Ddo::ConfigurationLongInt> ConfigurationsDatabase::getLongIntPtr(const std::string &name) const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		auto itr = _longIntMap.find(name);
		if(itr == _longIntMap.end())
			return nullptr;

		return itr->second;
	}

	shared_ptr<const Ddo::ConfigurationString> ConfigurationsDatabase::getStringPtr(const std::string &name) const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		auto itr = _stringMap.find(name);
		if(itr == _stringMap.end())
			return nullptr;

		return itr->second;
	}

	//Boolean getter
	bool ConfigurationsDatabase::getBool(const std::string &name, bool defaultValue) const
	{
		shared_ptr<const Ddo::ConfigurationBool> boolPtr = getBoolPtr(name);
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
		shared_ptr<const Ddo::ConfigurationDouble> doublePtr = getDoublePtr(name);
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
		shared_ptr<const Ddo::ConfigurationEnum> enumPtr = getEnumPtr(name);
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
		shared_ptr<const Ddo::ConfigurationFloat> floatPtr = getFloatPtr(name);
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
		shared_ptr<const Ddo::ConfigurationInt> intPtr = getIntPtr(name);
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
		shared_ptr<const Ddo::ConfigurationLongInt> longIntPtr = getLongIntPtr(name);
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
		shared_ptr<const Ddo::ConfigurationString> stringPtr = getStringPtr(name);
		if(!stringPtr)
		{
			Wt::log("warning") << "StringPtr not found in ConfigurationsDatabase in GetString(...). Name: " << name << ", Default Value: " << defaultValue;
			return defaultValue;
		}
		return stringPtr->value;
	}

	shared_ptr<const Ddo::ConfigurationLongInt> ConfigurationsDatabase::addLongInt(const std::string &name, long long value, Wt::Dbo::Session *alternateSession)
	{
		boost::lock_guard<boost::shared_mutex> lock(_mutex);

		Dbo::ptr<Configuration> configPtr;
		Dbo::ptr<ConfigurationLongInt> configValPtr;
		
		//Use server's session if not provided as argument
		if(!alternateSession)
			alternateSession = &dboSession;

		Wt::Dbo::Transaction t(*alternateSession);

		configPtr = dboSession.find<Configuration>().where("name = ? AND type = ?").bind(name).bind(Configuration::LongInt);
		if(!configPtr)
			configPtr = dboSession.addNew<Configuration>(name, Configuration::LongInt);

		configValPtr = configPtr->longIntPtr;
		if(!configValPtr)
			configValPtr = dboSession.addNew<ConfigurationLongInt>(configPtr);
		configValPtr.modify()->value = value;

		configPtr.flush();
		configValPtr.flush();
		t.commit();

		auto result = make_shared<Ddo::ConfigurationLongInt>(configValPtr);
		_longIntMap[configPtr->name()] = result;
		return result;
	}

	long long ConfigurationsDatabase::getLoadDurationinMS() const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		return _loadDuration.count();
	}

	std::size_t ConfigurationsDatabase::configurationsCount() const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		return _count;
	}

}
