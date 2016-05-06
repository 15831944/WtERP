#ifndef GS_COMMON_H
#define GS_COMMON_H

#define DEFAULT_CURRENCY "PKR"

#define ADMIN_PATHC "admin"

	#define SYSTEM_PATHC "system"
		#define ATTENDANCEDEVICES_PATHC "attendance-devices"
			#define NEW_ATTENDANCEDEVICE_PATHC "new"
			#define ATTENDANCEDEVICE_PREFIX "device-"

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

namespace GS
{
	namespace Permissions
	{
		enum GSPermissions
		{
			AccessAdminPanel = 0,

			CreateRecord = 10,
			ModifyRecord = 11,
			RemoveRecord = 12,

			CreateUser = 20,
			ModifyUser = 21,
			ModifyUserPermission = 22,
			RemoveUser = 23,

			ViewUnassignedUserRecord = 30,
			ViewOtherUserRecord = 31,
			ModifyOtherUserRecord = 32,
			RemoveOtherUserRecord = 33,

			ViewUnassignedRegionRecord = 40,
			ViewOtherRegionRecord = 41,
			ModifyOtherRegionRecord = 42,
			RemoveOtherRegionRecord = 43,

			GlobalAdministrator = 1000,
			RegionalAdministrator = 1001,
			RegionalUser = 1002,
		};
	}

}

#endif 