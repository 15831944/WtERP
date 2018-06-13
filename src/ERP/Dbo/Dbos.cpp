#include "Dbo/Dbos.h"
#include "Application/WServer.h"
#include "Application/WApplication.h"
#include <boost/filesystem/path.hpp>

namespace ERP
{
	void mapERPDbos(DboSession &dboSession)
	{
		dboSession.mapClass<User>(User::tableName());
		dboSession.mapClass<Region>(Region::tableName());
		dboSession.mapClass<Permission>(Permission::tableName());
		dboSession.mapClass<UserPermission>(UserPermission::tableName());
		dboSession.mapClass<DefaultPermission>(DefaultPermission::tableName());
		dboSession.mapClass<AuthInfo>("auth_info");
		dboSession.mapClass<AuthIdentity>("auth_identity");
		dboSession.mapClass<AuthToken>("auth_token");

		dboSession.mapClass<Entity>(Entity::tableName());
		dboSession.mapClass<Person>(Person::tableName());
		dboSession.mapClass<Business>(Business::tableName());
		dboSession.mapClass<EmployeePosition>(EmployeePosition::tableName());
		dboSession.mapClass<ClientService>(ClientService::tableName());
		dboSession.mapClass<ContactNumber>(ContactNumber::tableName());
		dboSession.mapClass<Country>(Country::tableName());
		dboSession.mapClass<City>(City::tableName());
		dboSession.mapClass<Location>(Location::tableName());
		dboSession.mapClass<EmployeeAssignment>(EmployeeAssignment::tableName());
		dboSession.mapClass<ClientAssignment>(ClientAssignment::tableName());
		dboSession.mapClass<ControlAccount>(ControlAccount::tableName());
		dboSession.mapClass<Account>(Account::tableName());
		dboSession.mapClass<AccountEntry>(AccountEntry::tableName());
		dboSession.mapClass<IncomeCycle>(IncomeCycle::tableName());
		dboSession.mapClass<ExpenseCycle>(ExpenseCycle::tableName());
		dboSession.mapClass<UploadedFile>(UploadedFile::tableName());
	}
	
	void createERPDboIndices(DboSession &dboSession)
	{
	}
	
