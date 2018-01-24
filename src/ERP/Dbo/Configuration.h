#ifndef WW_DBO_CONFIGURATION_H
#define WW_DBO_CONFIGURATION_H

#include <Wt/Dbo/Dbo.h>
#include <boost/optional.hpp>
#include <utility>

namespace WW
{
	namespace Dbo
	{
		template<class C>
		using ptr = Wt::Dbo::ptr<C>;

		class Configuration;
		class ConfigurationBool;
		class ConfigurationDouble;
		class ConfigurationEnum;
		class ConfigurationFloat;
		class ConfigurationInt;
		class ConfigurationLongInt;
		class ConfigurationString;
		class ConfigurationEnumValue;

		typedef Wt::Dbo::collection< ptr<Configuration> > ConfigurationCollection;
		typedef Wt::Dbo::collection< ptr<ConfigurationBool> > BoolCollection;
		typedef Wt::Dbo::collection< ptr<ConfigurationEnum> > EnumCollection;
		typedef Wt::Dbo::collection< ptr<ConfigurationDouble> > DoubleCollection;
		typedef Wt::Dbo::collection< ptr<ConfigurationFloat> > FloatCollection;
		typedef Wt::Dbo::collection< ptr<ConfigurationInt> > IntCollection;
		typedef Wt::Dbo::collection< ptr<ConfigurationLongInt> > LongIntCollection;
		typedef Wt::Dbo::collection< ptr<ConfigurationString> > StringCollection;
		typedef Wt::Dbo::collection< ptr<ConfigurationEnumValue> > EnumValueCollection;
	}

	namespace Ddo
	{
		template<class C>
		using ptr = std::shared_ptr<C>;

		template<class C>
		using cPtr = std::shared_ptr<const C>;
	}
}

namespace WW
{
	namespace Ddo
	{
		class SurrogateKey
		{
		public:
			long long id() const { return _id; }

		protected:
			SurrogateKey(long long id)
				: _id(id)
			{ };

			long long _id;
		};
	}
}

namespace Wt
{
	namespace Dbo
	{
		//ConfigurationBool
		template<>
		struct dbo_traits<WW::Dbo::ConfigurationBool> : public dbo_default_traits
		{
			typedef WW::Dbo::ptr<WW::Dbo::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationDouble
		template<>
		struct dbo_traits<WW::Dbo::ConfigurationDouble> : public dbo_default_traits
		{
			typedef WW::Dbo::ptr<WW::Dbo::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationEnum
		template<>
		struct dbo_traits<WW::Dbo::ConfigurationEnum> : public dbo_default_traits
		{
			typedef WW::Dbo::ptr<WW::Dbo::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationFloat
		template<>
		struct dbo_traits<WW::Dbo::ConfigurationFloat> : public dbo_default_traits
		{
			typedef WW::Dbo::ptr<WW::Dbo::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationInt
		template<>
		struct dbo_traits<WW::Dbo::ConfigurationInt> : public dbo_default_traits
		{
			typedef WW::Dbo::ptr<WW::Dbo::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationLongInt
		template<>
		struct dbo_traits<WW::Dbo::ConfigurationLongInt> : public dbo_default_traits
		{
			typedef WW::Dbo::ptr<WW::Dbo::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		//ConfigurationString
		template<>
		struct dbo_traits<WW::Dbo::ConfigurationString> : public dbo_default_traits
		{
			typedef WW::Dbo::ptr<WW::Dbo::Configuration> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
	}
}

namespace WW
{
	namespace Dbo
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
			Wt::Dbo::weak_ptr<ConfigurationBool>	boolPtr;
			Wt::Dbo::weak_ptr<ConfigurationDouble>	doublePtr;
			Wt::Dbo::weak_ptr<ConfigurationEnum>	enumPtr;
			Wt::Dbo::weak_ptr<ConfigurationFloat>	floatPtr;
			Wt::Dbo::weak_ptr<ConfigurationInt>		intPtr;
			Wt::Dbo::weak_ptr<ConfigurationLongInt>	longIntPtr;
			Wt::Dbo::weak_ptr<ConfigurationString>	stringPtr;

			Configuration() = default;
			Configuration(std::string name, ValueTypes type)
				: _name(std::move(name)), _type(type)
			{ }

			std::string name() const { return _name; };
			ValueTypes type() const { return _type; };

