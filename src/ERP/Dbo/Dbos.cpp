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
		dboSession.mapClass<Account>(Account::tableName());
		dboSession.mapClass<AccountEntry>(AccountEntry::tableName());
		dboSession.mapClass<OvertimeInfo>(OvertimeInfo::tableName());
		dboSession.mapClass<FineInfo>(FineInfo::tableName());
		dboSession.mapClass<PettyExpenditureInfo>(PettyExpenditureInfo::tableName());
		dboSession.mapClass<IncomeCycle>(IncomeCycle::tableName());
		dboSession.mapClass<ExpenseCycle>(ExpenseCycle::tableName());
		dboSession.mapClass<UploadedFile>(UploadedFile::tableName());
		dboSession.mapClass<AttendanceDevice>(AttendanceDevice::tableName());
		dboSession.mapClass<AttendanceDeviceV>(AttendanceDeviceV::tableName());
		dboSession.mapClass<AttendanceEntry>(AttendanceEntry::tableName());

// 		dboSession.mapClass<Inquiry>(Inquiry::tableName());
// 		dboSession.mapClass<Asset>(Asset::tableName());
// 		dboSession.mapClass<Inventory>(Inventory::tableName());
// 		dboSession.mapClass<ClothingTemplate>(ClothingTemplate::tableName());
// 		dboSession.mapClass<ClothingItem>(ClothingItem::tableName());
// 		dboSession.mapClass<AlarmTemplate>(AlarmTemplate::tableName());
// 		dboSession.mapClass<AlarmItem>(AlarmItem::tableName());
// 		dboSession.mapClass<WeaponTemplate>(WeaponTemplate::tableName());
// 		dboSession.mapClass<Weapon>(Weapon::tableName());
// 		dboSession.mapClass<VehicleTemplate>(VehicleTemplate::tableName());
// 		dboSession.mapClass<Vehicle>(Vehicle::tableName());
// 		dboSession.mapClass<AssetRegistration>(AssetRegistration::tableName());
// 		dboSession.mapClass<Office>(Office::tableName());
// 		dboSession.mapClass<RentHouse>(RentHouse::tableName());
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
