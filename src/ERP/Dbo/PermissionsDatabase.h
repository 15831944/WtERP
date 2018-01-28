#ifndef ERP_DBODATABASE_PERMISSION_H
#define ERP_DBODATABASE_PERMISSION_H

#include "Common.h"
#include "Dbo/Dbos.h"

#include <Wt/Auth/Login.h>
#include <boost/thread/shared_mutex.hpp>

namespace ERP
{
	using namespace std::chrono;

	class PermissionsDatabase
	{
	private:
		struct PermissionItem
		{
			shared_ptr<PermissionDdo> permissionPtr;
			PermissionMap linkedPermissions;
		};
		typedef std::map<long long, PermissionItem> _PermissionItemMap;

	public:
		PermissionsDatabase(Dbo::Session &session);
		void reload() { fetchAll(); }

		shared_ptr<const PermissionDdo> getPermissionPtr(long long permissionId) const;
		PermissionMap getUserPermissions(Dbo::ptr<User> userPtr, Wt::Auth::LoginState loginState, Dbo::Session *altSession = nullptr);

	protected:
		void fetchAll();
		void linkAllDescendants(long long parentPermissionId, const _PermissionItemMap &sourceMap, PermissionMap &linkedPermissionMap);

		_PermissionItemMap _permissionItemMap;
		PermissionMap _loggedOutPermissions;
		PermissionMap _disabledLoginPermissions;
		PermissionMap _logggedInPermissions;

		milliseconds _loadDuration;
		Dbo::Session &dboSession;

	private:
		mutable boost::shared_mutex _mutex;
	};
}

#endif