#ifndef GS_DBOS_H
#define GS_DBOS_H

#include "Common.h"
#include <Wt/Dbo/Dbo.h>
#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/WDate.h>
#include <Wt/WAny.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/SqlTraits.h>

#include <boost/lexical_cast.hpp>

#define TRANSACTION(app) Dbo::Transaction t(app->dboSession())

//Forward declarations and typedefs
namespace GS
{
	//User related
	class User;
	class Region;
	class Permission;
	struct UserPermissionPK;
	class UserPermission;
	class DefaultPermission;

	typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
	typedef Wt::Auth::Dbo::AuthIdentity<AuthInfo> AuthIdentity;
	typedef Wt::Auth::Dbo::AuthToken<AuthInfo> AuthToken;

	typedef Dbo::collection<Dbo::ptr<AuthInfo>> AuthInfoCollection;
	typedef Dbo::collection<Dbo::ptr<User>> UserCollection;
	typedef Dbo::collection<Dbo::ptr<Region>> RegionCollection;
	typedef Dbo::collection<Dbo::ptr<UserPermission>> UserPermissionCollection;
	typedef Dbo::collection<Dbo::ptr<Permission>> PermissionCollection;
	typedef Dbo::collection<Dbo::ptr<DefaultPermission>> DefaultPermissionCollection;

	struct PermissionDdo;
	typedef shared_ptr<PermissionDdo> PermissionSPtr;
	typedef shared_ptr<const PermissionDdo> PermissionCPtr;
	typedef std::map<long long, PermissionCPtr> PermissionMap;

	typedef Wt::Auth::Dbo::UserDatabase<GS::AuthInfo> UserDatabase;

	//The rest
	class Entity;
	class Person;
	class Business;
	class Employee;
	class Personnel;

	class EmployeePosition;
	class ClientService;

	class ContactNumber;
	class Country;
	class City;
	class Location;

	class EmployeeAssignment;
	class ClientAssignment;
//	class Inquiry;

// 	class Asset;
// 	class Inventory;
// 	class ClothingTemplate;
// 	class ClothingItem;
// 	class AlarmTemplate;
// 	class AlarmItem;
// 	class WeaponTemplate;
// 	class Weapon;
// 	class VehicleTemplate;
// 	class Vehicle;
// 	class Office;
// 	class RentHouse;
// 
// 	class AssetRegistration;

	class Account;
	class AccountEntry;
	class OvertimeInfo;
	class FineInfo;
	class PettyExpenditureInfo;

	class IncomeCycle;
	class ExpenseCycle;

	class UploadedFile;
	class AttendanceDevice;
	class AttendanceEntry;

