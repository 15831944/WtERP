#ifndef ERP_DBO_CONFIGURATION_H
#define ERP_DBO_CONFIGURATION_H

#include "Common.h"
#include <Wt/Dbo/Dbo.h>

namespace ERP
{
	void mapConfigurationDbos(Dbo::Session &dboSession);

	class Configuration;
	class ConfigurationBool;
	class ConfigurationDouble;
	class ConfigurationEnum;
	class ConfigurationFloat;
	class ConfigurationInt;
	class ConfigurationLongInt;
	class ConfigurationString;
	class ConfigurationEnumValue;

	typedef Dbo::collection<Dbo::ptr<Configuration>> ConfigurationCollection;
	typedef Dbo::collection<Dbo::ptr<ConfigurationBool>> ConfigurationBoolCollection;
	typedef Dbo::collection<Dbo::ptr<ConfigurationEnum>> ConfigurationEnumCollection;
	typedef Dbo::collection<Dbo::ptr<ConfigurationDouble>> ConfigurationDoubleCollection;
	typedef Dbo::collection<Dbo::ptr<ConfigurationFloat>> ConfigurationFloatCollection;
	typedef Dbo::collection<Dbo::ptr<ConfigurationInt>> ConfigurationIntCollection;
	typedef Dbo::collection<Dbo::ptr<ConfigurationLongInt>> ConfigurationLongIntCollection;
	typedef Dbo::collection<Dbo::ptr<ConfigurationString>> ConfigurationStringCollection;
	typedef Dbo::collection<Dbo::ptr<ConfigurationEnumValue>> EnumValueCollection;
}

namespace Wt
{
	namespace Dbo
	{
		//ConfigurationBool
		template<>
		struct dbo_traits<ERP::ConfigurationBool> : public dbo_default_traits
		{
			typedef ptr<ERP::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationDouble
		template<>
		struct dbo_traits<ERP::ConfigurationDouble> : public dbo_default_traits
		{
			typedef ptr<ERP::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationEnum
		template<>
		struct dbo_traits<ERP::ConfigurationEnum> : public dbo_default_traits
		{
			typedef ptr<ERP::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationFloat
		template<>
		struct dbo_traits<ERP::ConfigurationFloat> : public dbo_default_traits
		{
			typedef ptr<ERP::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationInt
		template<>
		struct dbo_traits<ERP::ConfigurationInt> : public dbo_default_traits
		{
			typedef ptr<ERP::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationLongInt
		template<>
		struct dbo_traits<ERP::ConfigurationLongInt> : public dbo_default_traits
		{
			typedef ptr<ERP::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationString
		template<>
		struct dbo_traits<ERP::ConfigurationString> : public dbo_default_traits
		{
			typedef ptr<ERP::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
	}
}

namespace ERP
{
	//Configuration DBO class
	class Configuration
	{
	public:
		enum ValueTypes
		{
			Bool = 0,
			Double = 1,
			Enum = 2,
			Float = 3,
			Int = 4,
			LongInt = 5,
			String = 6,
		};

	protected:
		std::string _name;
		ValueTypes _type = Bool;

	public:
		Dbo::weak_ptr<ConfigurationBool>	boolPtr;
		Dbo::weak_ptr<ConfigurationDouble>	doublePtr;
		Dbo::weak_ptr<ConfigurationEnum>	enumPtr;
		Dbo::weak_ptr<ConfigurationFloat>	floatPtr;
		Dbo::weak_ptr<ConfigurationInt>		intPtr;
		Dbo::weak_ptr<ConfigurationLongInt>	longIntPtr;
		Dbo::weak_ptr<ConfigurationString>	stringPtr;

		Configuration() = default;
		Configuration(std::string name, ValueTypes type)
			: _name(move(name)), _type(type)
		{ }

		std::string name() const { return _name; };
		ValueTypes type() const { return _type; };

		//Persistence Method
		template<class Action>
		void persist(Action &a)
		{
			Dbo::field(a, _name, "name", 50);
			Dbo::field(a, _type, "type");

			Dbo::hasOne(a, boolPtr, "configuration");
			Dbo::hasOne(a, doublePtr, "configuration");
			Dbo::hasOne(a, enumPtr, "configuration");
			Dbo::hasOne(a, floatPtr, "configuration");
			Dbo::hasOne(a, intPtr, "configuration");
			Dbo::hasOne(a, longIntPtr, "configuration");
			Dbo::hasOne(a, stringPtr, "configuration");
		}
		DEFINE_DBO_TABLENAME("configuration");
	};

	//ConfigurationBool DBO Class
	class BaseConfigurationBool
	{
	public:
		bool value = false;
		bool defaultValue = false;
		boost::optional<bool> recommendedValue;
	};
	class ConfigurationBool : public BaseConfigurationBool
	{
	private:
		Dbo::ptr<Configuration> _configurationPtr; //belongsTo

