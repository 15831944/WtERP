#ifndef GS_COMMON_H
#define GS_COMMON_H

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

#define SET_TRANSIENT_VIEW_DIALOG(dialog) dialog->setClosable(true);\
	dialog->resize(Wt::WLength(85, Wt::WLength::Percentage), Wt::WLength(95, Wt::WLength::Percentage));\
	dialog->setTransient(true);\
	dialog->setDeleteWhenHidden(true);\
	dialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);

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

}

#endif 