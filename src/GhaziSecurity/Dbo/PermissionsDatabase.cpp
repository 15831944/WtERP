#include "Dbo/PermissionsDatabase.h"

#include <Wt/WLogger.h>
#include <mutex>
#include <shared_mutex>

namespace GS
{

	PermissionsDatabase::PermissionsDatabase(Dbo::Session &session)
		: dboSession(session)
	{
		fetchAll();
	}

	void PermissionsDatabase::fetchAll()
	{
		std::lock_guard<std::shared_mutex> lock(_mutex);
		//Time at start
		steady_clock::time_point tpStart = steady_clock::now();

		//Insert into temporary objects first
		_PermissionItemMap permissionitemmap;

		//Fetch em all
		Dbo::Transaction t(dboSession);

		typedef tuple<long long, long long> PermissionLinkTuple; //by, to
		typedef Dbo::collection<PermissionLinkTuple> PermissionLinkCollection;
		PermissionCollection permissionCollection = dboSession.find<Permission>();
		PermissionLinkCollection permissionLinkCollection = dboSession.query<PermissionLinkTuple>("SELECT by_id, to_id FROM linked_permission");
		DefaultPermissionCollection defaultPermissionCollection = dboSession.find<DefaultPermission>();

		//Permission ptrs
		for(const Dbo::ptr<Permission> &ptr : permissionCollection)
		{
			permissionitemmap[ptr.id()].permissionPtr = make_shared<PermissionDdo>(ptr);
		}

		//Linked permission ptrs
		for(const PermissionLinkTuple &tuple : permissionLinkCollection)
		{
			//0: BY, 1: TO
			permissionitemmap[std::get<1>(tuple)].linkedPermissions[std::get<0>(tuple)] = permissionitemmap[std::get<1>(tuple)].permissionPtr;
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
		for(const Dbo::ptr<DefaultPermission> &ptr : defaultPermissionCollection)
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
		steady_clock::time_point tpEnd = steady_clock::now();
		_loadDuration = duration_cast<milliseconds>(tpEnd - tpStart);

		Wt::log("gs-info") << "PermissionsDatabase: " << _permissionItemMap.size() << " permissions successfully loaded in " << _loadDuration.count() << " ms";
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
		std::shared_lock<std::shared_mutex> lock(_mutex);
		_PermissionItemMap::const_iterator itr = _permissionItemMap.find(permissionId);
		if(itr == _permissionItemMap.end())
			return PermissionCPtr();

		return itr->second.permissionPtr;
	}

	PermissionMap PermissionsDatabase::getUserPermissions(Dbo::ptr<User> userPtr, Wt::Auth::LoginState loginState, Dbo::Session *altSession /*= nullptr*/)
	{
		if(!altSession)
			altSession = &dboSession;

		std::shared_lock<std::shared_mutex> lock(_mutex);

		//Return default logged out permissions if not logged in
		if(!userPtr || loginState == Wt::Auth::LoginState::LoggedOut)
			return _loggedOutPermissions;

		//Default permissions
		PermissionMap granted;
		std::set<long long> denied;
		if(loginState == Wt::Auth::LoginState::Disabled)
			granted = _disabledLoginPermissions;
		else
			granted = _logggedInPermissions;

		Dbo::Transaction t(*altSession);
		UserPermissionCollection collection = userPtr->userPermissionCollection;
		for(const Dbo::ptr<UserPermission> &ptr : collection)
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

