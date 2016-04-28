#include "Dbo/PermissionsDatabase.h"

#include <Wt/WLogger>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/lock_guard.hpp>

namespace GS
{

	PermissionsDatabase::PermissionsDatabase(Wt::Dbo::Session &session)
		: dboSession(session)
	{
		fetchAll();
	}

	void PermissionsDatabase::fetchAll()
	{
		boost::lock_guard<boost::shared_mutex> lock(_mutex);
		//Time at start
		boost::posix_time::ptime ptStart = boost::posix_time::microsec_clock::local_time();

		//Insert into temporary objects first
		_PermissionItemMap permissionitemmap;

		//Fetch em all
		Wt::Dbo::Transaction t(dboSession);

		typedef boost::tuple<long long, long long> PermissionLinkTuple; //by, to
		typedef Wt::Dbo::collection<PermissionLinkTuple> PermissionLinkCollection;
		PermissionCollection permissionCollection = dboSession.find<Permission>();
		PermissionLinkCollection permissionLinkCollection = dboSession.query<PermissionLinkTuple>("SELECT by_id, to_id FROM linked_permission");
		DefaultPermissionCollection defaultPermissionCollection = dboSession.find<DefaultPermission>();

		//Permission ptrs
		for(const Wt::Dbo::ptr<Permission> &ptr : permissionCollection)
		{
			permissionitemmap[ptr.id()].permissionPtr = PermissionSPtr(new PermissionDdo(ptr));
		}

		//Linked permission ptrs
		for(const PermissionLinkTuple &tuple : permissionLinkCollection)
		{
			//0: BY, 1: TO
			permissionitemmap[boost::get<1>(tuple)].linkedPermissions[boost::get<0>(tuple)] = permissionitemmap[boost::get<1>(tuple)].permissionPtr;
		}

		//Link all descendants in the (implicit) permission hierarchy and make each item's linkedPermissions flat
		_PermissionItemMap sourceMap(permissionitemmap);
		for(auto &item : sourceMap) //All items
		{
			for(const auto &linkedItem : item.second.linkedPermissions) //All linked permissions
			{
				linkAllDescendants(linkedItem.second->id, sourceMap, permissionitemmap[item.first].linkedPermissions);
			}
		}

		//Default permissions
		for(const Wt::Dbo::ptr<DefaultPermission> &ptr : defaultPermissionCollection)
		{
			const auto &item = permissionitemmap[ptr->permissionPtr.id()];
			if(ptr->loginStates & DefaultPermission::LoggedOut)
			{
				_loggedOutPermissions[item.permissionPtr->id] = item.permissionPtr;
				_loggedOutPermissions.insert(item.linkedPermissions.begin(), item.linkedPermissions.end());
			}
			if(ptr->loginStates & DefaultPermission::DisabledLogin)
			{
				_disabledLoginPermissions[item.permissionPtr->id] = item.permissionPtr;
				_disabledLoginPermissions.insert(item.linkedPermissions.begin(), item.linkedPermissions.end());
			}
			if(ptr->loginStates & DefaultPermission::LoggedIn)
			{
				_logggedInPermissions[item.permissionPtr->id] = item.permissionPtr;
				_logggedInPermissions.insert(item.linkedPermissions.begin(), item.linkedPermissions.end());
			}
		}

		t.commit();
		_permissionItemMap.swap(permissionitemmap);

		//Time at end
		boost::posix_time::ptime ptEnd = boost::posix_time::microsec_clock::local_time();
		_loadDuration = ptEnd - ptStart;

		Wt::log("info") << "PermissionsDatabase: " << _permissionItemMap.size() << " permissions successfully loaded in " << _loadDuration.total_milliseconds() << " ms";
	}

	void PermissionsDatabase::linkAllDescendants(long long parentPermissionId, const _PermissionItemMap &sourceMap, PermissionMap &linkedPermissionMap)
	{
		auto fitr = sourceMap.find(parentPermissionId);
		if(fitr == sourceMap.end())
			return;

		for(const auto &linkedItem : fitr->second.linkedPermissions)
		{
			linkedPermissionMap.insert(linkedItem);
			linkAllDescendants(linkedItem.first, sourceMap, linkedPermissionMap);
		}
	}

	PermissionCPtr PermissionsDatabase::getPermissionPtr(long long permissionId) const
	{
		boost::shared_lock<boost::shared_mutex> lock(_mutex);
		_PermissionItemMap::const_iterator itr = _permissionItemMap.find(permissionId);
		if(itr == _permissionItemMap.end())
			return PermissionCPtr();

		return itr->second.permissionPtr;
	}

	PermissionMap PermissionsDatabase::getUserPermissions(Wt::Dbo::ptr<User> userPtr, Wt::Auth::LoginState loginState, Wt::Dbo::Session *altSession /*= nullptr*/)
	{
		if(!altSession)
			altSession = &dboSession;

		boost::shared_lock<boost::shared_mutex> lock(_mutex);

		//Return default logged out permissions if not logged in
		if(!userPtr || loginState == Wt::Auth::LoggedOut)
			return _loggedOutPermissions;

		//Default permissions
		PermissionMap granted;
		std::set<long long> denied;
		if(loginState == Wt::Auth::DisabledLogin)
			granted = _disabledLoginPermissions;
		else
			granted = _logggedInPermissions;

		Wt::Dbo::Transaction t(*altSession);
		UserPermissionCollection collection = userPtr->userPermissionCollection;
		for(const Wt::Dbo::ptr<UserPermission> &ptr : collection)
		{
			_PermissionItemMap::const_iterator itr = _permissionItemMap.find(ptr.id().permissionPtr.id());
			if(itr == _permissionItemMap.end())
				continue;

			//Add the user permission
			if(ptr->denied)
				denied.insert(itr->first);
			else
				granted[itr->first] = itr->second.permissionPtr;

			//Add linked permissions
			for(const auto &linkedItem : itr->second.linkedPermissions)
			{
				if(ptr->denied)
					denied.insert(linkedItem.first);
				else
					granted.insert(linkedItem);
			}

			//Remove denied permissions
			for(const long long &deniedId : denied)
				granted.erase(deniedId);
		}
		return granted;
	}

}