	public:
		ConfigurationBool() = default;
		ConfigurationBool(const ConfigurationBool &) = default;
		ConfigurationBool(Dbo::ptr<Configuration> configurationPtr)
			: _configurationPtr(std::move(configurationPtr))
		{ }

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _configurationPtr, "configuration", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, value, "value");
			Dbo::field(a, defaultValue, "defaultValue");
			Dbo::field(a, recommendedValue, "recommendedValue");
		}
		DEFINE_DBO_TABLENAME("configurationbool");
	};

	//ConfigurationDouble DBO Class
	class BaseConfigurationDouble
	{
	public:
		double value = -1;
		double defaultValue = -1;
		boost::optional<double> recommendedValue;
		boost::optional<double> minValue;
		boost::optional<double> maxValue;
	};
	class ConfigurationDouble : public BaseConfigurationDouble
	{
	private:
		Dbo::ptr<Configuration> _configurationPtr; //belongsTo

	public:
		ConfigurationDouble() = default;
		ConfigurationDouble(const ConfigurationDouble &) = default;
		ConfigurationDouble(Dbo::ptr<Configuration> configurationPtr)
			: _configurationPtr(move(configurationPtr))
		{ }

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _configurationPtr, "configuration", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, value, "value");
			Dbo::field(a, defaultValue, "defaultValue");
			Dbo::field(a, recommendedValue, "recommendedValue");
			Dbo::field(a, minValue, "minValue");
			Dbo::field(a, maxValue, "maxValue");
		}
		DEFINE_DBO_TABLENAME("configurationdouble");
	};

	class ConfigurationEnumValue
	{
	public:
		Dbo::ptr<ConfigurationEnum> enumPtr; //belongsTo
		int value = -1;

		ConfigurationEnumValue() = default;
		ConfigurationEnumValue(Dbo::ptr<ConfigurationEnum> enumPtr)
			: enumPtr(move(enumPtr))
		{ }

		template<class Action>void persist(Action &a)
		{
			Dbo::field(a, value, "value");

			Dbo::belongsTo(a, enumPtr, "enum", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
		}
		DEFINE_DBO_TABLENAME("configurationenumvalue");
	};

	//ConfigurationEnum DBO Class
	class BaseConfigurationEnum
	{
	public:
		int value = -1;
		int defaultValue = -1;
		boost::optional<int> recommendedValue;
	};
	class ConfigurationEnum : public BaseConfigurationEnum
	{
	private:
		Dbo::ptr<Configuration> _configurationPtr; //belongsTo

	public:
		ConfigurationEnum() = default;
		ConfigurationEnum(const ConfigurationEnum &) = default;
		ConfigurationEnum(Dbo::ptr<Configuration> configurationPtr)
			: _configurationPtr(move(configurationPtr))
		{ }

		EnumValueCollection enumValueCollection;

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _configurationPtr, "configuration", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, value, "value");
			Dbo::field(a, defaultValue, "defaultValue");
			Dbo::field(a, recommendedValue, "recommendedValue");

			Dbo::hasMany(a, enumValueCollection, Dbo::ManyToOne, "enum");
		}
		DEFINE_DBO_TABLENAME("configurationenum");
	};

	//ConfigurationFloat DBO Class
	class BaseConfigurationFloat
	{
	public:
		float value = -1;
		float defaultValue = -1;
		boost::optional<float> recommendedValue;
		boost::optional<float> minValue;
		boost::optional<float> maxValue;
	};
	class ConfigurationFloat : public BaseConfigurationFloat
	{
	private:
		Dbo::ptr<Configuration> _configurationPtr; //belongsTo

	public:
		ConfigurationFloat() = default;
		ConfigurationFloat(const ConfigurationFloat &) = default;
		ConfigurationFloat(Dbo::ptr<Configuration> configurationPtr)
			: _configurationPtr(move(configurationPtr))
		{ }

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _configurationPtr, "configuration", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, value, "value");
			Dbo::field(a, defaultValue, "defaultValue");
			Dbo::field(a, recommendedValue, "recommendedValue");
			Dbo::field(a, minValue, "minValue");
			Dbo::field(a, maxValue, "maxValue");
		}
		DEFINE_DBO_TABLENAME("configurationfloat");
	};

	//ConfigurationInt DBO Class
	class BaseConfigurationInt
	{
	public:
		int value = -1;
		boost::optional<int> defaultValue;
		boost::optional<int> recommendedValue;
		boost::optional<int> minValue;
		boost::optional<int> maxValue;
	};
	class ConfigurationInt : public BaseConfigurationInt
	{
	private:
		Dbo::ptr<Configuration> _configurationPtr; //belongsTo