	typedef Dbo::collection<Dbo::ptr<Entity>> EntityCollection;
	typedef Dbo::collection<Dbo::ptr<Person>> PersonCollection;
	typedef Dbo::collection<Dbo::ptr<Business>> BusinessCollection;
	typedef Dbo::collection<Dbo::ptr<Employee>> EmployeeCollection;
	typedef Dbo::collection<Dbo::ptr<Personnel>> PersonnelCollection;
	typedef Dbo::collection<Dbo::ptr<EmployeePosition>> EmployeePositionCollection;
	typedef Dbo::collection<Dbo::ptr<ClientService>> ClientServiceCollection;
	typedef Dbo::collection<Dbo::ptr<ContactNumber>> ContactNumberCollection;
	typedef Dbo::collection<Dbo::ptr<Country>> CountryCollection;
	typedef Dbo::collection<Dbo::ptr<City>> CityCollection;
	typedef Dbo::collection<Dbo::ptr<Location>> LocationCollection;
	typedef Dbo::collection<Dbo::ptr<EmployeeAssignment>> EmployeeAssignmentCollection;
	typedef Dbo::collection<Dbo::ptr<ClientAssignment>> ClientAssignmentCollection;
// 	typedef Dbo::collection<Dbo::ptr<Inquiry>> InquiryCollection;
// 	typedef Dbo::collection<Dbo::ptr<Asset>> AssetCollection;
// 	typedef Dbo::collection<Dbo::ptr<Inventory>> InventoryCollection;
// 	typedef Dbo::collection<Dbo::ptr<ClothingTemplate>> ClothingTemplateCollection;
// 	typedef Dbo::collection<Dbo::ptr<ClothingItem>> ClothingItemCollection;
// 	typedef Dbo::collection<Dbo::ptr<AlarmTemplate>> AlarmTemplateCollection;
// 	typedef Dbo::collection<Dbo::ptr<AlarmItem>> AlarmItemCollection;
// 	typedef Dbo::collection<Dbo::ptr<WeaponTemplate>> WeaponTemplateCollection;
// 	typedef Dbo::collection<Dbo::ptr<Weapon>> WeaponCollection;
// 	typedef Dbo::collection<Dbo::ptr<VehicleTemplate>> VehicleTemplateCollection;
// 	typedef Dbo::collection<Dbo::ptr<Vehicle>> VehicleCollection;
// 	typedef Dbo::collection<Dbo::ptr<Office>> OfficeCollection;
// 	typedef Dbo::collection<Dbo::ptr<RentHouse>> RentHouseCollection;
// 	typedef Dbo::collection<Dbo::ptr<AssetRegistration>> LicenseCollection;
	typedef Dbo::collection<Dbo::ptr<IncomeCycle>> IncomeCycleCollection;
	typedef Dbo::collection<Dbo::ptr<ExpenseCycle>> ExpenseCycleCollection;
	typedef Dbo::collection<Dbo::ptr<Account>> AccountCollection;
	typedef Dbo::collection<Dbo::ptr<AccountEntry>> AccountEntryCollection;
	typedef Dbo::collection<Dbo::ptr<OvertimeInfo>> OvertimeInfoCollection;
	typedef Dbo::collection<Dbo::ptr<FineInfo>> FineInfoCollection;
	typedef Dbo::collection<Dbo::ptr<PettyExpenditureInfo>> PettyExpenditureInfoCollection;
	typedef Dbo::collection<Dbo::ptr<UploadedFile>> UploadedFileCollection;
	typedef Dbo::collection<Dbo::ptr<AttendanceDevice>> AttendanceDeviceCollection;
	typedef Dbo::collection<Dbo::ptr<AttendanceEntry>> AttendanceEntryCollection;
}

//Dbo traits
namespace Wt
{
	namespace Dbo
	{
		//User Permissions
		template <class Action>
		void field(Action &a, GS::UserPermissionPK &key, const std::string &, int size = -1);

		template<>
		struct dbo_traits<GS::UserPermission> : public dbo_default_traits
		{
			typedef GS::UserPermissionPK IdType;
			static IdType invalidId();
			static const char *surrogateIdField() { return nullptr; }
		};

		template<>
		struct dbo_traits<GS::Permission> : public dbo_default_traits
		{
			static const char *surrogateIdField() { return nullptr; }
		};

