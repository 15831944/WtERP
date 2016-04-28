#ifndef GS_DBODATABASE_PERMISSION_H
#define GS_DBODATABASE_PERMISSION_H

#include <map>
#include "Dbo/Dbos.h"

#include <Wt/Auth/Login>
#include <boost/thread/shared_mutex.hpp>

namespace GS
{
	class PermissionsDatabase
	{
	private:
		struct PermissionItem
		{
			PermissionSPtr permissionPtr;
			PermissionMap linkedPermissions;
		};
		typedef std::map< long long, PermissionItem > _PermissionItemMap;

	public:
		PermissionsDatabase(Wt::Dbo::Session &session);
		void reload() { fetchAll(); }

		PermissionCPtr getPermissionPtr(long long permissionId) const;
		PermissionMap getUserPermissions(Wt::Dbo::ptr<User> userPtr, Wt::Auth::LoginState loginState, Wt::Dbo::Session *altSession = nullptr);

	protected:
		void fetchAll();
		void linkAllDescendants(long long parentPermissionId, const _PermissionItemMap &sourceMap, PermissionMap &linkedPermissionMap);

		_PermissionItemMap _permissionItemMap;
		PermissionMap _loggedOutPermissions;
		PermissionMap _disabledLoginPermissions;
		PermissionMap _logggedInPermissions;

		boost::posix_time::time_duration _loadDuration;
		Wt::Dbo::Session &dboSession;

	private:
		mutable boost::shared_mutex _mutex;
	};
}

#endif