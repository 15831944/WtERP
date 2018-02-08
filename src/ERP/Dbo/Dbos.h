#ifndef ERP_DBOS_H
#define ERP_DBOS_H

#include "Common.h"
#include "Application/DboSession.h"
#include <Wt/Dbo/Dbo.h>
#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/WDate.h>
#include <Wt/WAny.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/SqlTraits.h>

#define TRANSACTION(app) Dbo::Transaction t((app)->dboSession())

//Utilities, forward declarations and typedefs
namespace ERP
{
	//Utilities
	void mapERPDbos(DboSession &dboSession);

	//User related
	class User;
	typedef Dbo::collection<Dbo::ptr<User>> UserCollection;

	class Region;
	typedef Dbo::collection<Dbo::ptr<Region>> RegionCollection;

	class Permission;
	typedef Dbo::collection<Dbo::ptr<Permission>> PermissionCollection;

	struct PermissionDdo;
	typedef std::map<long long, shared_ptr<const PermissionDdo>> PermissionMap;

	struct UserPermissionPK;
	class UserPermission;
	class DefaultPermission;
	typedef Dbo::collection<Dbo::ptr<UserPermission>> UserPermissionCollection;
	typedef Dbo::collection<Dbo::ptr<DefaultPermission>> DefaultPermissionCollection;

	typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
	typedef Wt::Auth::Dbo::AuthIdentity<AuthInfo> AuthIdentity;
	typedef Wt::Auth::Dbo::AuthToken<AuthInfo> AuthToken;
	typedef Dbo::collection<Dbo::ptr<AuthInfo>> AuthInfoCollection;
	typedef Wt::Auth::Dbo::UserDatabase<ERP::AuthInfo> UserDatabase;

	//The rest
	class Entity;
	typedef Dbo::collection<Dbo::ptr<Entity>> EntityCollection;

	class Person;
	typedef Dbo::collection<Dbo::ptr<Person>> PersonCollection;

	class Business;
	typedef Dbo::collection<Dbo::ptr<Business>> BusinessCollection;

	class EmployeePosition;
	typedef Dbo::collection<Dbo::ptr<EmployeePosition>> EmployeePositionCollection;

	class ClientService;
	typedef Dbo::collection<Dbo::ptr<ClientService>> ClientServiceCollection;

	class ContactNumber;
	typedef Dbo::collection<Dbo::ptr<ContactNumber>> ContactNumberCollection;

	class Country;
	typedef Dbo::collection<Dbo::ptr<Country>> CountryCollection;

	class City;
	typedef Dbo::collection<Dbo::ptr<City>> CityCollection;

	class Location;
	typedef Dbo::collection<Dbo::ptr<Location>> LocationCollection;

	class EmployeeAssignment;
	typedef Dbo::collection<Dbo::ptr<EmployeeAssignment>> EmployeeAssignmentCollection;

	class ClientAssignment;
	typedef Dbo::collection<Dbo::ptr<ClientAssignment>> ClientAssignmentCollection;

	class Account;
	typedef Dbo::collection<Dbo::ptr<Account>> AccountCollection;

	class AccountEntry;
	typedef Dbo::collection<Dbo::ptr<AccountEntry>> AccountEntryCollection;

	class OvertimeInfo;
	typedef Dbo::collection<Dbo::ptr<OvertimeInfo>> OvertimeInfoCollection;

	class FineInfo;
	typedef Dbo::collection<Dbo::ptr<FineInfo>> FineInfoCollection;

	class PettyExpenditureInfo;
	typedef Dbo::collection<Dbo::ptr<PettyExpenditureInfo>> PettyExpenditureInfoCollection;

	class IncomeCycle;
	typedef Dbo::collection<Dbo::ptr<IncomeCycle>> IncomeCycleCollection;

	class ExpenseCycle;
	typedef Dbo::collection<Dbo::ptr<ExpenseCycle>> ExpenseCycleCollection;

	class UploadedFile;
	typedef Dbo::collection<Dbo::ptr<UploadedFile>> UploadedFileCollection;

	class AttendanceDevice;
	class AttendanceDeviceV;
	typedef Dbo::collection<Dbo::ptr<AttendanceDevice>> AttendanceDeviceCollection;
	typedef Dbo::collection<Dbo::ptr<AttendanceDeviceV>> AttendanceDeviceVCollection;

	class AttendanceEntry;
	typedef Dbo::collection<Dbo::ptr<AttendanceEntry>> AttendanceEntryCollection;

//	class Inquiry;
// 	typedef Dbo::collection<Dbo::ptr<Inquiry>> InquiryCollection;
}

//Dbo traits
namespace Wt
{
	namespace Dbo
	{
		//User Permissions
		template <class Action>
		void field(Action &a, ERP::UserPermissionPK &key, const std::string &, int size = -1);

		template<>
		struct dbo_traits<ERP::UserPermission> : public dbo_default_traits
		{
			typedef ERP::UserPermissionPK IdType;
			static IdType invalidId();
			static const char *surrogateIdField() { return nullptr; }
		};