		//Entities
		template<>
		struct dbo_traits<GS::Person> : public dbo_default_traits
		{
			typedef ptr<GS::Entity> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Business> : public dbo_default_traits
		{
			typedef ptr<GS::Entity> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Employee> : public dbo_default_traits
		{
			typedef ptr<GS::Person> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Personnel> : public dbo_default_traits
		{
			typedef ptr<GS::Employee> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Country> : public dbo_default_traits
		{
			typedef std::string IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};

// 		//Assets
// 		template<>
// 		struct dbo_traits<GS::Inventory> : public dbo_default_traits
// 		{
// 			typedef ptr<GS::Asset> IdType;
// 			static IdType invalidId() { return IdType(); }
// 			static const char *surrogateIdField() { return nullptr; }
// 		};
// 		template<>
// 		struct dbo_traits<GS::Weapon> : public dbo_default_traits
// 		{
// 			typedef ptr<GS::Asset> IdType;
// 			static IdType invalidId() { return IdType(); }
// 			static const char *surrogateIdField() { return nullptr; }
// 		};
// 		template<>
// 		struct dbo_traits<GS::Vehicle> : public dbo_default_traits
// 		{
// 			typedef ptr<GS::Asset> IdType;
// 			static IdType invalidId() { return IdType(); }
// 			static const char *surrogateIdField() { return nullptr; }
// 		};
// 		template<>
// 		struct dbo_traits<GS::ClothingItem> : public dbo_default_traits
// 		{
// 			typedef ptr<GS::Asset> IdType;
// 			static IdType invalidId() { return IdType(); }
// 			static const char *surrogateIdField() { return nullptr; }
// 		};
// 		template<>
// 		struct dbo_traits<GS::AlarmItem> : public dbo_default_traits
// 		{
// 			typedef ptr<GS::Asset> IdType;
// 			static IdType invalidId() { return IdType(); }
// 			static const char *surrogateIdField() { return nullptr; }
// 		};

		//Accounts
		template<>
		struct dbo_traits<GS::OvertimeInfo> : public dbo_default_traits
		{
			typedef ptr<GS::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::FineInfo> : public dbo_default_traits
		{
			typedef ptr<GS::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::PettyExpenditureInfo> : public dbo_default_traits
		{
			typedef ptr<GS::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			static const char *surrogateIdField() { return nullptr; }
		};
	}
}

//Dbos
namespace GS
{
	class BaseAdminRecord
	{
	public:
		Dbo::ptr<User> creatorUserPtr;
		Dbo::ptr<Region> regionPtr;
		Wt::WDateTime timestamp = Wt::WDateTime::currentDateTime();

		void setCreatedByValues(bool setRegion = true);

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, creatorUserPtr, "creator_user", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, regionPtr, "region", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::field(a, timestamp, "timestamp");
		}
	};

	class User : public BaseAdminRecord
	{
	public:
		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" USERS_PATHC "/" NEW_USER_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" USERS_PATHC "/" USER_PREFIX + idStr; }

		Dbo::weak_ptr<AuthInfo> authInfoWPtr;

		UserPermissionCollection userPermissionCollection;

		EntityCollection entitiesCollection;
		AccountCollection accountsCollection;
		AccountEntryCollection accountEntriesCollection;
		IncomeCycleCollection incomeCyclesCollection;
		ExpenseCycleCollection expenseCyclesCollection;
		UserCollection createdUserCollection;

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

			BaseAdminRecord::persist(a);
		}
		static const char *tableName()
		{
			return "user";
		}
	};

	class Region
	{
	public:
		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" USERS_PATHC "/" REGIONS_PATHC "/" NEW_REGION_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
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
		}
		static const char *tableName()
		{
			return "region";
		}
	};

	class Permission
	{
	private:
		long long _id = -1;

	public:
		Permission() = default;
		Permission(long long id) : _id(id) { }
		Permission(long long id, const std::string &name) : _id(id), name(name) { }

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
		static const char *tableName()
		{
			return "permission";
		}
	};
	struct PermissionDdo
	{
		long long id = -1;
		std::string name;
		bool requireStrongLogin = false;

		PermissionDdo(Dbo::ptr<Permission> ptr)
			: id(ptr.id()), name(ptr->name), requireStrongLogin(ptr->requireStrongLogin)
		{ }
	};

	struct UserPermissionPK
	{
		Dbo::ptr<User> userPtr;
		Dbo::ptr<Permission> permissionPtr;

		UserPermissionPK() = default;
		UserPermissionPK(Dbo::ptr<User> userPtr, Dbo::ptr<Permission> permissionPtr)
			: userPtr(userPtr), permissionPtr(permissionPtr)
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
			_id.userPtr = userPtr;
			_id.permissionPtr = permissionPtr;
		}

		template<class Action>void persist(Action &a)
		{
			Dbo::id(a, _id, "id");
			Dbo::field(a, denied, "denied");
		}
		static const char *tableName()
		{
			return "user_permission";
		}

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
		static const char *tableName()
		{
			return "default_permission";
		}
	};
// 	struct DefaultPermissionDdo
// 	{
// 		long long permissionId;
// 		DefaultPermission::LoginStates loginStates = DefaultPermission::AllStates;
// 
// 		DefaultPermissionDdo(Dbo::ptr<DefaultPermission> ptr)
// 			: permissionId(ptr->permissionPtr.id()), loginStates(ptr->loginStates)
// 		{ }
// 	};

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

	class Entity : public BaseAdminRecord
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
			PersonnelType	= 0x02,
			ClientType		= 0x04
		};

		Entity() = default;
		Entity(Type type)
			: type(type)
		{ }

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" NEW_ENTITY_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
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
		Wt::WFlags<SpecificType> specificTypeMask = UnspecificType;
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
// 		RentHouseCollection owningRentHouseCollection;
// 		AlarmItemCollection assignedAlarmItemCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, name, "name", 70);
			Dbo::field(a, type, "type");
			Dbo::field(a, specificTypeMask, "specificTypeMask");
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
// 			Dbo::hasMany(a, owningRentHouseCollection, Dbo::ManyToOne, "owner_entity");
// 			Dbo::hasMany(a, assignedAlarmItemCollection, Dbo::ManyToOne, "entity");