	public:
		ConfigurationInt() = default;
		ConfigurationInt(const ConfigurationInt &) = default;
		ConfigurationInt(Dbo::ptr<Configuration> configurationPtr)
			: _configurationPtr(move(configurationPtr))
		{ }

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _configurationPtr, "configuration", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, value, "value");
			Dbo::field(a, defaultValue, "defaultValue");
			Dbo::field(a, recommendedValue, "recommendedValue");
			Dbo::field(a, minValue, "minValue");
			Dbo::field(a, maxValue, "maxValue");
		}
		DEFINE_DBO_TABLENAME("configurationint");
	};

	//ConfigurationLongInt DBO Class
	class BaseConfigurationLongInt
	{
	public:
		long long value = -1;
		boost::optional<long long> defaultValue;
		boost::optional<long long> recommendedValue;
		boost::optional<long long> minValue;
		boost::optional<long long> maxValue;
	};
	class ConfigurationLongInt : public BaseConfigurationLongInt
	{
	private:
		Dbo::ptr<Configuration> _configurationPtr; //belongsTo

	public:
		ConfigurationLongInt() = default;
		ConfigurationLongInt(const ConfigurationLongInt &) = default;
		ConfigurationLongInt(Dbo::ptr<Configuration> configurationPtr)
			: _configurationPtr(move(configurationPtr))
		{ }

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _configurationPtr, "configuration", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, value, "value");
			Dbo::field(a, defaultValue, "defaultValue");
			Dbo::field(a, recommendedValue, "recommendedValue");
			Dbo::field(a, minValue, "minValue");
			Dbo::field(a, maxValue, "maxValue");
		}
		DEFINE_DBO_TABLENAME("configurationlongint");
	};

	//ConfigurationString DBO Class
	class BaseConfigurationString
	{
	public:
		std::string value;
		std::string defaultValue;
		std::string recommendedValue;
		std::string exampleValue;
		boost::optional<int> minLength;
		boost::optional<int> maxLength;
	};
	class ConfigurationString : public BaseConfigurationString
	{
	private:
		Dbo::ptr<Configuration> _configurationPtr; //belongsTo

	public:
		ConfigurationString() = default;
		ConfigurationString(const ConfigurationString &) = default;
		ConfigurationString(Dbo::ptr<Configuration> configurationPtr)
			: _configurationPtr(move(configurationPtr))
		{ }

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _configurationPtr, "configuration", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, value, "value");
			Dbo::field(a, defaultValue, "defaultValue");
			Dbo::field(a, recommendedValue, "recommendedValue");
			Dbo::field(a, exampleValue, "exampleValue");
			Dbo::field(a, minLength, "minLength");
			Dbo::field(a, maxLength, "maxLength");
		}
		DEFINE_DBO_TABLENAME("configurationstring");
	};

	namespace Ddo
	{
		class ConfigurationKey
		{
		protected:
			ConfigurationKey(std::string name) : _name(move(name)) {}
			std::string _name;

		public:
			std::string name() const { return _name; };
		};

		class ConfigurationBool : public BaseConfigurationBool, public ConfigurationKey
		{
		public:
			ConfigurationBool(const Dbo::ptr<ERP::ConfigurationBool> &ptr)
				: BaseConfigurationBool(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationDouble : public BaseConfigurationDouble, public ConfigurationKey
		{
		public:
			ConfigurationDouble(const Dbo::ptr<ERP::ConfigurationDouble> &ptr)
				: BaseConfigurationDouble(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationEnum : public BaseConfigurationEnum, public ConfigurationKey
		{
		public:
			ConfigurationEnum(const Dbo::ptr<ERP::ConfigurationEnum> &ptr)
				: BaseConfigurationEnum(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationFloat : public BaseConfigurationFloat, public ConfigurationKey
		{
		public:
			ConfigurationFloat(const Dbo::ptr<ERP::ConfigurationFloat> &ptr)
				: BaseConfigurationFloat(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationInt : public BaseConfigurationInt, public ConfigurationKey
		{
		public:
			ConfigurationInt(const Dbo::ptr<ERP::ConfigurationInt> &ptr)
				: BaseConfigurationInt(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationLongInt : public BaseConfigurationLongInt, public ConfigurationKey
		{
		public:
			ConfigurationLongInt(const Dbo::ptr<ERP::ConfigurationLongInt> &ptr)
				: BaseConfigurationLongInt(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationString : public BaseConfigurationString, public ConfigurationKey
		{
		public:
			ConfigurationString(const Dbo::ptr<ERP::ConfigurationString> &ptr)
				: BaseConfigurationString(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};
	}
}

#endif