		template<>
		struct dbo_traits<ERP::Permission> : public dbo_default_traits
		{
			static const char *surrogateIdField() { return nullptr; }
		};

		//Entities
		template<>
		struct dbo_traits<ERP::Person> : public dbo_default_traits
		{
			typedef ptr<ERP::Entity> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<ERP::Business> : public dbo_default_traits
		{
			typedef ptr<ERP::Entity> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<ERP::Country> : public dbo_default_traits
		{
			typedef std::string IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};

		//Accounts
		template<>
		struct dbo_traits<ERP::OvertimeInfo> : public dbo_default_traits
		{
			typedef ptr<ERP::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<ERP::FineInfo> : public dbo_default_traits
		{
			typedef ptr<ERP::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<ERP::PettyExpenditureInfo> : public dbo_default_traits
		{
			typedef ptr<ERP::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
	}
}

//Dbos
namespace ERP
{
	class BaseRecordDbo
	{
	private:
		Wt::WDateTime _timestamp = Wt::WDateTime::currentDateTime();

		friend class AccountsDatabase;

	public:
		Wt::WDateTime timestamp() const { return _timestamp; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, _timestamp, "timestamp");
		}
	};

	class RestrictedRecordDbo : public BaseRecordDbo
	{
	private:
		Dbo::ptr<User> _creatorUserPtr;
		Dbo::ptr<Region> _regionPtr;

		friend class AccountsDatabase;
		friend class UserFormModel;

	public:
		Dbo::ptr<User> creatorUserPtr() const { return _creatorUserPtr; }
		Dbo::ptr<Region> regionPtr() const { return _regionPtr; }
		void setCreatedByValues(bool setRegion = true);

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, _creatorUserPtr, "creator_user", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, _regionPtr, "region", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			BaseRecordDbo::persist(a);
		}
	};

	class BaseRecordVersionDbo : public BaseRecordDbo
	{
	private:
		Dbo::ptr<User> _modifierUserPtr;

	public:
		Dbo::ptr<User> modifierUserPtr() const { return _modifierUserPtr; }
		void setModifiedByValues();

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, _modifierUserPtr, "modifier_user", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			BaseRecordDbo::persist(a);
		}
	};

	template<class C>
	class RecordVersionDbo : public BaseRecordVersionDbo
	{
	private:
		Dbo::ptr<C> _parentPtr;

	public:
		RecordVersionDbo(Dbo::ptr<C> parentPtr = nullptr) : _parentPtr(move(parentPtr)) { }
		Dbo::ptr<C> parentPtr() const { return _parentPtr; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, _parentPtr, "parent", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			BaseRecordVersionDbo::persist(a);
		}
	};

	class Permission
	{
	private:
		long long _id = -1;

	public:
		Permission() = default;
		Permission(long long id) : _id(id) { }
		Permission(long long id, std::string name) : _id(id), name(move(name)) { }

		std::string name;
		bool requireStrongLogin = false;

		UserPermissionCollection userPermissionCollection;
		DefaultPermissionCollection defaultPermissionCollection;
		PermissionCollection linkedToCollection;
		PermissionCollection linkedByCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, _id);
			Dbo::field(a, name, "name", 70);
			Dbo::field(a, requireStrongLogin, "requireStrongLogin");
			Dbo::hasMany(a, userPermissionCollection, Dbo::ManyToOne, "permission");
			Dbo::hasMany(a, defaultPermissionCollection, Dbo::ManyToOne, "permission");
			Dbo::hasMany(a, linkedToCollection, Dbo::ManyToMany, "linked_permission", "to");
			Dbo::hasMany(a, linkedByCollection, Dbo::ManyToMany, "linked_permission", "by");
		}
		DEFINE_DBO_TABLENAME("permission");
	};
	struct PermissionDdo
	{
		long long id = -1;
		std::string name;
		bool requireStrongLogin = false;

		PermissionDdo(const Dbo::ptr<Permission> &ptr)
				: id(ptr.id()), name(ptr->name), requireStrongLogin(ptr->requireStrongLogin)
		{ }
	};

	struct UserPermissionPK
	{
		Dbo::ptr<User> userPtr;
		Dbo::ptr<Permission> permissionPtr;

		UserPermissionPK() = default;
		UserPermissionPK(Dbo::ptr<User> userPtr, Dbo::ptr<Permission> permissionPtr)
				: userPtr(move(userPtr)), permissionPtr(move(permissionPtr))
		{ }

		bool operator==(const UserPermissionPK &other) const
		{
			return userPtr == other.userPtr && permissionPtr == other.permissionPtr;
		}

		bool operator<(const UserPermissionPK &other) const
		{
			if(userPtr < other.userPtr)
				return true;
			else if(userPtr == other.userPtr)
				return permissionPtr < other.permissionPtr;
			else
				return false;
		}
	};

	class UserPermission
	{
	public:
		bool denied = false;