			//Persistence Method
			template<class Action>
			void persist(Action &a)
			{
				Wt::Dbo::field(a, _name, "name", 50);
				Wt::Dbo::field(a, _type, "type");

				Wt::Dbo::hasOne(a, boolPtr, "configuration");
				Wt::Dbo::hasOne(a, doublePtr, "configuration");
				Wt::Dbo::hasOne(a, enumPtr, "configuration");
				Wt::Dbo::hasOne(a, floatPtr, "configuration");
				Wt::Dbo::hasOne(a, intPtr, "configuration");
				Wt::Dbo::hasOne(a, longIntPtr, "configuration");
				Wt::Dbo::hasOne(a, stringPtr, "configuration");
			}
			constexpr static const char *tableName()
			{
				return "configuration";
			}
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
			ptr<Configuration> _configurationPtr; //belongsTo

		public:
			ConfigurationBool() = default;
			ConfigurationBool(const ConfigurationBool &) = default;
			ConfigurationBool(ptr<Configuration> configurationPtr)
				: _configurationPtr(std::move(configurationPtr))
			{ }

			template<class Action>void persist(Action &a)
			{
				Wt::Dbo::id(a, _configurationPtr, "configuration", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
				Wt::Dbo::field(a, value, "value");
				Wt::Dbo::field(a, defaultValue, "defaultValue");
				Wt::Dbo::field(a, recommendedValue, "recommendedValue");
			}
			constexpr static const char *tableName()
			{
				return "configurationbool";
			}
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
			ptr<Configuration> _configurationPtr; //belongsTo

		public:
			ConfigurationDouble() = default;
			ConfigurationDouble(const ConfigurationDouble &) = default;
			ConfigurationDouble(ptr<Configuration> configurationPtr)
				: _configurationPtr(std::move(configurationPtr))
			{ }

			template<class Action>void persist(Action &a)
			{
				Wt::Dbo::id(a, _configurationPtr, "configuration", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
				Wt::Dbo::field(a, value, "value");
				Wt::Dbo::field(a, defaultValue, "defaultValue");
				Wt::Dbo::field(a, recommendedValue, "recommendedValue");
				Wt::Dbo::field(a, minValue, "minValue");
				Wt::Dbo::field(a, maxValue, "maxValue");
			}
			constexpr static const char *tableName()
			{
				return "configurationdouble";
			}
		};

		class ConfigurationEnumValue
		{
		public:
			ptr<ConfigurationEnum> enumPtr; //belongsTo
			int value = -1;

			ConfigurationEnumValue() = default;
			ConfigurationEnumValue(ptr<ConfigurationEnum> enumPtr)
				: enumPtr(std::move(enumPtr))
			{ }

			template<class Action>void persist(Action &a)
			{
				Wt::Dbo::field(a, value, "value");

				Wt::Dbo::belongsTo(a, enumPtr, "enum", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			}
			constexpr static const char *tableName()
			{
				return "configurationenumvalue";
			}
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
			ptr<Configuration> _configurationPtr; //belongsTo

		public:
			ConfigurationEnum() = default;
			ConfigurationEnum(const ConfigurationEnum &) = default;
			ConfigurationEnum(ptr<Configuration> configurationPtr)
				: _configurationPtr(std::move(configurationPtr))
			{ }

			EnumValueCollection enumValueCollection;

			template<class Action>void persist(Action &a)
			{
				Wt::Dbo::id(a, _configurationPtr, "configuration", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
				Wt::Dbo::field(a, value, "value");
				Wt::Dbo::field(a, defaultValue, "defaultValue");
				Wt::Dbo::field(a, recommendedValue, "recommendedValue");

				Wt::Dbo::hasMany(a, enumValueCollection, Wt::Dbo::ManyToOne, "enum");
			}
			constexpr static const char *tableName()
			{
				return "configurationenum";
			}
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
			ptr<Configuration> _configurationPtr; //belongsTo

		public:
			ConfigurationFloat() = default;
			ConfigurationFloat(const ConfigurationFloat &) = default;
			ConfigurationFloat(ptr<Configuration> configurationPtr)
				: _configurationPtr(std::move(configurationPtr))
			{ }

			template<class Action>void persist(Action &a)
			{
				Wt::Dbo::id(a, _configurationPtr, "configuration", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
				Wt::Dbo::field(a, value, "value");
				Wt::Dbo::field(a, defaultValue, "defaultValue");
				Wt::Dbo::field(a, recommendedValue, "recommendedValue");
				Wt::Dbo::field(a, minValue, "minValue");
				Wt::Dbo::field(a, maxValue, "maxValue");
			}
			constexpr static const char *tableName()
			{
				return "configurationfloat";
			}
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
			ptr<Configuration> _configurationPtr; //belongsTo

		public:
			ConfigurationInt() = default;
			ConfigurationInt(const ConfigurationInt &) = default;
			ConfigurationInt(ptr<Configuration> configurationPtr)
				: _configurationPtr(std::move(configurationPtr))
			{ }

			template<class Action>void persist(Action &a)
			{
				Wt::Dbo::id(a, _configurationPtr, "configuration", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
				Wt::Dbo::field(a, value, "value");
				Wt::Dbo::field(a, defaultValue, "defaultValue");
				Wt::Dbo::field(a, recommendedValue, "recommendedValue");
				Wt::Dbo::field(a, minValue, "minValue");
				Wt::Dbo::field(a, maxValue, "maxValue");
			}
			constexpr static const char *tableName()
			{
				return "configurationint";
			}
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
			ptr<Configuration> _configurationPtr; //belongsTo

		public:
			ConfigurationLongInt() = default;
			ConfigurationLongInt(const ConfigurationLongInt &) = default;
			ConfigurationLongInt(ptr<Configuration> configurationPtr)
				: _configurationPtr(std::move(configurationPtr))
			{ }

			template<class Action>void persist(Action &a)
			{
				Wt::Dbo::id(a, _configurationPtr, "configuration", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
				Wt::Dbo::field(a, value, "value");
				Wt::Dbo::field(a, defaultValue, "defaultValue");
				Wt::Dbo::field(a, recommendedValue, "recommendedValue");
				Wt::Dbo::field(a, minValue, "minValue");
				Wt::Dbo::field(a, maxValue, "maxValue");
			}
			constexpr static const char *tableName()
			{
				return "configurationlongint";
			}
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
			ptr<Configuration> _configurationPtr; //belongsTo

		public:
			ConfigurationString() = default;
			ConfigurationString(const ConfigurationString &) = default;
			ConfigurationString(ptr<Configuration> configurationPtr)
				: _configurationPtr(std::move(configurationPtr))
			{ }

			template<class Action>void persist(Action &a)
			{
				Wt::Dbo::id(a, _configurationPtr, "configuration", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
				Wt::Dbo::field(a, value, "value");
				Wt::Dbo::field(a, defaultValue, "defaultValue");
				Wt::Dbo::field(a, recommendedValue, "recommendedValue");
				Wt::Dbo::field(a, exampleValue, "exampleValue");
				Wt::Dbo::field(a, minLength, "minLength");
				Wt::Dbo::field(a, maxLength, "maxLength");
			}
			constexpr static const char *tableName()
			{
				return "configurationstring";
			}
		};

	}

	namespace Ddo
	{

		class ConfigurationKey
		{
		protected:
			std::string _name;

			ConfigurationKey(std::string name)
				: _name(std::move(name))
			{}

		public:
			std::string name() const { return _name; };
		};

		class ConfigurationBool : public Dbo::BaseConfigurationBool, public ConfigurationKey
		{
		public:
			ConfigurationBool(const Dbo::ptr<Dbo::ConfigurationBool> &ptr)
				: BaseConfigurationBool(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationDouble : public Dbo::BaseConfigurationDouble, public ConfigurationKey
		{
		public:
			ConfigurationDouble(const Dbo::ptr<Dbo::ConfigurationDouble> &ptr)
				: Dbo::BaseConfigurationDouble(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationEnum : public Dbo::BaseConfigurationEnum, public ConfigurationKey
		{
		public:
			ConfigurationEnum(const Dbo::ptr<Dbo::ConfigurationEnum> &ptr)
				: Dbo::BaseConfigurationEnum(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationFloat : public Dbo::BaseConfigurationFloat, public ConfigurationKey
		{
		public:
			ConfigurationFloat(const Dbo::ptr<Dbo::ConfigurationFloat> &ptr)
				: Dbo::BaseConfigurationFloat(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationInt : public Dbo::BaseConfigurationInt, public ConfigurationKey
		{
		public:
			ConfigurationInt(const Dbo::ptr<Dbo::ConfigurationInt> &ptr)
				: Dbo::BaseConfigurationInt(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationLongInt : public Dbo::BaseConfigurationLongInt, public ConfigurationKey
		{
		public:
			ConfigurationLongInt(const Dbo::ptr<Dbo::ConfigurationLongInt> &ptr)
				: Dbo::BaseConfigurationLongInt(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

		class ConfigurationString : public Dbo::BaseConfigurationString, public ConfigurationKey
		{
		public:
			ConfigurationString(const Dbo::ptr<Dbo::ConfigurationString> &ptr)
				: BaseConfigurationString(*ptr), ConfigurationKey(ptr.id()->name())
			{ }
		};

	}

	namespace Dbo
	{
		void mapConfigurationDboTree(Wt::Dbo::Session &dboSession);
	}
}

#endif