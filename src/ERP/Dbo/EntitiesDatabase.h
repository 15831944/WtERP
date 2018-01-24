#ifndef ERP_ENTITIESDATABASE_H
#define ERP_ENTITIESDATABASE_H

#include "Common.h"
#include "Dbo/Dbos.h"

namespace ERP
{
	class EntitiesDatabase
	{
	public:
		EntitiesDatabase(Dbo::Session &session) : dboSession(session) { }

		Dbo::ptr<Entity> findOrCreateSelfEntity();

	protected:
		Dbo::Session &dboSession;
	};
}

#endif