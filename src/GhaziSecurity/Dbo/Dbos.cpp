#include "Dbos.h"
#include "Application/WServer.h"
#include "Application/WApplication.h"
#include <boost/filesystem/path.hpp>

namespace GS
{
	void mapDboTree(Wt::Dbo::Session &dboSession)
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
		dboSession.mapClass<Client>(Client::tableName());
		dboSession.mapClass<Supplier>(Supplier::tableName());
		dboSession.mapClass<Employee>(Employee::tableName());
		dboSession.mapClass<Personnel>(Personnel::tableName());
		dboSession.mapClass<EmployeePosition>(EmployeePosition::tableName());
		dboSession.mapClass<ClientService>(ClientService::tableName());
		dboSession.mapClass<ContactNumber>(ContactNumber::tableName());
		dboSession.mapClass<Country>(Country::tableName());
		dboSession.mapClass<City>(City::tableName());
		dboSession.mapClass<Location>(Location::tableName());
		dboSession.mapClass<Office>(Office::tableName());
		dboSession.mapClass<RentHouse>(RentHouse::tableName());
		dboSession.mapClass<EmployeeAssignment>(EmployeeAssignment::tableName());
		dboSession.mapClass<ClientAssignment>(ClientAssignment::tableName());
		dboSession.mapClass<PersonnelAbsence>(PersonnelAbsence::tableName());
		dboSession.mapClass<PersonnelDischarge>(PersonnelDischarge::tableName());
		dboSession.mapClass<Inquiry>(Inquiry::tableName());
		dboSession.mapClass<Asset>(Asset::tableName());
		dboSession.mapClass<Inventory>(Inventory::tableName());
		dboSession.mapClass<ClothingTemplate>(ClothingTemplate::tableName());
		dboSession.mapClass<ClothingItem>(ClothingItem::tableName());
		dboSession.mapClass<AlarmTemplate>(AlarmTemplate::tableName());
		dboSession.mapClass<AlarmItem>(AlarmItem::tableName());
		dboSession.mapClass<WeaponTemplate>(WeaponTemplate::tableName());
		dboSession.mapClass<Weapon>(Weapon::tableName());
		dboSession.mapClass<VehicleTemplate>(VehicleTemplate::tableName());
		dboSession.mapClass<Vehicle>(Vehicle::tableName());
		dboSession.mapClass<AssetRegistration>(AssetRegistration::tableName());
		dboSession.mapClass<Account>(Account::tableName());
		dboSession.mapClass<AccountEntry>(AccountEntry::tableName());
		/*dboSession.mapClass<AccountEntryInfo>(AccountEntryInfo::tableName());*/
		dboSession.mapClass<OvertimeInfo>(OvertimeInfo::tableName());
		dboSession.mapClass<FineInfo>(FineInfo::tableName());
		dboSession.mapClass<PettyExpenditureInfo>(PettyExpenditureInfo::tableName());
		dboSession.mapClass<IncomeCycle>(IncomeCycle::tableName());
		dboSession.mapClass<ExpenseCycle>(ExpenseCycle::tableName());
		dboSession.mapClass<UploadedFile>(UploadedFile::tableName());
		dboSession.mapClass<AttendanceDevice>(AttendanceDevice::tableName());
		dboSession.mapClass<AttendanceEntry>(AttendanceEntry::tableName());
	}
	
	std::string UploadedFile::pathToFile() const
	{
		boost::filesystem::path result(pathToDirectory());
		result /= boost::lexical_cast<std::string>(id()) + extension;
		return result.string();
	}

	std::string UploadedFile::pathToDirectory() const
	{
		auto result = boost::filesystem::path(SERVER->appRoot()) / "uploads" / boost::lexical_cast<std::string>(entityPtr.id());
		return result.string();
	}

	std::ostream &operator<<(std::ostream &o, const UserPermissionPK &k)
	{
		return o << "(" << k.userPtr << ", " << k.permissionPtr << ")";
	}

	boost::posix_time::ptime addCycleInterval(boost::posix_time::ptime pTime, CycleInterval interval, int nIntervals)
	{
		if(interval == DailyInterval)
			pTime += boost::gregorian::days(nIntervals);
		else if(interval == WeeklyInterval)
			pTime += boost::gregorian::weeks(nIntervals);
		else if(interval == MonthlyInterval)
			pTime += boost::gregorian::months(nIntervals);
		else if(interval == YearlyInterval)
			pTime += boost::gregorian::years(nIntervals);

		return pTime;
	}

	void BaseAdminRecord::setCreatedByValues()
	{
		Wt::Dbo::ptr<User> userPtr = APP->authLogin().userPtr();
		if(userPtr)
		{
			creatorUserPtr = userPtr;
			regionPtr = userPtr->regionPtr;
		}
	}

}

Wt::Dbo::dbo_traits<GS::UserPermission>::IdType Wt::Dbo::dbo_traits<GS::UserPermission>::invalidId()
{
	return IdType();
}