		UserPermission() = default;
		UserPermission(Dbo::ptr<User> userPtr, Dbo::ptr<Permission> permissionPtr)
		{
			_id.userPtr = move(userPtr);
			_id.permissionPtr = move(permissionPtr);
		}

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _id, "id");
			Dbo::field(a, denied, "denied");
		}
		DEFINE_DBO_TABLENAME("user_permission");

	private:
		UserPermissionPK _id;
	};
	std::ostream &operator<<(std::ostream &o, const UserPermissionPK &k);

	class DefaultPermission
	{
	public:
		enum LoginStates
		{
			LoggedOut		= 0x01,
			DisabledLogin	= 0x02,
			LoggedIn		= 0x04,
			AllStates		= 0xFF
		};

		Dbo::ptr<Permission> permissionPtr;
		LoginStates loginStates = AllStates;

		template<class Action>void persist(Action &a)
		{
			Dbo::belongsTo(a, permissionPtr, "permission", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, loginStates, "loginStates");
		}
		DEFINE_DBO_TABLENAME("default_permission");
	};

	class User : public RestrictedRecordDbo
	{
	public:
		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" USERS_PATHC "/" NEW_USER_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" USERS_PATHC "/" USER_PREFIX + idStr; }

		Dbo::weak_ptr<AuthInfo> authInfoWPtr;

		UserPermissionCollection userPermissionCollection;

		EntityCollection entitiesCollection;
		AccountCollection accountsCollection;
		AccountEntryCollection accountEntriesCollection;
		IncomeCycleCollection incomeCyclesCollection;
		ExpenseCycleCollection expenseCyclesCollection;
		UserCollection createdUserCollection;

		AttendanceDeviceVCollection attendanceDeviceVCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::hasOne(a, authInfoWPtr, "user");
			Dbo::hasMany(a, userPermissionCollection, Dbo::ManyToOne, "user");

			Dbo::hasMany(a, entitiesCollection, Dbo::ManyToOne, "creator_user");
			Dbo::hasMany(a, accountsCollection, Dbo::ManyToOne, "creator_user");
			Dbo::hasMany(a, accountEntriesCollection, Dbo::ManyToOne, "creator_user");
			Dbo::hasMany(a, incomeCyclesCollection, Dbo::ManyToOne, "creator_user");
			Dbo::hasMany(a, expenseCyclesCollection, Dbo::ManyToOne, "creator_user");
			Dbo::hasMany(a, createdUserCollection, Dbo::ManyToOne, "creator_user");

			Dbo::hasMany(a, attendanceDeviceVCollection, Dbo::ManyToOne, "modifier_user");

			RestrictedRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("user");
	};

	class Region : public BaseRecordDbo
	{
	public:
		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" USERS_PATHC "/" REGIONS_PATHC "/" NEW_REGION_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" USERS_PATHC "/" REGIONS_PATHC "/" REGION_PREFIX + idStr; }

		std::string name;

		UserCollection userCollection;
		EntityCollection entitiesCollection;
		AccountCollection accountsCollection;
		AccountEntryCollection accountEntriesCollection;
		IncomeCycleCollection incomeCyclesCollection;
		ExpenseCycleCollection expenseCyclesCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, name, "name", 70);
			Dbo::hasMany(a, userCollection, Dbo::ManyToOne, "region");

			Dbo::hasMany(a, entitiesCollection, Dbo::ManyToOne, "region");
			Dbo::hasMany(a, accountsCollection, Dbo::ManyToOne, "region");
			Dbo::hasMany(a, accountEntriesCollection, Dbo::ManyToOne, "region");
			Dbo::hasMany(a, incomeCyclesCollection, Dbo::ManyToOne, "region");
			Dbo::hasMany(a, expenseCyclesCollection, Dbo::ManyToOne, "region");

			BaseRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("region");
	};

	enum BloodType
	{
		UnknownBT = 0,
		OPositive = 1,
		ONegative = 2,
		APositive = 3,
		ANegative = 4,
		BPositive = 5,
		BNegative = 6,
		ABPositive = 7,
		ABNegative = 8
	};
	enum MaritalStatus
	{
		UnknownMS = 0,
		Married = 1,
		Unmarried = 2
	};

	class Entity : public RestrictedRecordDbo
	{
	public:
		enum Type
		{
			InvalidType = -1,
			PersonType = 0,
			BusinessType = 1
		};
		enum SpecificType
		{
			UnspecificType	= 0x00,
			EmployeeType	= 0x01,
			ClientType		= 0x02
		};

		Entity() = default;
		Entity(Type type)
			: type(type)
		{ }

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" NEW_ENTITY_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" ENTITY_PREFIX + idStr; }
		static Wt::WString typeToArgString(Type type)
		{
			switch(type)
			{
			case PersonType: return tr("person");
			case BusinessType: return tr("business");
			default: return tr("entity");
			}
		}

		std::string name;
		Type type = InvalidType;
		Dbo::ptr<Account> balAccountPtr;
		Dbo::ptr<Account> pnlAccountPtr;

		Dbo::weak_ptr<Person> personWPtr;
		Dbo::weak_ptr<Business> businessWPtr;

		ContactNumberCollection contactNumberCollection;
		LocationCollection locationCollection;
		UploadedFileCollection uploadedFileCollection;
		IncomeCycleCollection incomeCycleCollection;
		ExpenseCycleCollection expenseCycleCollection;
		EmployeeAssignmentCollection employeeAssignmentCollection;
		ClientAssignmentCollection clientAssignmentCollection;
		AttendanceEntryCollection attendanceCollection;