	void insertERPDbos(DboSession &dboSession)
	{
		//Countries
		auto pakistan = dboSession.addNew<Country>("PK", "Pakistan");
		//Cities
		auto karachi = dboSession.addNew<City>(pakistan, "Karachi");
		auto hyderabad = dboSession.addNew<City>(pakistan, "Hyderabad");
		auto sukhur = dboSession.addNew<City>(pakistan, "Sukhur");
		auto Quetta = dboSession.addNew<City>(pakistan, "Quetta");
		auto Multan = dboSession.addNew<City>(pakistan, "Multan");
		auto Lahore = dboSession.addNew<City>(pakistan, "Lahore");
		auto Islamabad = dboSession.addNew<City>(pakistan, "Islamabad");
		auto Peshawar = dboSession.addNew<City>(pakistan, "Peshawar");
		auto Faisalabad = dboSession.addNew<City>(pakistan, "Faisalabad");
		
		//Permissions
		auto accessAdminPanel = dboSession.addNew<Permission>(Permissions::AccessAdminPanel, "Access Admin Panel");
		
		auto createRecord = dboSession.addNew<Permission>(Permissions::CreateRecord, "Create records");
		auto modifyRecord = dboSession.addNew<Permission>(Permissions::ModifyRecord, "Modify records");
		auto removeRecord = dboSession.addNew<Permission>(Permissions::RemoveRecord, "Remove records");
		
		auto viewUser = dboSession.addNew<Permission>(Permissions::ViewUser, "View users");
		auto createUser = dboSession.addNew<Permission>(Permissions::CreateUser, "Create users");
		auto modifyUser = dboSession.addNew<Permission>(Permissions::ModifyUser, "Modify users");
		auto modifyUserPermission = dboSession.addNew<Permission>(Permissions::ModifyUserPermission, "Modify user permissions");
		auto removeUser = dboSession.addNew<Permission>(Permissions::RemoveUser, "Remove users");
		
		auto viewUnassignedUserRecord = dboSession.addNew<Permission>(Permissions::ViewUnassignedUserRecord, "View records not assigned to any user");
		auto viewOtherUserRecord = dboSession.addNew<Permission>(Permissions::ViewOtherUserRecord, "View records created by other users");
		auto modifyOtherUserRecord = dboSession.addNew<Permission>(Permissions::ModifyOtherUserRecord, "Modify records created by other users");
		auto removeOtherUserRecord = dboSession.addNew<Permission>(Permissions::RemoveOtherUserRecord, "Remove records created by other users");
		
		auto viewUnassignedRegionRecord = dboSession.addNew<Permission>(Permissions::ViewUnassignedRegionRecord, "View records not assigned to any region");
		auto viewOtherRegionRecord = dboSession.addNew<Permission>(Permissions::ViewOtherRegionRecord, "View records created by other regions");
		auto modifyOtherRegionRecord = dboSession.addNew<Permission>(Permissions::ModifyOtherRegionRecord, "Modify records created by other regions");
		auto removeOtherRegionRecord = dboSession.addNew<Permission>(Permissions::RemoveOtherRegionRecord, "Remove records created by other regions");
		
		auto viewRegion = dboSession.addNew<Permission>(Permissions::ViewRegion, "View regions");
		auto createRegion = dboSession.addNew<Permission>(Permissions::CreateRegion, "Create regions");
		auto modifyRegion = dboSession.addNew<Permission>(Permissions::ModifyRegion, "Modify regions");
		
		auto globalAdministrator = dboSession.addNew<Permission>(Permissions::GlobalAdministrator, "Global administrator permissions");
		globalAdministrator.modify()->linkedToCollection.insert(accessAdminPanel);
		globalAdministrator.modify()->linkedToCollection.insert(createRecord);
		globalAdministrator.modify()->linkedToCollection.insert(modifyRecord);
		globalAdministrator.modify()->linkedToCollection.insert(removeRecord);
		globalAdministrator.modify()->linkedToCollection.insert(viewUser);
		globalAdministrator.modify()->linkedToCollection.insert(createUser);
		globalAdministrator.modify()->linkedToCollection.insert(modifyUser);
		globalAdministrator.modify()->linkedToCollection.insert(modifyUserPermission);
		globalAdministrator.modify()->linkedToCollection.insert(removeUser);
		globalAdministrator.modify()->linkedToCollection.insert(viewUnassignedUserRecord);
		globalAdministrator.modify()->linkedToCollection.insert(viewOtherUserRecord);
		globalAdministrator.modify()->linkedToCollection.insert(modifyOtherUserRecord);
		globalAdministrator.modify()->linkedToCollection.insert(removeOtherUserRecord);
		globalAdministrator.modify()->linkedToCollection.insert(viewUnassignedRegionRecord);
		globalAdministrator.modify()->linkedToCollection.insert(viewOtherRegionRecord);
		globalAdministrator.modify()->linkedToCollection.insert(modifyOtherRegionRecord);
		globalAdministrator.modify()->linkedToCollection.insert(removeOtherRegionRecord);
		globalAdministrator.modify()->linkedToCollection.insert(viewRegion);
		globalAdministrator.modify()->linkedToCollection.insert(createRegion);
		globalAdministrator.modify()->linkedToCollection.insert(modifyRegion);
		
		auto regionalAdministrator = dboSession.addNew<Permission>(Permissions::RegionalAdministrator, "Regional administrator permissions");
		regionalAdministrator.modify()->linkedToCollection.insert(accessAdminPanel);
		regionalAdministrator.modify()->linkedToCollection.insert(createRecord);
		regionalAdministrator.modify()->linkedToCollection.insert(modifyRecord);
		regionalAdministrator.modify()->linkedToCollection.insert(removeRecord);
		regionalAdministrator.modify()->linkedToCollection.insert(viewUser);
		regionalAdministrator.modify()->linkedToCollection.insert(createUser);
		regionalAdministrator.modify()->linkedToCollection.insert(modifyUser);
		regionalAdministrator.modify()->linkedToCollection.insert(modifyUserPermission);
		regionalAdministrator.modify()->linkedToCollection.insert(removeUser);
		regionalAdministrator.modify()->linkedToCollection.insert(viewUnassignedUserRecord);
		regionalAdministrator.modify()->linkedToCollection.insert(viewOtherUserRecord);
		regionalAdministrator.modify()->linkedToCollection.insert(modifyOtherUserRecord);
		regionalAdministrator.modify()->linkedToCollection.insert(removeOtherUserRecord);
		regionalAdministrator.modify()->linkedToCollection.insert(viewUnassignedRegionRecord);
		
		auto regionalUser = dboSession.addNew<Permission>(Permissions::RegionalUser, "Regional user permissions");
		regionalUser.modify()->linkedToCollection.insert(accessAdminPanel);
		regionalUser.modify()->linkedToCollection.insert(createRecord);
		regionalUser.modify()->linkedToCollection.insert(modifyRecord);
		regionalUser.modify()->linkedToCollection.insert(removeRecord);
		regionalUser.modify()->linkedToCollection.insert(viewUnassignedUserRecord);
		regionalUser.modify()->linkedToCollection.insert(viewOtherUserRecord);
		regionalUser.modify()->linkedToCollection.insert(viewUnassignedRegionRecord);
		
		//Users
		{
			WServer *server = SERVER;
			UserDatabase userDatabase(dboSession);
			Wt::Auth::User authUser;
			Dbo::ptr<AuthInfo> authInfoPtr;
			Dbo::ptr<User> userPtr;
			
			authUser = userDatabase.registerNew();
			authUser.setIdentity(Wt::Auth::Identity::LoginName, "admin");
			server->getPasswordService().updatePassword(authUser, "changeme");
			authInfoPtr = userDatabase.find(authUser);
			userPtr = dboSession.addNew<User>();
			authInfoPtr.modify()->setUser(userPtr);
			dboSession.addNew<UserPermission>(userPtr, globalAdministrator);
			
// 			authUser = userDatabase.registerNew();
// 			authUser.setIdentity(Wt::Auth::Identity::LoginName, "regionaladmin");
// 			server->getPasswordService().updatePassword(authUser, "changeme");
// 			authInfoPtr = userDatabase.find(authUser);
// 			userPtr = dboSession.addNew<User>();
// 			authInfoPtr.modify()->setUser(userPtr);
// 			dboSession.addNew<UserPermission>(userPtr, regionalAdministrator);
//
// 			authUser = userDatabase.registerNew();
// 			authUser.setIdentity(Wt::Auth::Identity::LoginName, "regionaluser");
// 			server->getPasswordService().updatePassword(authUser, "changeme");
// 			authInfoPtr = userDatabase.find(authUser);
// 			userPtr = dboSession.addNew<User>();
// 			authInfoPtr.modify()->setUser(userPtr);
// 			dboSession.addNew<UserPermission>(userPtr, regionalUser);
//
// 			authUser = userDatabase.registerNew();
// 			authUser.setIdentity(Wt::Auth::Identity::LoginName, "test");
// 			server->getPasswordService().updatePassword(authUser, "changeme");
// 			authInfoPtr = userDatabase.find(authUser);
// 			userPtr = dboSession.addNew<User>();
// 			authInfoPtr.modify()->setUser(userPtr);
		}
	}

