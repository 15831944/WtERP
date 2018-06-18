#include "Dbo/InventoryDatabase.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"

namespace ERP
{
	InventoryDatabase::InventoryDatabase(DboSession &serverDboSession)
		: _serverDboSession(serverDboSession)
	{ }
	
	DboSession &InventoryDatabase::dboSession()
	{
		WApplication *app = APP;
		if(app)
			return app->dboSession();
		return _serverDboSession;
	}
	
	InventoryDatabase &InventoryDatabase::instance()
	{
		return SERVER->inventoryDatabase();
	}
}