// 		InquiryCollection inquiryCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, name, "name", 70);
			Dbo::field(a, type, "type");
			Dbo::belongsTo(a, balAccountPtr, "bal_account", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, pnlAccountPtr, "pnl_account", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			Dbo::hasOne(a, personWPtr, "entity");
			Dbo::hasOne(a, businessWPtr, "entity");

			Dbo::hasMany(a, contactNumberCollection, Dbo::ManyToOne, "entity");
			Dbo::hasMany(a, locationCollection, Dbo::ManyToOne, "entity");
			Dbo::hasMany(a, uploadedFileCollection, Dbo::ManyToOne, "entity");
			Dbo::hasMany(a, incomeCycleCollection, Dbo::ManyToOne, "entity");
			Dbo::hasMany(a, expenseCycleCollection, Dbo::ManyToOne, "entity");
			Dbo::hasMany(a, employeeAssignmentCollection, Dbo::ManyToOne, "entity");
			Dbo::hasMany(a, clientAssignmentCollection, Dbo::ManyToOne, "entity");
			Dbo::hasMany(a, attendanceCollection, Dbo::ManyToOne, "entity");
// 			Dbo::hasMany(a, inquiryCollection, Dbo::ManyToOne, "entity");

			RestrictedRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("entity");
	};
	class Person
	{
	private:
		Dbo::ptr<Entity> _entityPtr;

	public:
		Dbo::ptr<Entity> entityPtr() const { return _entityPtr; }
		Wt::WDate dateOfBirth;
		std::string cnicNumber;
		std::string motherTongue;
		std::string identificationMark;
		float height = -1;
		BloodType bloodType = UnknownBT;
		MaritalStatus maritalStatus = UnknownMS;
		std::string remarks;

		Dbo::ptr<Person> fatherPersonPtr;
		Dbo::ptr<Person> motherPersonPtr;
		PersonCollection fatherChildrenCollection;
		PersonCollection motherChildrenCollection;

		PersonCollection nextOfKinCollection;
		Dbo::ptr<Person> nextOfKinOfPtr;

		Dbo::ptr<UploadedFile> profilePictureFilePtr;
		Dbo::ptr<UploadedFile> cnicFile1Ptr;
		Dbo::ptr<UploadedFile> cnicFile2Ptr;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, _entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, dateOfBirth, "dateOfBirth");
			Dbo::field(a, cnicNumber, "cnicNumber", 13);
			Dbo::field(a, motherTongue, "motherTongue", 70);
			Dbo::field(a, identificationMark, "identificationMark", 255);
			Dbo::field(a, height, "height");
			Dbo::field(a, bloodType, "bloodType");
			Dbo::field(a, maritalStatus, "maritalStatus");
			Dbo::field(a, remarks, "remarks");

			Dbo::belongsTo(a, fatherPersonPtr, "father_person", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, motherPersonPtr, "mother_person", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::hasMany(a, fatherChildrenCollection, Dbo::ManyToOne, "father_person");
			Dbo::hasMany(a, motherChildrenCollection, Dbo::ManyToOne, "mother_person");

			Dbo::hasMany(a, nextOfKinCollection, Dbo::ManyToOne, "nextOfKin_person");
			Dbo::belongsTo(a, nextOfKinOfPtr, "nextOfKin_person", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			Dbo::belongsTo(a, profilePictureFilePtr, "profilePictureFile", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, cnicFile1Ptr, "cnicFile1", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, cnicFile2Ptr, "cnicFile2", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
		}
		DEFINE_DBO_TABLENAME("person");

	private:
		friend class PersonFormModel;
	};

	class EmployeePosition : public BaseRecordDbo
	{
	public:
		std::string title;
		EmployeeAssignmentCollection employeeAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, title, "title", 70);
			Dbo::hasMany(a, employeeAssignmentCollection, Dbo::ManyToOne, "employeeposition");

			BaseRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("employeeposition");
	};

	class ClientService : public BaseRecordDbo
	{
	public:
		std::string title;
		ClientAssignmentCollection clientAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, title, "title", 70);
			Dbo::hasMany(a, clientAssignmentCollection, Dbo::ManyToOne, "clientservice");

			BaseRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("clientservice");
	};

	class Business
	{
	private:
		Dbo::ptr<Entity> _entityPtr;

	public:
		Dbo::ptr<Entity> entityPtr() const { return _entityPtr; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, _entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
		}
		DEFINE_DBO_TABLENAME("business");

	private:
		friend class BusinessFormModel;
	};

	class ContactNumber : public BaseRecordDbo
	{
	public:
		Dbo::ptr<Entity> entityPtr;
		std::string countryCode;
		std::string nationalNumber;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, countryCode, "countryCode", 3);
			Dbo::field(a, nationalNumber, "nationalNumber", 15);

			BaseRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("contactnumber");
	};
	class Country : public BaseRecordDbo
	{
	public:
		std::string code;
		std::string name;
		CityCollection cityCollection;
		
		Country() = default;
		Country(std::string code, std::string name = "") : code(move(code)), name(move(name)) { }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, code, "code", 3);
			Dbo::field(a, name, "name", 70);
			Dbo::hasMany(a, cityCollection, Dbo::ManyToOne, "country");

			BaseRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("country");
	};
	class City : public BaseRecordDbo
	{
	public:
		Dbo::ptr<Country> countryPtr;
		std::string name;

		City() = default;
		City(Dbo::ptr<Country> countryPtr, std::string name = "") : countryPtr(move(countryPtr)), name(move(name)) { }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, countryPtr, "country", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, name, "name", 70);

			BaseRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("city");
	};
	class Location : public BaseRecordDbo
	{
	public:
		Dbo::ptr<Entity> entityPtr;
		std::string address;
		Dbo::ptr<Country> countryPtr;
		Dbo::ptr<City> cityPtr;

		EmployeeAssignmentCollection assignedEmployeeCollection;
		AttendanceDeviceVCollection attendanceDeviceVCollection;
		AttendanceEntryCollection attendanceCollection;
// 		InquiryCollection inquiryCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::field(a, address, "address");
			Dbo::belongsTo(a, countryPtr, "country", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, cityPtr, "city", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			Dbo::hasMany(a, assignedEmployeeCollection, Dbo::ManyToOne, "location");
			Dbo::hasMany(a, attendanceDeviceVCollection, Dbo::ManyToOne, "location");
			Dbo::hasMany(a, attendanceCollection, Dbo::ManyToOne, "location");
// 			Dbo::hasMany(a, inquiryCollection, Dbo::ManyToOne, "location");

			BaseRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("location");
	};

	class EmployeeAssignment : public RestrictedRecordDbo
	{
	public:
		Wt::WDate startDate;
		Wt::WDate endDate;
		std::string description;
		Dbo::ptr<EmployeePosition> positionPtr;
		Dbo::ptr<Entity> entityPtr;
		Dbo::ptr<Location> locationPtr;
		Dbo::ptr<ExpenseCycle> expenseCyclePtr;
		Dbo::ptr<ClientAssignment> clientAssignmentPtr;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" EMPLOYEES_PATHC "/" NEW_EMPLOYEEASSIGNMENT_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" EMPLOYEES_PATHC "/" EMPLOYEEASSIGNMENTS_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, startDate, "startDate");
			Dbo::field(a, endDate, "endDate");
			Dbo::field(a, description, "description");
			Dbo::belongsTo(a, positionPtr, "employeeposition", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, expenseCyclePtr, "expensecycle", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, clientAssignmentPtr, "clientassignment", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			RestrictedRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("employeeassignment");
	};

	class ClientAssignment : public RestrictedRecordDbo
	{
	public:
		Wt::WDate startDate;
		Wt::WDate endDate;
		std::string description;
		Dbo::ptr<ClientService> servicePtr;
		Dbo::ptr<Entity> entityPtr;
		Dbo::ptr<IncomeCycle> incomeCyclePtr;

		EmployeeAssignmentCollection employeeAssignmentCollection;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" EMPLOYEES_PATHC "/" NEW_CLIENTASSIGNMENT_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" CLIENTS_PATHC "/" CLIENTASSIGNMENTS_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, startDate, "startDate");
			Dbo::field(a, endDate, "endDate");
			Dbo::field(a, description, "description");
			Dbo::belongsTo(a, servicePtr, "clientservice", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::belongsTo(a, incomeCyclePtr, "incomecycle", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			Dbo::hasMany(a, employeeAssignmentCollection, Dbo::ManyToOne, "clientassignment");

			RestrictedRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("clientassignment");
	};

	class Account : public RestrictedRecordDbo
	{
	public:
		enum Type
		{
			Asset = 0,
			Liability = 1,
			EntityBalanceAccount = 100,
			EntityPnlAccount = 101
		};

		Account() = default;
		Account &operator=(const Account &other) = default;
		Account(Type t) : type(t) { }

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" NEW_ACCOUNT_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" ACCOUNT_PREFIX + idStr; }

		std::string name;
		Type type = Asset;
		Money balance() const { return Money(_balanceInCents, DEFAULT_CURRENCY); }
		long long balanceInCents() const { return _balanceInCents; }

		AccountEntryCollection debitEntryCollection;
		AccountEntryCollection creditEntryCollection;
		Dbo::weak_ptr<Entity> balOfEntityWPtr;
		Dbo::weak_ptr<Entity> pnlOfEntityWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, name, "name", 70);
			Dbo::field(a, type, "type");
			Dbo::field(a, _currency, "currency", 3);
			Dbo::field(a, _balanceInCents, "balance");

			Dbo::hasMany(a, debitEntryCollection, Dbo::ManyToOne, "debit_account");
			Dbo::hasMany(a, creditEntryCollection, Dbo::ManyToOne, "credit_account");
			Dbo::hasOne(a, balOfEntityWPtr, "bal_account");
			Dbo::hasOne(a, pnlOfEntityWPtr, "pnl_account");

			RestrictedRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("account");

	private:
		std::string _currency = DEFAULT_CURRENCY;
		long long _balanceInCents = 0;

		friend class AccountsDatabase;
	};

	class AccountEntry : public RestrictedRecordDbo
	{
	private:
		AccountEntry(const Money &amount, Dbo::ptr<Account> debitAccountPtr, Dbo::ptr<Account> creditAccountPtr)
			: _amountInCents(amount.valueInCents()), _debitAccountPtr(move(debitAccountPtr)), _creditAccountPtr(move(creditAccountPtr))
		{ }
		AccountEntry(const AccountEntry &) = default;

		long long _amountInCents = 0;
		Dbo::ptr<Account> _debitAccountPtr;
		Dbo::ptr<Account> _creditAccountPtr;

		friend class AccountsDatabase;

	public:
		enum Type
		{
			UnspecifiedType = 0,
			OvertimeExpense = 1,
			FineIncome = 2,
			PettyExpenditure = 3
		};

		AccountEntry() = default;
		AccountEntry(AccountEntry &&) = default;
		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" NEW_ACCOUNTENTRY_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" ACCOUNTENTRY_PREFIX + idStr; }

		Type type = UnspecifiedType;
		Money amount() const { return Money(_amountInCents, DEFAULT_CURRENCY); }
		Dbo::ptr<Account> debitAccountPtr() const { return _debitAccountPtr; }
		Dbo::ptr<Account> creditAccountPtr() const { return _creditAccountPtr; }
		Wt::WString description;

		Dbo::ptr<ExpenseCycle> expenseCyclePtr;
		Dbo::ptr<IncomeCycle> incomeCyclePtr;

		Dbo::weak_ptr<OvertimeInfo> overtimeInfoWPtr;
		Dbo::weak_ptr<FineInfo> fineInfoWPtr;
		Dbo::weak_ptr<PettyExpenditureInfo> pettyExpenditureInfoWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, _debitAccountPtr, "debit_account", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::belongsTo(a, _creditAccountPtr, "credit_account", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, type, "type");
			Dbo::field(a, _amountInCents, "amount");
			Dbo::field(a, description, "description", 255);

			Dbo::belongsTo(a, expenseCyclePtr, "expensecycle", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, incomeCyclePtr, "incomecycle", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			Dbo::hasOne(a, overtimeInfoWPtr, "accountentry");
			Dbo::hasOne(a, fineInfoWPtr, "accountentry");
			Dbo::hasOne(a, pettyExpenditureInfoWPtr, "accountentry");

			RestrictedRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("accountentry");
	};

	class OvertimeInfo
	{
	private:
		Dbo::ptr<AccountEntry> entryPtr;

	public:
		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, entryPtr, "accountentry", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
		}
		DEFINE_DBO_TABLENAME("overtimeInfo");
	};

	class FineInfo
	{
	private:
		Dbo::ptr<AccountEntry> entryPtr;

	public:
		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, entryPtr, "accountentry", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
		}
		DEFINE_DBO_TABLENAME("fineinfo");
	};

	class PettyExpenditureInfo
	{
	private:
		Dbo::ptr<AccountEntry> entryPtr;

	public:
		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, entryPtr, "accountentry", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
		}
		DEFINE_DBO_TABLENAME("pettyexpenditureinfo");
	};

	enum CycleInterval
	{
		DailyInterval = 0,
		WeeklyInterval = 1,
		MonthlyInterval = 2,
		YearlyInterval = 3
	};
	Wt::WDateTime addCycleInterval(const Wt::WDateTime &dt, CycleInterval interval, int nIntervals);
	Wt::WString rsEveryNIntervals(const Money &amount, CycleInterval interval, uint64_t nIntervals);

	class EntryCycle : public RestrictedRecordDbo
	{
	public:
		Dbo::ptr<Entity> entityPtr;
		Wt::WDate startDate;
		Wt::WDate endDate;
		CycleInterval interval = YearlyInterval;
		int nIntervals = std::numeric_limits<int>::max();
		bool firstEntryAfterCycle = false;

		Money amount() const { return Money(_amountInCents, DEFAULT_CURRENCY); }
		//Money setAmount(const Money &amount) { _amountInCents = amount.valueInCents(); }
		
		AccountEntryCollection entryCollection;

		template<class Action>
		void persist(Action& a, const std::string &cycleName)
		{
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, startDate, "startDate");
			Dbo::field(a, endDate, "endDate");
			Dbo::field(a, _amountInCents, "amount");
			Dbo::field(a, interval, "interval");
			Dbo::field(a, nIntervals, "nIntervals");
			Dbo::field(a, firstEntryAfterCycle, "firstEntryAfterCycle");

			Dbo::hasMany(a, entryCollection, Dbo::ManyToOne, cycleName);
			RestrictedRecordDbo::persist(a);
		}

	protected:
		long long _amountInCents = 0;
		friend class AccountsDatabase;
		friend class EntryCycleFormModel;
	};

	class IncomeCycle : public EntryCycle
	{
	public:
		enum Purpose
		{
			UnspecifiedPurpose = 0,
			Services = 1
		};

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" INCOMECYCLES_PATHC "/" NEW_INCOMECYCLE_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" INCOMECYCLES_PATHC "/" INCOMECYCLE_PREFIX + idStr; }

		ClientAssignmentCollection clientAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			EntryCycle::persist(a, "incomecycle");
			Dbo::hasMany(a, clientAssignmentCollection, Dbo::ManyToOne, "incomecycle");
		}
		DEFINE_DBO_TABLENAME("incomecycle");

	private:
		friend class IncomeCycleFormModel;
	};

	class ExpenseCycle : public EntryCycle
	{
	public:
		enum Purpose
		{
			UnspecifiedPurpose = 0,
			Salary = 1
		};

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" EXPENSECYCLES_PATHC "/" NEW_EXPENSECYCLE_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" EXPENSECYCLES_PATHC "/" EXPENSECYCLE_PREFIX + idStr; }
		
		EmployeeAssignmentCollection employeeAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			EntryCycle::persist(a, "expensecycle");
			Dbo::hasMany(a, employeeAssignmentCollection, Dbo::ManyToOne, "expensecycle");
		}
		DEFINE_DBO_TABLENAME("expensecycle");

	private:
		friend class ExpenseCycleFormModel;
	};

	class UploadedFile : public Dbo::Dbo<UploadedFile>, public BaseRecordDbo
	{
	public:
		Wt::Dbo::ptr<Entity> entityPtr;
		std::string description;
		std::string extension;
		std::string mimeType;

		Wt::Dbo::weak_ptr<Person> profilePictureOfWPtr;
		Wt::Dbo::weak_ptr<Person> cnicPicture1OfWPtr;
		Wt::Dbo::weak_ptr<Person> cnicPicture2OfWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, description, "description");
			Wt::Dbo::field(a, extension, "extension", 255);
			Wt::Dbo::field(a, mimeType, "mimeType", 255);

			Wt::Dbo::hasOne(a, profilePictureOfWPtr, "profilePictureFile");
			Wt::Dbo::hasOne(a, cnicPicture1OfWPtr, "cnicFile1");
			Wt::Dbo::hasOne(a, cnicPicture2OfWPtr, "cnicFile2");

			BaseRecordDbo::persist(a);
		}
		std::string pathToFile() const;
		std::string pathToDirectory() const;
		DEFINE_DBO_TABLENAME("uploadedfile");
	};

	class AttendanceDevice : public RestrictedRecordDbo
	{
	public:
		AttendanceDeviceVCollection versionsCollection;
		AttendanceEntryCollection attendanceCollection;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEDEVICES_PATHC "/" NEW_ATTENDANCEDEVICE_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEDEVICES_PATHC "/" ATTENDANCEDEVICE_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::hasMany(a, versionsCollection, Dbo::ManyToOne, "parent");
			Dbo::hasMany(a, attendanceCollection, Dbo::ManyToOne, "attendancedevice");

			RestrictedRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("attendancedevice");
	};
	class AttendanceDeviceV : public RecordVersionDbo<AttendanceDevice>
	{
	public:
		AttendanceDeviceV() = default;
		AttendanceDeviceV(Dbo::ptr<AttendanceDevice> parentPtr) : RecordVersionDbo(move(parentPtr)) { }

		std::string hostName;
		Dbo::ptr<Location> locationPtr;

		template<class Action>
		void persist(Action& a)
		{
			RecordVersionDbo::persist(a);

			Dbo::field(a, hostName, "hostName", 255);
			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
		}
		DEFINE_DBO_TABLENAME("attendancedevice_v");
	};

	class AttendanceEntry : public BaseRecordDbo
	{
	public:
		Wt::WDateTime timestampIn;
		Wt::WDateTime timestampOut;
		Dbo::ptr<Entity> entityPtr;
		Dbo::ptr<AttendanceDevice> attendanceDevicePtr;
		Dbo::ptr<Location> locationPtr;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" NEW_ATTENDANCEENTRY_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(std::to_string(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEENTRY_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, timestampIn, "timestampIn");
			Dbo::field(a, timestampOut, "timestampOut");
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::belongsTo(a, attendanceDevicePtr, "attendancedevice", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			BaseRecordDbo::persist(a);
		}
		DEFINE_DBO_TABLENAME("attendanceentry");
	};