	void RestrictedRecordDbo::setCreatedByValues(bool setRegion)
	{
		WApplication *app = APP;
		if(app->authLogin().userPtr())
		{
			_creatorUserPtr = app->authLogin().userPtr();

			if(setRegion)
				_regionPtr = app->authLogin().userPtr()->regionPtr();
		}
	}

	void BaseRecordVersionDbo::setModifiedByValues()
	{
		WApplication *app = APP;
		if(app->authLogin().userPtr())
			_modifierUserPtr = app->authLogin().userPtr();
	}

	std::string UploadedFile::pathToFile() const
	{
		boost::filesystem::path result(pathToDirectory());
		result /= std::to_string(id()) + extension;
		return result.string();
	}

	std::string UploadedFile::pathToDirectory() const
	{
		auto result = boost::filesystem::path(SERVER->appRoot()) / "uploads" / std::to_string(entityPtr.id());
		return result.string();
	}

	std::ostream &operator<<(std::ostream &o, const UserPermissionPK &k)
	{
		return o << "(" << k.userPtr << ", " << k.permissionPtr << ")";
	}

	Wt::WDateTime addCycleInterval(const Wt::WDateTime &dt, CycleInterval interval, int nIntervals)
	{
		switch(interval)
		{
		case DailyInterval: return dt.addDays(nIntervals);
		case WeeklyInterval: return dt.addDays(nIntervals*7);
		case MonthlyInterval: return dt.addMonths(nIntervals);
		case YearlyInterval: return dt.addYears(nIntervals);
		default: throw std::logic_error("addCycleInterval(): Invalid CycleInterval");
		}
	}

	Wt::WString rsEveryNIntervals(const Money &amount, CycleInterval interval, uint64_t nIntervals)
	{
		switch(interval)
		{
		case DailyInterval: return trn("RsEveryNDays", nIntervals).arg(Wt::WLocale::currentLocale().toString(amount)).arg(nIntervals);
		case WeeklyInterval: return trn("RsEveryNWeeks", nIntervals).arg(Wt::WLocale::currentLocale().toString(amount)).arg(nIntervals);
		case MonthlyInterval: return trn("RsEveryNMonths", nIntervals).arg(Wt::WLocale::currentLocale().toString(amount)).arg(nIntervals);
		case YearlyInterval: return trn("RsEveryNYears", nIntervals).arg(Wt::WLocale::currentLocale().toString(amount)).arg(nIntervals);
		default: return Wt::WString::Empty;
		}
	}
}

Wt::Dbo::dbo_traits<ERP::UserPermission>::IdType Wt::Dbo::dbo_traits<ERP::UserPermission>::invalidId()
{
	return IdType();
}
