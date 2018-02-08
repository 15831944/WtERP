#ifndef ERP_DBOSESSION_H
#define ERP_DBOSESSION_H

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/Exception.h>

class DboSession : public Wt::Dbo::Session
{
public:
	template<class C, class P>
	Wt::Dbo::ptr<C> loadLatestVersion(const Wt::Dbo::ptr<P> &parentPtr)
	{
		Wt::Dbo::ptr<C> latestVersionPtr = find<C>().where("parent_id = ?").orderBy("timestamp desc").limit(1)
				.bind(parentPtr.id());
		if(!latestVersionPtr)
			throw Wt::Dbo::ObjectNotFoundException(C::tableName(), "parent_id = " + std::to_string(parentPtr.id()));
		return latestVersionPtr;
	}
};


#endif