// 	class Inquiry
// 	{
// 	public:
// 		std::string notes;
// 		Wt::WDateTime startDt;
// 		Wt::WDateTime resolutionDt;
// 
// 		Dbo::ptr<Entity> entityPtr;
// 		Dbo::ptr<Location> locationPtr;
// 		Dbo::ptr<Asset> assetPtr;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::field(a, notes, "notes");
// 			Dbo::field(a, startDt, "startDt");
// 			Dbo::field(a, resolutionDt, "resolutionDt");
// 
// 			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 			Dbo::belongsTo(a, assetPtr, "asset", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 		}
// 		DEFINE_DBO_TABLENAME("inquiry");
// 	};

}

//Any Traits
namespace Wt
{
	using namespace ERP;

	template<>
	struct any_traits<Money>
	{
		static Wt::WString asString(const Money &value, const Wt::WString &) { return WLocale::currentLocale().toString(value); }
		static double asNumber(const Money &v) { return v.valueDbl(); }
		static int compare(const Money &v1, const Money &v2) { return v1.currency() == v2.currency() ? v1.valueInCents() == v2.valueInCents() ? 0 : (v1.valueInCents() < v2.valueInCents() ? -1 : 1) : (v1.currency() < v2.currency() ? -1 : 1); }
	};

	template<>
	struct any_traits<Entity::Type> : public any_traits<int>
	{
		static Wt::WString asString(const Entity::Type &value, const Wt::WString &)
		{
			switch(value)
			{
			case Entity::PersonType: return tr("Person");
			case Entity::BusinessType: return tr("Business");
			default: return tr("Unknown");
			}
		}
	};

