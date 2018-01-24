#include "Dbo/Configuration.h"

namespace ERP
{
	void mapConfigurationDboTree(Wt::Dbo::Session &dboSession)
	{
		dboSession.mapClass<Configuration>(Configuration::tableName());
		dboSession.mapClass<ConfigurationBool>(ConfigurationBool::tableName());
		dboSession.mapClass<ConfigurationEnum>(ConfigurationEnum::tableName());
		dboSession.mapClass<ConfigurationEnumValue>(ConfigurationEnumValue::tableName());
		dboSession.mapClass<ConfigurationDouble>(ConfigurationDouble::tableName());
		dboSession.mapClass<ConfigurationFloat>(ConfigurationFloat::tableName());
		dboSession.mapClass<ConfigurationInt>(ConfigurationInt::tableName());
		dboSession.mapClass<ConfigurationLongInt>(ConfigurationLongInt::tableName());
		dboSession.mapClass<ConfigurationString>(ConfigurationString::tableName());
	}
}

