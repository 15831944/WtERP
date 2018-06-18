#ifndef ERP_INVENTORYDATABASE_H
#define ERP_INVENTORYDATABASE_H

#include "Common.h"
#include "Dbo/Dbos.h"

namespace ERP
{
	class InventoryDatabase
	{
	public:
		InventoryDatabase(DboSession &serverDboSession);
		static InventoryDatabase &instance();
		
	private:
		DboSession &dboSession();
		
		DboSession &_serverDboSession;
	};
}

#endif