	template<>
	struct any_traits<Account::Type> : public any_traits<int>
	{
		static Wt::WString asString(const Account::Type &value, const Wt::WString &)
		{
			switch(value)
			{
			case Account::EntityBalanceAccount: return tr("EntityBalanceAccount");
			case Account::EntityPnlAccount: return tr("EntityPnlAccount");
			case Account::Asset: return tr("Asset");
			case Account::Liability: return tr("Liability");
			default: return tr("Unknown");
			}
		}
	};

	template<>
	struct any_traits<Wt::WFlags<Entity::SpecificType>> : public any_traits<int>
	{
		static Wt::WString asString(const Wt::WFlags<Entity::SpecificType> &value, const Wt::WString &)
		{
			std::string result;
			if(value.test(Entity::EmployeeType))
				result += tr("Employee").toUTF8() + ", ";
			if(value.test(Entity::ClientType))
				result += tr("Client").toUTF8() + ", ";

			result = result.substr(0, result.size() - 2);
			return Wt::WString::fromUTF8(result);
		}
	};

	template<>
	struct any_traits<BloodType> : public any_traits<int>
	{
		static Wt::WString asString(const BloodType &value, const Wt::WString &)
		{
			switch(value)
			{
			case OPositive: return "O+";
			case ONegative: return "O-";
			case APositive: return "A+";
			case ANegative: return "A-";
			case BPositive: return "B+";
			case BNegative: return "B-";
			case ABPositive: return "AB+";
			case ABNegative: return "AB-";
			default: return tr("Unknown");
			}
		}
	};

