#include "Dbo/EntitiesDatabase.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Application/WServer.h"

namespace GS
{

	Wt::Dbo::ptr<Entity> EntitiesDatabase::findOrCreateSelfEntity()
	{
		WServer *server = SERVER;

		Wt::Dbo::Transaction t(dboSession);
		Wt::Dbo::ptr<Entity> entityPtr;

		//Config found
		if(server->configs()->getLongIntPtr("SelfEntityId"))
		{
			long long entityId = server->configs()->getLongInt("SelfEntityId", -1);
			if(entityId != -1)
				entityPtr = dboSession.find<Entity>().where("id = ?").bind(entityId);
		}

		//Create entity if entity not found
		if(!entityPtr)
		{
			Wt::log("warning") << "Self entity was not found, creating self entity";
			entityPtr = dboSession.add(new Entity(Entity::InvalidType));
			entityPtr.modify()->name = "Self entity (Modify this entity)";
			entityPtr.flush();

			if(!server->configs()->addLongInt("SelfEntityId", entityPtr.id(), &dboSession))
				throw std::runtime_error("Error creating SelfEntityId config, ConfigurationsDatabase::addLongInt() returned null");
		}
		
		t.commit();
		return entityPtr;
	}

}