			BaseAdminRecord::persist(a);
		}
		static const char *tableName()
		{
			return "entity";
		}
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

		Dbo::weak_ptr<Employee> employeeWPtr;
		PersonnelCollection witnesserCollection;

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

			Dbo::hasOne(a, employeeWPtr, "person");
			Dbo::hasMany(a, witnesserCollection, Dbo::ManyToMany, "personnel_witness");

			Dbo::belongsTo(a, profilePictureFilePtr, "profilePictureFile", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, cnicFile1Ptr, "cnicFile1", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, cnicFile2Ptr, "cnicFile2", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
		}
		static const char *tableName()
		{
			return "person";
		}

	private:
		friend class PersonFormModel;
	};
	class Employee
	{
	private:
		Dbo::ptr<Person> _personPtr;

	public:
		Dbo::ptr<Person> personPtr() const { return _personPtr; }
		std::string companyNumber;
		std::string grade;
		Wt::WDate recruitmentDate;
		std::string education;
		std::string experience;
		std::string addtionalQualifications;

		//ClothingItemCollection assignedClothesCollection;
		Dbo::weak_ptr<Personnel> personnelWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, _personPtr, "person", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, companyNumber, "companyNumber", 35);
			Dbo::field(a, grade, "grade", 35);
			Dbo::field(a, recruitmentDate, "recruitmentDate");
			Dbo::field(a, education, "education");
			Dbo::field(a, experience, "experience");
			Dbo::field(a, addtionalQualifications, "addtionalQualifications");

			//Dbo::hasMany(a, assignedClothesCollection, Dbo::ManyToOne, "employee");
			Dbo::hasOne(a, personnelWPtr, "employee");
		}
		static const char *tableName()
		{
			return "employee";
		}

	private:
		friend class EmployeeFormModel;
	};
	class Personnel
	{
	private:
		Dbo::ptr<Employee> _employeePtr;

	public:
		Dbo::ptr<Employee> employeePtr() const { return _employeePtr; }
		std::string policeStation;
		bool policeVerified = false;
		std::string trainingCourses;
		std::string armyNumber;
		std::string rank;

		PersonCollection witnessCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, _employeePtr, "employee", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, policeStation, "policeStation", 70);
			Dbo::field(a, policeVerified, "policeVerified");
			Dbo::field(a, trainingCourses, "trainingCourses");
			Dbo::field(a, armyNumber, "armyNumber", 35);
			Dbo::field(a, rank, "rank", 35);

			Dbo::hasMany(a, witnessCollection, Dbo::ManyToMany, "personnel_witness");
		}
		static const char *tableName()
		{
			return "personnel";
		}

	private:
		friend class PersonnelFormModel;
	};

	class EmployeePosition
	{
	public:
		enum Type
		{
			OtherType = 0,
			PersonnelType = 1
		};

		std::string title;
		Type type;
		EmployeeAssignmentCollection employeeAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, title, "title", 70);
			Dbo::field(a, type, "type");
			Dbo::hasMany(a, employeeAssignmentCollection, Dbo::ManyToOne, "employeeposition");
		}
		static const char *tableName()
		{
			return "employeeposition";
		}
	};

	class ClientService
	{
	public:
		std::string title;
		ClientAssignmentCollection clientAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, title, "title", 70);
			Dbo::hasMany(a, clientAssignmentCollection, Dbo::ManyToOne, "clientservice");
		}
		static const char *tableName()
		{
			return "clientservice";
		}
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
		static const char *tableName()
		{
			return "business";
		}

	private:
		friend class BusinessFormModel;
	};

	class ContactNumber
	{
	public:
		Dbo::ptr<Entity> entityPtr;
		Dbo::ptr<Location> locationPtr;
		std::string countryCode;
		std::string nationalNumber;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::field(a, countryCode, "countryCode", 3);
			Dbo::field(a, nationalNumber, "nationalNumber", 15);
		}
		static const char *tableName()
		{
			return "contactnumber";
		}
	};
	class Country
	{
	public:
		std::string code;
		std::string name;
		CityCollection cityCollection;
		
		Country() = default;
		Country(const std::string &code, const std::string &name = "") : code(code), name(name) { }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::id(a, code, "code", 3);
			Dbo::field(a, name, "name", 70);
			Dbo::hasMany(a, cityCollection, Dbo::ManyToOne, "country");
		}
		static const char *tableName()
		{
			return "country";
		}
	};
	class City
	{
	public:
		Dbo::ptr<Country> countryPtr;
		std::string name;

		City() = default;
		City(Dbo::ptr<Country> countryPtr, const std::string &name = "") : countryPtr(countryPtr), name(name) { }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, countryPtr, "country", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, name, "name", 70);
		}
		static const char *tableName()
		{
			return "city";
		}
	};
	class Location : public BaseAdminRecord
	{
	public:
		Dbo::ptr<Entity> entityPtr;
		std::string address;
		Dbo::ptr<Country> countryPtr;
		Dbo::ptr<City> cityPtr;

		ContactNumberCollection contactNumberCollection;
		EmployeeAssignmentCollection assignedEmployeeCollection;
		AttendanceDeviceCollection attendanceDeviceCollection;
		AttendanceEntryCollection attendanceCollection;
// 		InquiryCollection inquiryCollection;
// 		AssetCollection assetCollection;
// 		RentHouseCollection rentHouseCollection;

		template<class Action>
		void persist(Action& a)
		{
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::field(a, address, "address");
			Dbo::belongsTo(a, countryPtr, "country", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, cityPtr, "city", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			Dbo::hasMany(a, contactNumberCollection, Dbo::ManyToOne, "location");
			Dbo::hasMany(a, assignedEmployeeCollection, Dbo::ManyToOne, "location");
			Dbo::hasMany(a, attendanceDeviceCollection, Dbo::ManyToOne, "location");
			Dbo::hasMany(a, attendanceCollection, Dbo::ManyToOne, "location");
// 			Dbo::hasMany(a, inquiryCollection, Dbo::ManyToOne, "location");
// 			Dbo::hasMany(a, assetCollection, Dbo::ManyToOne, "location");
// 			Dbo::hasMany(a, rentHouseCollection, Dbo::ManyToOne, "location");

			BaseAdminRecord::persist(a);
		}
		static const char *tableName()
		{
			return "location";
		}
	};

	class EmployeeAssignment : public BaseAdminRecord
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

		Dbo::weak_ptr<EmployeeAssignment> transferredToWPtr;
		Dbo::ptr<EmployeeAssignment> transferredFromPtr;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" EMPLOYEES_PATHC "/" NEW_EMPLOYEEASSIGNMENT_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
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

			Dbo::hasOne(a, transferredToWPtr, "transferred_from");
			Dbo::belongsTo(a, transferredFromPtr, "transferred_from", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			BaseAdminRecord::persist(a);
		}
		static const char *tableName()
		{
			return "employeeassignment";
		}
	};

	class ClientAssignment : public BaseAdminRecord
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
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
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

			BaseAdminRecord::persist(a);
		}
		static const char *tableName()
		{
			return "clientassignment";
		}
	};

	class Account : public BaseAdminRecord
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
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
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

			BaseAdminRecord::persist(a);
		}
		static const char *tableName()
		{
			return "account";
		}

	private:
		std::string _currency = DEFAULT_CURRENCY;
		long long _balanceInCents = 0;

		friend class AccountsDatabase;
	};

	class AccountEntry : public BaseAdminRecord
	{
	private:
		AccountEntry(Money amount, Dbo::ptr<Account> debitAccountPtr, Dbo::ptr<Account> creditAccountPtr)
			: _amountInCents(amount.valueInCents()), _debitAccountPtr(debitAccountPtr), _creditAccountPtr(creditAccountPtr)
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
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" ACCOUNTENTRY_PREFIX + idStr; }

		Type type = UnspecifiedType;
		Money amount() const { return Money(_amountInCents, DEFAULT_CURRENCY); }
		Dbo::ptr<Account> debitAccountPtr() const { return _debitAccountPtr; }
		Dbo::ptr<Account> creditAccountPtr() const { return _creditAccountPtr; }
		Wt::WString description;

		Dbo::ptr<ExpenseCycle> expenseCyclePtr;
		Dbo::ptr<IncomeCycle> incomeCyclePtr;
		//Dbo::ptr<RentHouse> depositRentHousePtr;

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
			//Dbo::belongsTo(a, depositRentHousePtr, "deposit_renthouse", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			Dbo::hasOne(a, overtimeInfoWPtr, "accountentry");
			Dbo::hasOne(a, fineInfoWPtr, "accountentry");
			Dbo::hasOne(a, pettyExpenditureInfoWPtr, "accountentry");

			BaseAdminRecord::persist(a);
		}
		static const char *tableName()
		{
			return "accountentry";
		}
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
		static const char *tableName()
		{
			return "overtimeInfo";
		}
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
		static const char *tableName()
		{
			return "fineinfo";
		}
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
		static const char *tableName()
		{
			return "pettyexpenditureinfo";
		}
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

	class EntryCycle : public BaseAdminRecord
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
			BaseAdminRecord::persist(a);
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
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" INCOMECYCLES_PATHC "/" INCOMECYCLE_PREFIX + idStr; }

		ClientAssignmentCollection clientAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			EntryCycle::persist(a, "incomecycle");
			Dbo::hasMany(a, clientAssignmentCollection, Dbo::ManyToOne, "incomecycle");
		}
		static const char *tableName()
		{
			return "incomecycle";
		}

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
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" EXPENSECYCLES_PATHC "/" EXPENSECYCLE_PREFIX + idStr; }
		
		EmployeeAssignmentCollection employeeAssignmentCollection;
		//Dbo::weak_ptr<RentHouse> rentHouseWPtr;

		template<class Action>
		void persist(Action& a)
		{
			EntryCycle::persist(a, "expensecycle");
			Dbo::hasMany(a, employeeAssignmentCollection, Dbo::ManyToOne, "expensecycle");
			//Dbo::hasOne(a, rentHouseWPtr, "expensecycle");
		}
		static const char *tableName()
		{
			return "expensecycle";
		}

	private:
		friend class ExpenseCycleFormModel;
	};

	class UploadedFile : public Dbo::Dbo<UploadedFile>
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
		}
		std::string pathToFile() const;
		std::string pathToDirectory() const;
		static const char *tableName()
		{
			return "uploadedfile";
		}
	};

	class AttendanceDevice
	{
	public:
		std::string hostName;
		Dbo::ptr<Location> locationPtr;

		AttendanceEntryCollection attendanceCollection;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEDEVICES_PATHC "/" NEW_ATTENDANCEDEVICE_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEDEVICES_PATHC "/" ATTENDANCEDEVICE_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, hostName, "hostName", 255);
			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);

			Dbo::hasMany(a, attendanceCollection, Dbo::ManyToOne, "attendancedevice");
		}
		static const char *tableName()
		{
			return "attendancedevice";
		}
	};

	class AttendanceEntry
	{
	public:
		Wt::WDateTime timestampIn;
		Wt::WDateTime timestampOut;
		Dbo::ptr<Entity> entityPtr;
		Dbo::ptr<AttendanceDevice> attendanceDevicePtr;
		Dbo::ptr<Location> locationPtr;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" NEW_ATTENDANCEENTRY_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string &idStr) { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEENTRY_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Dbo::field(a, timestampIn, "timestampIn");
			Dbo::field(a, timestampOut, "timestampOut");
			Dbo::belongsTo(a, entityPtr, "entity", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
			Dbo::belongsTo(a, attendanceDevicePtr, "attendancedevice", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
		}
		static const char *tableName()
		{
			return "attendanceentry";
		}
	};

	void mapDboTree(Dbo::Session &dboSession);

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
// 		static const char *tableName()
// 		{
// 			return "inquiry";
// 		}
// 	};
// 
// 	class Asset
// 	{
// 	public:
// 		std::string name;
// 		Dbo::ptr<Location> locationPtr;
// 		Dbo::ptr<AssetRegistration> registrationPtr;
// 
// 		InquiryCollection inquiryCollection;
// 		Dbo::weak_ptr<Office> officeWPtr;
// 		Dbo::weak_ptr<RentHouse> rentHouseWPtr;
// 		Dbo::weak_ptr<Inventory> inventoryWPtr;
// 		Dbo::weak_ptr<ClothingItem> clothingItemWPtr;
// 		Dbo::weak_ptr<AlarmItem> alarmItemWPtr;
// 		Dbo::weak_ptr<Weapon> weaponWPtr;
// 		Dbo::weak_ptr<Vehicle> vehicleWPtr;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::field(a, name, "name", 255);
// 			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 			Dbo::belongsTo(a, registrationPtr, "assetregistration", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 
// 			Dbo::hasMany(a, inquiryCollection, Dbo::ManyToOne, "asset");
// 			Dbo::hasOne(a, officeWPtr, "asset");
// 			Dbo::hasOne(a, rentHouseWPtr, "asset");
// 			Dbo::hasOne(a, inventoryWPtr, "asset");
// 			Dbo::hasOne(a, clothingItemWPtr, "asset");
// 			Dbo::hasOne(a, alarmItemWPtr, "asset");
// 			Dbo::hasOne(a, weaponWPtr, "asset");
// 			Dbo::hasOne(a, vehicleWPtr, "asset");
// 		}
// 		static const char *tableName()
// 		{
// 			return "asset";
// 		}
// 	};
// 	class Inventory
// 	{
// 	private:
// 		Dbo::ptr<Asset> assetPtr;
// 
// 	public:
// 		int quantity = 0;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::id(a, assetPtr, "asset", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
// 			Dbo::field(a, quantity, "quantity");
// 		}
// 		static const char *tableName()
// 		{
// 			return "inventory";
// 		}
// 	};
// 	class ClothingTemplate
// 	{
// 	public:
// 		ClothingItemCollection clothingItemCollection;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::hasMany(a, clothingItemCollection, Dbo::ManyToOne, "clothingtemplate");
// 		}
// 		static const char *tableName()
// 		{
// 			return "clothingtemplate";
// 		}
// 	};
// 	class ClothingItem
// 	{
// 	private:
// 		Dbo::ptr<Asset> assetPtr;
// 
// 	public:
// 		Dbo::ptr<ClothingTemplate> clothingTemplatePtr;
// 		bool serviceable = false;
// 		Dbo::ptr<Employee> assignedEmployeePtr;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::id(a, assetPtr, "asset", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
// 			Dbo::belongsTo(a, clothingTemplatePtr, "clothingtemplate", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade);
// 			Dbo::field(a, serviceable, "serviceable");
// 			Dbo::belongsTo(a, assignedEmployeePtr, "employee", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 		}
// 		static const char *tableName()
// 		{
// 			return "clothingitem";
// 		}
// 	};
// 	class AlarmTemplate
// 	{
// 	public:
// 		AlarmItemCollection alarmItemCollection;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::hasMany(a, alarmItemCollection, Dbo::ManyToOne, "alarmtemplate");
// 		}
// 		static const char *tableName()
// 		{
// 			return "alarmtemplate";
// 		}
// 	};
// 	class AlarmItem
// 	{
// 	private:
// 		Dbo::ptr<Asset> assetPtr;
// 
// 	public:
// 		Dbo::ptr<AlarmTemplate> alarmTemplatePtr;
// 		bool serviceable = false;
// 		Dbo::ptr<Entity> assignedEntityPtr;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::id(a, assetPtr, "asset", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
// 			Dbo::belongsTo(a, alarmTemplatePtr, "alarmtemplate", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade);
// 			Dbo::field(a, serviceable, "serviceable");
// 			Dbo::belongsTo(a, assignedEntityPtr, "entity", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 		}
// 		static const char *tableName()
// 		{
// 			return "alarmitem";
// 		}
// 	};
// 	class WeaponTemplate
// 	{
// 	public:
// 		WeaponCollection weaponCollection;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::hasMany(a, weaponCollection, Dbo::ManyToOne, "weapontemplate");
// 		}
// 		static const char *tableName()
// 		{
// 			return "weapontemplate";
// 		}
// 	};
// 	class Weapon
// 	{
// 	private:
// 		Dbo::ptr<Asset> assetPtr;
// 
// 	public:
// 		Dbo::ptr<WeaponTemplate> weaponTemplatePtr;
// 		int weaponNumber;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::id(a, assetPtr, "asset", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
// 			Dbo::belongsTo(a, weaponTemplatePtr, "weapontemplate", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade);
// 			Dbo::field(a, weaponNumber, "weaponNumber");
// 		}
// 		static const char *tableName()
// 		{
// 			return "weapon";
// 		}
// 	};
// 	class VehicleTemplate
// 	{
// 	public:
// 		VehicleCollection vehicleCollection;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::hasMany(a, vehicleCollection, Dbo::ManyToOne, "vehicletemplate");
// 		}
// 		static const char *tableName()
// 		{
// 			return "vehicletemplate";
// 		}
// 	};
// 	class Vehicle
// 	{
// 	private:
// 		Dbo::ptr<Asset> assetPtr;
// 
// 	public:
// 		Dbo::ptr<VehicleTemplate> vehicleTemplatePtr;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::id(a, assetPtr, "asset", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade | Dbo::NotNull);
// 			Dbo::belongsTo(a, vehicleTemplatePtr, "vehicletemplate", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade);
// 		}
// 		static const char *tableName()
// 		{
// 			return "vehicle";
// 		}
// 	};
// 
// 	class AssetRegistration
// 	{
// 	public:
// 		Wt::WDate registrationDate;
// 		Wt::WDate expiryDate;
// 		std::string registrationNumber;
// 
// 		Dbo::ptr<AssetRegistration> renewedLicensePtr;
// 		Dbo::weak_ptr<AssetRegistration> previousLicenseWPtr;
// 
// 		Dbo::weak_ptr<Asset> assetWPtr;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::field(a, registrationDate, "registrationDate");
// 			Dbo::field(a, expiryDate, "expiryDate");
// 			Dbo::field(a, registrationNumber, "registrationNumber", 20);
// 
// 			Dbo::belongsTo(a, renewedLicensePtr, "renewed_license", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 			Dbo::hasOne(a, previousLicenseWPtr, "renewed_license");
// 
// 			Dbo::hasOne(a, assetWPtr, "assetregistration");
// 		}
// 		static const char *tableName()
// 		{
// 			return "assetregistration";
// 		}
// 	};
// 	class Office
// 	{
// 	public:
// 		Dbo::ptr<Asset> assetPtr;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::belongsTo(a, assetPtr, "asset", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 		}
// 		static const char *tableName()
// 		{
// 			return "office";
// 		}
// 	};
// 	class RentHouse
// 	{
// 	public:
// 		Dbo::ptr<Asset> assetPtr;
// 		std::string name;
// 		Dbo::ptr<Location> locationPtr;
// 		Dbo::ptr<ExpenseCycle> expenseCyclePtr;
// 		Dbo::ptr<Entity> ownerEntityPtr;
// 
// 		AccountEntryCollection depositEntryCollection;
// 
// 		template<class Action>
// 		void persist(Action& a)
// 		{
// 			Dbo::belongsTo(a, assetPtr, "asset", Dbo::OnDeleteCascade | Dbo::OnUpdateCascade);
// 			Dbo::field(a, name, "name", 255);
// 			Dbo::belongsTo(a, locationPtr, "location", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 			Dbo::belongsTo(a, expenseCyclePtr, "expensecycle", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 			Dbo::belongsTo(a, ownerEntityPtr, "owner_entity", Dbo::OnDeleteSetNull | Dbo::OnUpdateCascade);
// 
// 			Dbo::hasMany(a, depositEntryCollection, Dbo::ManyToOne, "deposit_renthouse");
// 		}
// 		static const char *tableName()
// 		{
// 			return "renthouse";
// 		}
// 	};


}

//Any Traits
namespace Wt
{
	using namespace GS;

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
			if(value & Entity::EmployeeType)
				result += tr("Employee").toUTF8() + ", ";
			if(value & Entity::PersonnelType)
				result += tr("Personnel").toUTF8() + ", ";
			if(value & Entity::ClientType)
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

	template<>
	struct any_traits<EmployeePosition::Type> : public any_traits<int>
	{
		static Wt::WString asString(const EmployeePosition::Type &value, const Wt::WString &)
		{
			switch(value)
			{
			case EmployeePosition::OtherType: return tr("Other");
			case EmployeePosition::PersonnelType: return tr("PersonnelPosition");
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
		void field(Action &a, GS::UserPermissionPK &key, const std::string &, int size)
		{
			belongsTo(a, key.userPtr, "user", OnDeleteCascade | OnUpdateCascade | NotNull);
			belongsTo(a, key.permissionPtr, "permission", OnDeleteCascade | OnUpdateCascade | NotNull);
		}
	}
}


#endif