	template<>
	struct any_traits<MaritalStatus> : public any_traits<int>
	{
		static Wt::WString asString(const MaritalStatus &value, const Wt::WString &)
		{
			switch(value)
			{
			case Married: return tr("Married");
			case Unmarried: return tr("Unmarried");
			default: return tr("Unknown");
			}
		}
	};

	template<>
	struct any_traits<CycleInterval> : public any_traits<int>
	{
		static Wt::WString asString(const CycleInterval &value, const Wt::WString &)
		{
			switch(value)
			{
			case DailyInterval: return tr("Daily");
			case WeeklyInterval: return tr("Weekly");
			case MonthlyInterval: return tr("Monthly");
			case YearlyInterval: return tr("Yearly");
			default: return "";
			}
		}
	};

	template<>
	struct any_traits<IncomeCycle::Purpose> : public any_traits<int>
	{
		static Wt::WString asString(const IncomeCycle::Purpose &value, const Wt::WString &)
		{
			switch(value)
			{
			case IncomeCycle::UnspecifiedPurpose: return tr("UnspecificPurpose");
			case IncomeCycle::Services: return tr("Services");
			default: return tr("Unknown");
			}
		}
	};

	template<>
	struct any_traits<ExpenseCycle::Purpose> : public any_traits<int>
	{
		static Wt::WString asString(const ExpenseCycle::Purpose &value, const Wt::WString &)
		{
			switch(value)
			{
			case ExpenseCycle::UnspecifiedPurpose: return tr("UnspecificPurpose");
			case ExpenseCycle::Salary: return tr("Salary");
			default: return tr("Unknown");
			}
		}
	};

	namespace Dbo
	{
		template<typename V>
		struct sql_value_traits<WFlags<V>, void> : public sql_value_traits<int>
		{
			static const bool specialized = true;
			static bool read(WFlags<V> &flags, SqlStatement *statement, int column, int size)
			{
				int v;
				if(!sql_value_traits<int>::read(v, statement, column, size))
					return false;

				flags = WFlags<V>(static_cast<V>(v));
				return true;
			}
		};
	}
}

//Dbo traits template definition
namespace Wt
{
	namespace Dbo
	{
		//User Permissions
		template<class Action>
		void field(Action &a, ERP::UserPermissionPK &key, const std::string &, int)
		{
			belongsTo(a, key.userPtr, "user", OnDeleteCascade | OnUpdateCascade | NotNull);
			belongsTo(a, key.permissionPtr, "permission", OnDeleteCascade | OnUpdateCascade | NotNull);
		}
	}
}


#endif
