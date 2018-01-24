#ifndef GS_COMMON_H
#define GS_COMMON_H

#define BOOST_BIND_NO_PLACEHOLDERS

#include <functional>
#include <chrono>
#include <tuple>
#include <memory>

#include <Wt/WGlobal.h>
#include <Wt/WString.h>
#include <Wt/cpp17/any.hpp>
#include <Wt/Payment/Money.h>

#include <boost/optional.hpp>

#define DEFAULT_CURRENCY "PKR"

#define ADMIN_PATHC "admin"

	#define ATTENDANCE_PATHC "attendance"
		#define ATTENDANCEDEVICES_PATHC "devices"
			#define NEW_ATTENDANCEDEVICE_PATHC "add"
			#define ATTENDANCEDEVICE_PREFIX "device-"
		#define NEW_ATTENDANCEENTRY_PATHC "new-entry"
		#define ATTENDANCEENTRY_PREFIX "entry-"

	#define ENTITIES_PATHC "entities"
		#define ENTITY_PREFIX "entity-"
		#define NEW_ENTITY_PATHC "new"
		#define PERSONS_PATHC "persons"
		#define EMPLOYEES_PATHC "employees"
			#define NEW_EMPLOYEEASSIGNMENT_PATHC "assign"
			#define EMPLOYEEASSIGNMENTS_PREFIX "assignment-"
			#define EMPLOYEEASSIGNMENTS_PATHC "assignments"
		#define PERSONNEL_PATHC "personnel"
		#define BUSINESSES_PATHC "businesses"
		#define CLIENTS_PATHC "clients"
			#define NEW_CLIENTASSIGNMENT_PATHC "assign"
			#define CLIENTASSIGNMENTS_PREFIX "assignment-"
			#define CLIENTASSIGNMENTS_PATHC "assignments"

	#define ACCOUNTS_PATHC "accounts"
		#define ACCOUNT_PREFIX "account-"
		#define ACCOUNTENTRY_PREFIX "entry-"
		#define NEW_ACCOUNT_PATHC "new"
		#define NEW_ACCOUNTENTRY_PATHC "new-entry"
		#define NEW_TRANSACTION_PATHC "new-transaction"

		#define INCOMECYCLES_PATHC "incomes"
			#define INCOMECYCLE_PREFIX "income-"
			#define NEW_INCOMECYCLE_PATHC "new"
		#define EXPENSECYCLES_PATHC "expenses"
			#define EXPENSECYCLE_PREFIX "expense-"
			#define NEW_EXPENSECYCLE_PATHC "new"

	#define USERS_PATHC "users"
		#define USER_PREFIX "user-"
		#define NEW_USER_PATHC "new"
		#define REGIONS_PATHC "regions"
			#define REGION_PREFIX "region-"
			#define NEW_REGION_PATHC "new"

namespace ERP
{
	namespace Permissions
	{
		enum GSPermissions
		{
			AccessAdminPanel = 0,

			CreateRecord = 10,
			ModifyRecord = 11,
			RemoveRecord = 12,

			ViewUser = 20,
			CreateUser = 21,
			ModifyUser = 22,
			ModifyUserPermission = 23,
			RemoveUser = 24,

			ViewUnassignedUserRecord = 30,
			ViewOtherUserRecord = 31,
			ModifyOtherUserRecord = 32,
			RemoveOtherUserRecord = 33,

			ViewUnassignedRegionRecord = 40,
			ViewOtherRegionRecord = 41,
			ModifyOtherRegionRecord = 42,
			RemoveOtherRegionRecord = 43,

			ViewRegion = 50,
			CreateRegion = 51,
			ModifyRegion = 52,

			GlobalAdministrator = 1000,
			RegionalAdministrator = 1001,
			RegionalUser = 1002,
		};
	}

	inline Wt::WString tr(const std::string &key) { return Wt::WString::tr(key); }
	inline Wt::WString tr(const char *key) { return Wt::WString::tr(key); }
	inline Wt::WString trn(const std::string &key, ::uint64_t n) { return Wt::WString::trn(key, n); }
	inline Wt::WString trn(const char *key, ::uint64_t n) { return Wt::WString::trn(key, n); }
}

namespace Wt
{
	using namespace cpp17;
}
namespace ERP
{
	namespace Dbo = Wt::Dbo;
	using Wt::Payment::Money;

	using std::chrono::system_clock;
	using std::chrono::steady_clock;
	using std::chrono::high_resolution_clock;
	using std::chrono::time_point;
	using std::chrono::duration;
	using std::chrono::duration_cast;
	using std::chrono::nanoseconds;
	using std::chrono::microseconds;
	using std::chrono::milliseconds;
	using std::chrono::seconds;
	using std::chrono::minutes;
	using std::chrono::hours;

	using std::move;
	using std::unique_ptr;
	using std::shared_ptr;
	using std::make_unique;
	using std::make_shared;
	using std::static_pointer_cast;
	using std::dynamic_pointer_cast;
	using std::pair;
	using std::tuple;

	using boost::optional;
	using namespace std::placeholders;
}

#endif 