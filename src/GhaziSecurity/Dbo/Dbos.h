#ifndef GS_DBOS_H
#define GS_DBOS_H

#include "Utilities/Common.h"
#include <Wt/Dbo/Dbo>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WDate>
#include <Wt/WBoostAny>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/SqlTraits>
#include <Wt/Payment/Money>

#define TRANSACTION(app) Wt::Dbo::Transaction t(app->dboSession())

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
	typedef AuthInfo::AuthIdentityType AuthIdentity;
	typedef AuthInfo::AuthTokenType AuthToken;

	typedef Wt::Dbo::collection<Wt::Dbo::ptr<User>> UserCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Region>> RegionCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<UserPermission>> UserPermissionCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Permission>> PermissionCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<DefaultPermission>> DefaultPermissionCollection;

	struct PermissionDdo;
	typedef std::shared_ptr<PermissionDdo> PermissionSPtr;
	typedef std::shared_ptr<const PermissionDdo> PermissionCPtr;
	typedef std::map<long long, PermissionCPtr> PermissionMap;

	typedef Wt::Auth::Dbo::UserDatabase<GS::AuthInfo> UserDatabase;

	//The rest
	class Entity;
	class Person;
	class Business;

	class Client;
	class Supplier;
	class Employee;
	class Personnel;

	class EmployeePosition;
	class ClientService;

	class ContactNumber;
	class Country;
	class City;
	class Location;
	class Office;
	class RentHouse;

	class EmployeeAssignment;
	class ClientAssignment;
	class PersonnelAbsence;
	class PersonnelDischarge;
	class Inquiry;

	class Asset;
	class Inventory;
	class ClothingTemplate;
	class ClothingItem;
	class AlarmTemplate;
	class AlarmItem;
	class WeaponTemplate;
	class Weapon;
	class VehicleTemplate;
	class Vehicle;

	class AssetRegistration;

	class Account;
	class AccountEntry;
	class AccountEntryInfo;
	class OvertimeInfo;
	class FineInfo;
	class PettyExpenditureInfo;

	class IncomeCycle;
	class ExpenseCycle;

	class UploadedFile;
	class AttendanceDevice;
	class AttendanceEntry;

	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Entity>> EntityCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Person>> PersonCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Business>> BusinessCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Client>> ClientCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Supplier>> SupplierCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Personnel>> PersonnelCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<EmployeePosition>> EmployeePositionCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<ClientService>> ClientServiceCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Employee>> EmployeeCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<ContactNumber>> ContactNumberCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Country>> CountryCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<City>> CityCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Location>> LocationCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Office>> OfficeCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<RentHouse>> RentHouseCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<EmployeeAssignment>> EmployeeAssignmentCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<ClientAssignment>> ClientAssignmentCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<PersonnelAbsence>> PersonnelAbsenceCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<PersonnelDischarge>> PersonnelDischargeCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Inquiry>> InquiryCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Asset>> AssetCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Inventory>> InventoryCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<ClothingTemplate>> ClothingTemplateCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<ClothingItem>> ClothingItemCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AlarmTemplate>> AlarmTemplateCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AlarmItem>> AlarmItemCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<WeaponTemplate>> WeaponTemplateCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Weapon>> WeaponCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<VehicleTemplate>> VehicleTemplateCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Vehicle>> VehicleCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AssetRegistration>> LicenseCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<IncomeCycle>> IncomeCycleCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<ExpenseCycle>> ExpenseCycleCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Account>> AccountCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AccountEntry>> AccountEntryCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AccountEntryInfo>> AccountEntryInfoCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<OvertimeInfo>> OvertimeInfoCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<FineInfo>> FineInfoCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<PettyExpenditureInfo>> PettyExpenditureInfoCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<UploadedFile>> UploadedFileCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AttendanceDevice>> AttendanceDeviceCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AttendanceEntry>> AttendanceEntryCollection;
}

namespace Wt
{
	namespace Dbo
	{
		//User Permissions
		template <class Action>
		void field(Action &a, GS::UserPermissionPK &key, const std::string &, int size = -1)
		{
			Wt::Dbo::belongsTo(a, key.userPtr, "user", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, key.permissionPtr, "permission", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
		}
		template<>
		struct dbo_traits<GS::UserPermission> : public dbo_default_traits
		{
			typedef GS::UserPermissionPK IdType;
			static IdType invalidId();
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Permission> : public dbo_default_traits
		{
			constexpr static const char *surrogateIdField() { return nullptr; }
		};

		//Entities
		template<>
		struct dbo_traits<GS::Person> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Entity> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Business> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Entity> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Client> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Entity> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Supplier> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Entity> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Employee> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Person> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Personnel> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Employee> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Country> : public dbo_default_traits
		{
			typedef std::string IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};

		//Assets
		template<>
		struct dbo_traits<GS::Inventory> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Asset> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Weapon> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Asset> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::Vehicle> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Asset> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::ClothingItem> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Asset> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::AlarmItem> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::Asset> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};

		//Accounts
		template<>
		struct dbo_traits<GS::OvertimeInfo> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::FineInfo> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
		template<>
		struct dbo_traits<GS::PettyExpenditureInfo> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<GS::AccountEntry> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
	}
}

namespace GS
{
	typedef Wt::Payment::Money Money;

	class BaseAdminRecord
	{
	public:
		Wt::Dbo::ptr<User> creatorUserPtr;
		Wt::Dbo::ptr<Region> regionPtr;
		Wt::WDateTime timestamp = Wt::WDateTime(boost::posix_time::microsec_clock::local_time());

		void setCreatedByValues();

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, creatorUserPtr, "creator_user", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, regionPtr, "region", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::field(a, timestamp, "timestamp");
		}
	};

	class User
	{
	public:
		Wt::Dbo::ptr<Region> regionPtr;
		Wt::Dbo::weak_ptr<AuthInfo> authInfoWPtr;

		UserPermissionCollection userPermissionCollection;

		EntityCollection entitiesCollection;
		AccountCollection accountsCollection;
		AccountEntryCollection accountEntriesCollection;
		IncomeCycleCollection incomeCyclesCollection;
		ExpenseCycleCollection expenseCyclesCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, regionPtr, "region", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::hasOne(a, authInfoWPtr, "user");
			Wt::Dbo::hasMany(a, userPermissionCollection, Wt::Dbo::ManyToOne, "user");

			Wt::Dbo::hasMany(a, entitiesCollection, Wt::Dbo::ManyToOne, "creator_user");
			Wt::Dbo::hasMany(a, accountsCollection, Wt::Dbo::ManyToOne, "creator_user");
			Wt::Dbo::hasMany(a, accountEntriesCollection, Wt::Dbo::ManyToOne, "creator_user");
			Wt::Dbo::hasMany(a, incomeCyclesCollection, Wt::Dbo::ManyToOne, "creator_user");
			Wt::Dbo::hasMany(a, expenseCyclesCollection, Wt::Dbo::ManyToOne, "creator_user");
		}
		constexpr static const char *tableName()
		{
			return "user";
		}
	};

	class Region
	{
	public:
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
			Wt::Dbo::field(a, name, "name", 70);
			Wt::Dbo::hasMany(a, userCollection, Wt::Dbo::ManyToOne, "region");

			Wt::Dbo::hasMany(a, entitiesCollection, Wt::Dbo::ManyToOne, "region");
			Wt::Dbo::hasMany(a, accountsCollection, Wt::Dbo::ManyToOne, "region");
			Wt::Dbo::hasMany(a, accountEntriesCollection, Wt::Dbo::ManyToOne, "region");
			Wt::Dbo::hasMany(a, incomeCyclesCollection, Wt::Dbo::ManyToOne, "region");
			Wt::Dbo::hasMany(a, expenseCyclesCollection, Wt::Dbo::ManyToOne, "region");
		}
		constexpr static const char *tableName()
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
			Wt::Dbo::id(a, _id);
			Wt::Dbo::field(a, name, "name", 70);
			Wt::Dbo::field(a, requireStrongLogin, "requireStrongLogin");
			Wt::Dbo::hasMany(a, userPermissionCollection, Wt::Dbo::ManyToOne, "permission");
			Wt::Dbo::hasMany(a, defaultPermissionCollection, Wt::Dbo::ManyToOne, "permission");
			Wt::Dbo::hasMany(a, linkedToCollection, Wt::Dbo::ManyToMany, "linked_permission", "to");
			Wt::Dbo::hasMany(a, linkedByCollection, Wt::Dbo::ManyToMany, "linked_permission", "by");
		}
		constexpr static const char *tableName()
		{
			return "permission";
		}
	};
	struct PermissionDdo
	{
		long long id = -1;
		std::string name;
		bool requireStrongLogin = false;

		PermissionDdo(Wt::Dbo::ptr<Permission> ptr)
			: id(ptr.id()), name(ptr->name), requireStrongLogin(ptr->requireStrongLogin)
		{ }
	};

	struct UserPermissionPK
	{
		Wt::Dbo::ptr<User> userPtr;
		Wt::Dbo::ptr<Permission> permissionPtr;

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
		UserPermission(Wt::Dbo::ptr<User> userPtr, Wt::Dbo::ptr<Permission> permissionPtr)
		{
			_id.userPtr = userPtr;
			_id.permissionPtr = permissionPtr;
		}

		template<class Action>void persist(Action &a)
		{
			Wt::Dbo::id(a, _id, "id");
			Wt::Dbo::field(a, denied, "denied");
		}
		constexpr static const char *tableName()
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

		Wt::Dbo::ptr<Permission> permissionPtr;
		LoginStates loginStates = AllStates;

		template<class Action>void persist(Action &a)
		{
			Wt::Dbo::belongsTo(a, permissionPtr, "permission", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, loginStates, "loginStates");
		}
		constexpr static const char *tableName()
		{
			return "default_permission";
		}
	};
// 	struct DefaultPermissionDdo
// 	{
// 		long long permissionId;
// 		DefaultPermission::LoginStates loginStates = DefaultPermission::AllStates;
// 
// 		DefaultPermissionDdo(Wt::Dbo::ptr<DefaultPermission> ptr)
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
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" ENTITY_PREFIX + idStr; }
		static Wt::WString typeToArgString(Type type)
		{
			switch(type)
			{
			case PersonType: return Wt::WString::tr("person");
			case BusinessType: return Wt::WString::tr("business");
			default: return Wt::WString::tr("entity");
			}
		}

		std::string name;
		Type type = InvalidType;
		Wt::WFlags<SpecificType> specificTypeMask = UnspecificType;
		Wt::Dbo::ptr<Account> balAccountPtr;
		Wt::Dbo::ptr<Account> pnlAccountPtr;

		Wt::Dbo::weak_ptr<Person> personWPtr;
		Wt::Dbo::weak_ptr<Business> businessWPtr;
		Wt::Dbo::weak_ptr<Business> clientWPtr;
		Wt::Dbo::weak_ptr<Business> supplierWPtr;

		ContactNumberCollection contactNumberCollection;
		InquiryCollection inquiryCollection;
		RentHouseCollection owningRentHouseCollection;
		LocationCollection locationCollection;
		UploadedFileCollection uploadedFileCollection;
		IncomeCycleCollection incomeCycleCollection;
		ExpenseCycleCollection expenseCycleCollection;
		EmployeeAssignmentCollection employeeAssignmentCollection;
		ClientAssignmentCollection clientAssignmentCollection;
		AttendanceEntryCollection attendanceCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, name, "name", 70);
			Wt::Dbo::field(a, type, "type");
			Wt::Dbo::field(a, specificTypeMask, "specificTypeMask");
			Wt::Dbo::belongsTo(a, balAccountPtr, "bal_account", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, pnlAccountPtr, "pnl_account", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasOne(a, personWPtr, "entity");
			Wt::Dbo::hasOne(a, businessWPtr, "entity");
			Wt::Dbo::hasOne(a, clientWPtr, "entity");
			Wt::Dbo::hasOne(a, supplierWPtr, "entity");

			Wt::Dbo::hasMany(a, contactNumberCollection, Wt::Dbo::ManyToOne, "entity");
			Wt::Dbo::hasMany(a, inquiryCollection, Wt::Dbo::ManyToOne, "entity");
			Wt::Dbo::hasMany(a, owningRentHouseCollection, Wt::Dbo::ManyToOne, "owner_entity");
			Wt::Dbo::hasMany(a, locationCollection, Wt::Dbo::ManyToOne, "entity");
			Wt::Dbo::hasMany(a, uploadedFileCollection, Wt::Dbo::ManyToOne, "entity");
			Wt::Dbo::hasMany(a, incomeCycleCollection, Wt::Dbo::ManyToOne, "entity");
			Wt::Dbo::hasMany(a, expenseCycleCollection, Wt::Dbo::ManyToOne, "entity");
			Wt::Dbo::hasMany(a, employeeAssignmentCollection, Wt::Dbo::ManyToOne, "entity");
			Wt::Dbo::hasMany(a, clientAssignmentCollection, Wt::Dbo::ManyToOne, "entity");
			Wt::Dbo::hasMany(a, attendanceCollection, Wt::Dbo::ManyToOne, "entity");

			BaseAdminRecord::persist(a);
		}
		constexpr static const char *tableName()
		{
			return "entity";
		}
	};
	class Person
	{
	private:
		Wt::Dbo::ptr<Entity> _entityPtr;

	public:
		Wt::Dbo::ptr<Entity> entityPtr() const { return _entityPtr; }
		Wt::WDate dateOfBirth;
		std::string cnicNumber;
		std::string motherTongue;
		std::string identificationMark;
		float height = -1;
		BloodType bloodType = UnknownBT;
		MaritalStatus maritalStatus = UnknownMS;
		std::string remarks;

		Wt::Dbo::ptr<Person> fatherPersonPtr;
		Wt::Dbo::ptr<Person> motherPersonPtr;
		PersonCollection fatherChildrenCollection;
		PersonCollection motherChildrenCollection;

		PersonCollection nextOfKinCollection;
		Wt::Dbo::ptr<Person> nextOfKinOfPtr;

		Wt::Dbo::weak_ptr<Employee> employeeWPtr;
		PersonnelCollection witnesserCollection;

		Wt::Dbo::ptr<UploadedFile> profilePictureFilePtr;
		Wt::Dbo::ptr<UploadedFile> cnicFile1Ptr;
		Wt::Dbo::ptr<UploadedFile> cnicFile2Ptr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, _entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, dateOfBirth, "dateOfBirth");
			Wt::Dbo::field(a, cnicNumber, "cnicNumber", 13);
			Wt::Dbo::field(a, motherTongue, "motherTongue", 70);
			Wt::Dbo::field(a, identificationMark, "identificationMark", 255);
			Wt::Dbo::field(a, height, "height");
			Wt::Dbo::field(a, bloodType, "bloodType");
			Wt::Dbo::field(a, maritalStatus, "maritalStatus");
			Wt::Dbo::field(a, remarks, "remarks");

			Wt::Dbo::belongsTo(a, fatherPersonPtr, "father_person", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, motherPersonPtr, "mother_person", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::hasMany(a, fatherChildrenCollection, Wt::Dbo::ManyToOne, "father_person");
			Wt::Dbo::hasMany(a, motherChildrenCollection, Wt::Dbo::ManyToOne, "mother_person");

			Wt::Dbo::hasMany(a, nextOfKinCollection, Wt::Dbo::ManyToOne, "nextOfKin_person");
			Wt::Dbo::belongsTo(a, nextOfKinOfPtr, "nextOfKin_person", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasOne(a, employeeWPtr, "person");
			Wt::Dbo::hasMany(a, witnesserCollection, Wt::Dbo::ManyToMany, "personnel_witness");

			Wt::Dbo::belongsTo(a, profilePictureFilePtr, "profilePictureFile", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, cnicFile1Ptr, "cnicFile1", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, cnicFile2Ptr, "cnicFile2", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
		}
		constexpr static const char *tableName()
		{
			return "person";
		}

	private:
		friend class PersonFormModel;
	};
	class Employee
	{
	private:
		Wt::Dbo::ptr<Person> _personPtr;

	public:
		Wt::Dbo::ptr<Person> personPtr() const { return _personPtr; }
		std::string companyNumber;
		std::string grade;
		Wt::WDate recruitmentDate;
		std::string education;
		std::string experience;
		std::string addtionalQualifications;

		ClothingItemCollection assignedClothesCollection;
		Wt::Dbo::weak_ptr<Personnel> personnelWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, _personPtr, "person", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, companyNumber, "companyNumber", 35);
			Wt::Dbo::field(a, grade, "grade", 35);
			Wt::Dbo::field(a, recruitmentDate, "recruitmentDate");
			Wt::Dbo::field(a, education, "education");
			Wt::Dbo::field(a, experience, "experience");
			Wt::Dbo::field(a, addtionalQualifications, "addtionalQualifications");

			Wt::Dbo::hasMany(a, assignedClothesCollection, Wt::Dbo::ManyToOne, "employee");
			Wt::Dbo::hasOne(a, personnelWPtr, "employee");
		}
		constexpr static const char *tableName()
		{
			return "employee";
		}

	private:
		friend class EmployeeFormModel;
	};
	class Personnel
	{
	private:
		Wt::Dbo::ptr<Employee> _employeePtr;

	public:
		Wt::Dbo::ptr<Employee> employeePtr() const { return _employeePtr; }
		std::string policeStation;
		bool policeVerified = false;
		std::string trainingCourses;
		std::string armyNumber;
		std::string rank;

		Wt::Dbo::weak_ptr<PersonnelDischarge> dischargeWPtr;
		PersonnelAbsenceCollection absenceCollection;
		PersonCollection witnessCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, _employeePtr, "employee", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, policeStation, "policeStation", 70);
			Wt::Dbo::field(a, policeVerified, "policeVerified");
			Wt::Dbo::field(a, trainingCourses, "trainingCourses");
			Wt::Dbo::field(a, armyNumber, "armyNumber", 35);
			Wt::Dbo::field(a, rank, "rank", 35);

			Wt::Dbo::hasOne(a, dischargeWPtr, "personnel");
			Wt::Dbo::hasMany(a, absenceCollection, Wt::Dbo::ManyToOne, "personnel");
			Wt::Dbo::hasMany(a, witnessCollection, Wt::Dbo::ManyToMany, "personnel_witness");
		}
		constexpr static const char *tableName()
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
			Wt::Dbo::field(a, title, "title", 70);
			Wt::Dbo::field(a, type, "type");
			Wt::Dbo::hasMany(a, employeeAssignmentCollection, Wt::Dbo::ManyToOne, "employeeposition");
		}
		constexpr static const char *tableName()
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
			Wt::Dbo::field(a, title, "title", 70);
			Wt::Dbo::hasMany(a, clientAssignmentCollection, Wt::Dbo::ManyToOne, "clientservice");
		}
		constexpr static const char *tableName()
		{
			return "clientservice";
		}
	};

	class Business
	{
	private:
		Wt::Dbo::ptr<Entity> _entityPtr;

	public:
		Wt::Dbo::ptr<Entity> entityPtr() const { return _entityPtr; }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, _entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
		}
		constexpr static const char *tableName()
		{
			return "business";
		}

	private:
		friend class BusinessFormModel;
	};
	class Client
	{
	private:
		Wt::Dbo::ptr<Entity> _entityPtr;

	public:
		Wt::Dbo::ptr<Entity> entityPtr() const { return _entityPtr; }
		AlarmItemCollection assignedAlarmItemCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, _entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::hasMany(a, assignedAlarmItemCollection, Wt::Dbo::ManyToOne, "client");
		}
		constexpr static const char *tableName()
		{
			return "client";
		}

	private:
		friend class BusinessFormModel;
	};
	class Supplier
	{
	private:
		Wt::Dbo::ptr<Entity> _entityPtr;

	public:
		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, _entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
		}
		constexpr static const char *tableName()
		{
			return "supplier";
		}
	};

	class ContactNumber
	{
	public:
		Wt::Dbo::ptr<Entity> entityPtr;
		Wt::Dbo::ptr<Location> locationPtr;
		std::string countryCode;
		std::string nationalNumber;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, locationPtr, "location", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::field(a, countryCode, "countryCode", 3);
			Wt::Dbo::field(a, nationalNumber, "nationalNumber", 15);
		}
		constexpr static const char *tableName()
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
		Country(const std::string &code) : code(code) { }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, code, "code", 3);
			Wt::Dbo::field(a, name, "name", 70);
			Wt::Dbo::hasMany(a, cityCollection, Wt::Dbo::ManyToOne, "country");
		}
		constexpr static const char *tableName()
		{
			return "country";
		}
	};
	class City
	{
	public:
		Wt::Dbo::ptr<Country> countryPtr;
		std::string name;

		City() = default;
		City(Wt::Dbo::ptr<Country> countryPtr) : countryPtr(countryPtr) { }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, countryPtr, "country", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, name, "name", 70);
		}
		constexpr static const char *tableName()
		{
			return "city";
		}
	};
	class Location : public BaseAdminRecord
	{
	public:
		Wt::Dbo::ptr<Entity> entityPtr;
		std::string address;
		Wt::Dbo::ptr<Country> countryPtr;
		Wt::Dbo::ptr<City> cityPtr;

		ContactNumberCollection contactNumberCollection;
		EmployeeAssignmentCollection assignedEmployeeCollection;
		InquiryCollection inquiryCollection;
		AssetCollection assetCollection;
		RentHouseCollection rentHouseCollection;
		AttendanceDeviceCollection attendanceDeviceCollection;
		AttendanceEntryCollection attendanceCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, address, "address");
			Wt::Dbo::belongsTo(a, countryPtr, "country", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, cityPtr, "city", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasMany(a, contactNumberCollection, Wt::Dbo::ManyToOne, "location");
			Wt::Dbo::hasMany(a, assignedEmployeeCollection, Wt::Dbo::ManyToOne, "location");
			Wt::Dbo::hasMany(a, inquiryCollection, Wt::Dbo::ManyToOne, "location");
			Wt::Dbo::hasMany(a, assetCollection, Wt::Dbo::ManyToOne, "location");
			Wt::Dbo::hasMany(a, rentHouseCollection, Wt::Dbo::ManyToOne, "location");
			Wt::Dbo::hasMany(a, attendanceDeviceCollection, Wt::Dbo::ManyToOne, "location");
			Wt::Dbo::hasMany(a, attendanceCollection, Wt::Dbo::ManyToOne, "location");

			BaseAdminRecord::persist(a);
		}
		constexpr static const char *tableName()
		{
			return "location";
		}
	};
	class Office
	{
	public:
		Wt::Dbo::ptr<Asset> assetPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, assetPtr, "asset", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
		}
		constexpr static const char *tableName()
		{
			return "office";
		}
	};
	class RentHouse
	{
	public:
		Wt::Dbo::ptr<Asset> assetPtr;
		std::string name;
		Wt::Dbo::ptr<Location> locationPtr;
		Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr;
		Wt::Dbo::ptr<Entity> ownerEntityPtr;

		AccountEntryCollection depositEntryCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, assetPtr, "asset", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::field(a, name, "name", 255);
			Wt::Dbo::belongsTo(a, locationPtr, "location", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, expenseCyclePtr, "expensecycle", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, ownerEntityPtr, "owner_entity", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasMany(a, depositEntryCollection, Wt::Dbo::ManyToOne, "deposit_renthouse");
		}
		constexpr static const char *tableName()
		{
			return "renthouse";
		}
	};

	class EmployeeAssignment : public BaseAdminRecord
	{
	public:
		Wt::WDate startDate;
		Wt::WDate endDate;
		std::string description;
		Wt::Dbo::ptr<EmployeePosition> positionPtr;
		Wt::Dbo::ptr<Entity> entityPtr;
		Wt::Dbo::ptr<Location> locationPtr;
		Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr;
		Wt::Dbo::ptr<ClientAssignment> clientAssignmentPtr;

		Wt::Dbo::weak_ptr<EmployeeAssignment> transferredToWPtr;
		Wt::Dbo::ptr<EmployeeAssignment> transferredFromPtr;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" EMPLOYEES_PATHC "/" NEW_EMPLOYEEASSIGNMENT_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" EMPLOYEES_PATHC "/" EMPLOYEEASSIGNMENTS_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, startDate, "startDate");
			Wt::Dbo::field(a, endDate, "endDate");
			Wt::Dbo::field(a, description, "description");
			Wt::Dbo::belongsTo(a, positionPtr, "employeeposition", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, locationPtr, "location", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, expenseCyclePtr, "expensecycle", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, clientAssignmentPtr, "clientassignment", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasOne(a, transferredToWPtr, "transferred_from");
			Wt::Dbo::belongsTo(a, transferredFromPtr, "transferred_from", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			BaseAdminRecord::persist(a);
		}
		constexpr static const char *tableName()
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
		Wt::Dbo::ptr<ClientService> servicePtr;
		Wt::Dbo::ptr<Entity> entityPtr;
		Wt::Dbo::ptr<IncomeCycle> incomeCyclePtr;

		EmployeeAssignmentCollection employeeAssignmentCollection;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" EMPLOYEES_PATHC "/" NEW_CLIENTASSIGNMENT_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ENTITIES_PATHC "/" CLIENTS_PATHC "/" CLIENTASSIGNMENTS_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, startDate, "startDate");
			Wt::Dbo::field(a, endDate, "endDate");
			Wt::Dbo::field(a, description, "description");
			Wt::Dbo::belongsTo(a, servicePtr, "clientservice", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, incomeCyclePtr, "incomecycle", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasMany(a, employeeAssignmentCollection, Wt::Dbo::ManyToOne, "clientassignment");

			BaseAdminRecord::persist(a);
		}
		constexpr static const char *tableName()
		{
			return "clientassignment";
		}
	};

	class PersonnelAbsence
	{
	public:
		Wt::Dbo::ptr<Personnel> personnelPtr;
		Wt::WDate date;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, personnelPtr, "personnel", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, date, "date");
		}
		constexpr static const char *tableName()
		{
			return "personnelabsence";
		}
	};
	class PersonnelDischarge
	{
	public:
		Wt::Dbo::ptr<Personnel> personnelPtr;
		Wt::WDate date;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, personnelPtr, "personnel", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, date, "date");
		}
		constexpr static const char *tableName()
		{
			return "personneldischarge";
		}
	};
	class Inquiry
	{
	public:
		std::string notes;
		Wt::WDateTime startDt;
		Wt::WDateTime resolutionDt;

		Wt::Dbo::ptr<Entity> entityPtr;
		Wt::Dbo::ptr<Location> locationPtr;
		Wt::Dbo::ptr<Asset> assetPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, notes, "notes");
			Wt::Dbo::field(a, startDt, "startDt");
			Wt::Dbo::field(a, resolutionDt, "resolutionDt");

			Wt::Dbo::belongsTo(a, entityPtr, "entity", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, locationPtr, "location", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, assetPtr, "asset", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
		}
		constexpr static const char *tableName()
		{
			return "inquiry";
		}
	};

	class Asset
	{
	public:
		std::string name;
		Wt::Dbo::ptr<Location> locationPtr;
		Wt::Dbo::ptr<AssetRegistration> registrationPtr;

		InquiryCollection inquiryCollection;
		Wt::Dbo::weak_ptr<Office> officeWPtr;
		Wt::Dbo::weak_ptr<RentHouse> rentHouseWPtr;
		Wt::Dbo::weak_ptr<Inventory> inventoryWPtr;
		Wt::Dbo::weak_ptr<ClothingItem> clothingItemWPtr;
		Wt::Dbo::weak_ptr<AlarmItem> alarmItemWPtr;
		Wt::Dbo::weak_ptr<Weapon> weaponWPtr;
		Wt::Dbo::weak_ptr<Vehicle> vehicleWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, name, "name", 255);
			Wt::Dbo::belongsTo(a, locationPtr, "location", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, registrationPtr, "assetregistration", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasMany(a, inquiryCollection, Wt::Dbo::ManyToOne, "asset");
			Wt::Dbo::hasOne(a, officeWPtr, "asset");
			Wt::Dbo::hasOne(a, rentHouseWPtr, "asset");
			Wt::Dbo::hasOne(a, inventoryWPtr, "asset");
			Wt::Dbo::hasOne(a, clothingItemWPtr, "asset");
			Wt::Dbo::hasOne(a, alarmItemWPtr, "asset");
			Wt::Dbo::hasOne(a, weaponWPtr, "asset");
			Wt::Dbo::hasOne(a, vehicleWPtr, "asset");
		}
		constexpr static const char *tableName()
		{
			return "asset";
		}
	};
	class Inventory
	{
	private:
		Wt::Dbo::ptr<Asset> assetPtr;

	public:
		int quantity = 0;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, assetPtr, "asset", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, quantity, "quantity");
		}
		constexpr static const char *tableName()
		{
			return "inventory";
		}
	};
	class ClothingTemplate
	{
	public:
		ClothingItemCollection clothingItemCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::hasMany(a, clothingItemCollection, Wt::Dbo::ManyToOne, "clothingtemplate");
		}
		constexpr static const char *tableName()
		{
			return "clothingtemplate";
		}
	};
	class ClothingItem
	{
	private:
		Wt::Dbo::ptr<Asset> assetPtr;

	public:
		Wt::Dbo::ptr<ClothingTemplate> clothingTemplatePtr;
		bool serviceable = false;
		Wt::Dbo::ptr<Employee> assignedEmployeePtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, assetPtr, "asset", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, clothingTemplatePtr, "clothingtemplate", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::field(a, serviceable, "serviceable");
			Wt::Dbo::belongsTo(a, assignedEmployeePtr, "employee", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
		}
		constexpr static const char *tableName()
		{
			return "clothingitem";
		}
	};
	class AlarmTemplate
	{
	public:
		AlarmItemCollection alarmItemCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::hasMany(a, alarmItemCollection, Wt::Dbo::ManyToOne, "alarmtemplate");
		}
		constexpr static const char *tableName()
		{
			return "alarmtemplate";
		}
	};
	class AlarmItem
	{
	private:
		Wt::Dbo::ptr<Asset> assetPtr;

	public:
		Wt::Dbo::ptr<AlarmTemplate> alarmTemplatePtr;
		bool serviceable = false;
		Wt::Dbo::ptr<Client> assignedClientPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, assetPtr, "asset", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, alarmTemplatePtr, "alarmtemplate", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::field(a, serviceable, "serviceable");
			Wt::Dbo::belongsTo(a, assignedClientPtr, "client", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
		}
		constexpr static const char *tableName()
		{
			return "alarmitem";
		}
	};
	class WeaponTemplate
	{
	public:
		WeaponCollection weaponCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::hasMany(a, weaponCollection, Wt::Dbo::ManyToOne, "weapontemplate");
		}
		constexpr static const char *tableName()
		{
			return "weapontemplate";
		}
	};
	class Weapon
	{
	private:
		Wt::Dbo::ptr<Asset> assetPtr;

	public:
		Wt::Dbo::ptr<WeaponTemplate> weaponTemplatePtr;
		int weaponNumber;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, assetPtr, "asset", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, weaponTemplatePtr, "weapontemplate", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::field(a, weaponNumber, "weaponNumber");
		}
		constexpr static const char *tableName()
		{
			return "weapon";
		}
	};
	class VehicleTemplate
	{
	public:
		VehicleCollection vehicleCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::hasMany(a, vehicleCollection, Wt::Dbo::ManyToOne, "vehicletemplate");
		}
		constexpr static const char *tableName()
		{
			return "vehicletemplate";
		}
	};
	class Vehicle
	{
	private:
		Wt::Dbo::ptr<Asset> assetPtr;

	public:
		Wt::Dbo::ptr<VehicleTemplate> vehicleTemplatePtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, assetPtr, "asset", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, vehicleTemplatePtr, "vehicletemplate", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade);
		}
		constexpr static const char *tableName()
		{
			return "vehicle";
		}
	};

	class AssetRegistration
	{
	public:
		Wt::WDate registrationDate;
		Wt::WDate expiryDate;
		std::string registrationNumber;

		Wt::Dbo::ptr<AssetRegistration> renewedLicensePtr;
		Wt::Dbo::weak_ptr<AssetRegistration> previousLicenseWPtr;

		Wt::Dbo::weak_ptr<Asset> assetWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, registrationDate, "registrationDate");
			Wt::Dbo::field(a, expiryDate, "expiryDate");
			Wt::Dbo::field(a, registrationNumber, "registrationNumber", 20);

			Wt::Dbo::belongsTo(a, renewedLicensePtr, "renewed_license", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::hasOne(a, previousLicenseWPtr, "renewed_license");

			Wt::Dbo::hasOne(a, assetWPtr, "assetregistration");
		}
		constexpr static const char *tableName()
		{
			return "assetregistration";
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
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" ACCOUNT_PREFIX + idStr; }

		std::string name;
		Type type = Asset;
		Money balance() const { return Money(_balanceInCents, DEFAULT_CURRENCY); }
		long long balanceInCents() const { return _balanceInCents; }

		AccountEntryCollection debitEntryCollection;
		AccountEntryCollection creditEntryCollection;
		Wt::Dbo::weak_ptr<Entity> balOfEntityWPtr;
		Wt::Dbo::weak_ptr<Entity> pnlOfEntityWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, name, "name", 70);
			Wt::Dbo::field(a, type, "type");
			Wt::Dbo::field(a, _currency, "currency", 3);
			Wt::Dbo::field(a, _balanceInCents, "balance");

			Wt::Dbo::hasMany(a, debitEntryCollection, Wt::Dbo::ManyToOne, "debit_account");
			Wt::Dbo::hasMany(a, creditEntryCollection, Wt::Dbo::ManyToOne, "credit_account");
			Wt::Dbo::hasOne(a, balOfEntityWPtr, "bal_account");
			Wt::Dbo::hasOne(a, pnlOfEntityWPtr, "pnl_account");

			BaseAdminRecord::persist(a);
		}
		constexpr static const char *tableName()
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
		AccountEntry(Money amount, Wt::Dbo::ptr<Account> debitAccountPtr, Wt::Dbo::ptr<Account> creditAccountPtr)
			: _amountInCents(amount.valueInCents()), _debitAccountPtr(debitAccountPtr), _creditAccountPtr(creditAccountPtr)
		{ }
		AccountEntry(const AccountEntry &) = default;
		AccountEntry(AccountEntry &&) = default;

		long long _amountInCents = 0;
		Wt::Dbo::ptr<Account> _debitAccountPtr;
		Wt::Dbo::ptr<Account> _creditAccountPtr;

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
		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" NEW_ACCOUNTENTRY_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" ACCOUNTENTRY_PREFIX + idStr; }

		Type type = UnspecifiedType;
		Money amount() const { return Money(_amountInCents, DEFAULT_CURRENCY); }
		Wt::Dbo::ptr<Account> debitAccountPtr() const { return _debitAccountPtr; }
		Wt::Dbo::ptr<Account> creditAccountPtr() const { return _creditAccountPtr; }
		Wt::WString description;

		Wt::Dbo::ptr<RentHouse> depositRentHousePtr;
		Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr;
		Wt::Dbo::ptr<IncomeCycle> incomeCyclePtr;

		Wt::Dbo::weak_ptr<OvertimeInfo> overtimeInfoWPtr;
		Wt::Dbo::weak_ptr<FineInfo> fineInfoWPtr;
		Wt::Dbo::weak_ptr<PettyExpenditureInfo> pettyExpenditureInfoWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::belongsTo(a, _debitAccountPtr, "debit_account", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, _creditAccountPtr, "credit_account", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, type, "type");
			Wt::Dbo::field(a, _amountInCents, "amount");
			Wt::Dbo::field(a, description, "description", 255);

			Wt::Dbo::belongsTo(a, depositRentHousePtr, "deposit_renthouse", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, expenseCyclePtr, "expensecycle", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, incomeCyclePtr, "incomecycle", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasOne(a, overtimeInfoWPtr, "accountentry");
			Wt::Dbo::hasOne(a, fineInfoWPtr, "accountentry");
			Wt::Dbo::hasOne(a, pettyExpenditureInfoWPtr, "accountentry");

			BaseAdminRecord::persist(a);
		}
		constexpr static const char *tableName()
		{
			return "accountentry";
		}
	};
	class OvertimeInfo
	{
	private:
		Wt::Dbo::ptr<AccountEntry> entryPtr;

	public:
		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, entryPtr, "accountentry", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
		}
		constexpr static const char *tableName()
		{
			return "overtimeInfo";
		}
	};
	class FineInfo
	{
	private:
		Wt::Dbo::ptr<AccountEntry> entryPtr;

	public:
		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, entryPtr, "accountentry", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
		}
		constexpr static const char *tableName()
		{
			return "fineinfo";
		}
	};
	class PettyExpenditureInfo
	{
	private:
		Wt::Dbo::ptr<AccountEntry> entryPtr;

	public:
		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, entryPtr, "accountentry", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
		}
		constexpr static const char *tableName()
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
	boost::posix_time::ptime addCycleInterval(boost::posix_time::ptime pTime, CycleInterval interval, int nIntervals);

	class EntryCycle : public BaseAdminRecord
	{
	public:
		Wt::Dbo::ptr<Entity> entityPtr;
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
			Wt::Dbo::belongsTo(a, entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::field(a, startDate, "startDate");
			Wt::Dbo::field(a, endDate, "endDate");
			Wt::Dbo::field(a, _amountInCents, "amount");
			Wt::Dbo::field(a, interval, "interval");
			Wt::Dbo::field(a, nIntervals, "nIntervals");
			Wt::Dbo::field(a, firstEntryAfterCycle, "firstEntryAfterCycle");

			Wt::Dbo::hasMany(a, entryCollection, Wt::Dbo::ManyToOne, cycleName);
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
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" INCOMECYCLES_PATHC "/" INCOMECYCLE_PREFIX + idStr; }

// 		Purpose purpose = UnspecifiedPurpose;

		ClientAssignmentCollection clientAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			EntryCycle::persist(a, "incomecycle");
			//Wt::Dbo::field(a, purpose, "purpose");

			Wt::Dbo::hasMany(a, clientAssignmentCollection, Wt::Dbo::ManyToOne, "incomecycle");
		}
		constexpr static const char *tableName()
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
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ACCOUNTS_PATHC "/" EXPENSECYCLES_PATHC "/" EXPENSECYCLE_PREFIX + idStr; }
	
// 		Purpose purpose = UnspecifiedPurpose;
		
		Wt::Dbo::weak_ptr<RentHouse> rentHouseWPtr;
		EmployeeAssignmentCollection employeeAssignmentCollection;

		template<class Action>
		void persist(Action& a)
		{
			EntryCycle::persist(a, "expensecycle");
			//Wt::Dbo::field(a, purpose, "purpose");

			Wt::Dbo::hasOne(a, rentHouseWPtr, "expensecycle");
			Wt::Dbo::hasMany(a, employeeAssignmentCollection, Wt::Dbo::ManyToOne, "expensecycle");
		}
		constexpr static const char *tableName()
		{
			return "expensecycle";
		}

	private:
		friend class ExpenseCycleFormModel;
	};

	class UploadedFile : public Wt::Dbo::Dbo<UploadedFile>
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
		constexpr static const char *tableName()
		{
			return "uploadedfile";
		}
	};

	class AttendanceDevice
	{
	public:
		std::string hostName;
		Wt::Dbo::ptr<Location> locationPtr;

		AttendanceEntryCollection attendanceCollection;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEDEVICES_PATHC "/" NEW_ATTENDANCEDEVICE_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEDEVICES_PATHC "/" ATTENDANCEDEVICE_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, hostName, "hostName", 255);
			Wt::Dbo::belongsTo(a, locationPtr, "location", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasMany(a, attendanceCollection, Wt::Dbo::ManyToOne, "attendancedevice");
		}
		constexpr static const char *tableName()
		{
			return "attendancedevice";
		}
	};

	class AttendanceEntry
	{
	public:
		Wt::WDateTime timestampIn;
		Wt::WDateTime timestampOut;
		Wt::Dbo::ptr<Entity> entityPtr;
		Wt::Dbo::ptr<AttendanceDevice> attendanceDevicePtr;
		Wt::Dbo::ptr<Location> locationPtr;

		static std::string newInternalPath() { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" NEW_ATTENDANCEENTRY_PATHC; }
		static std::string viewInternalPath(long long id) { return viewInternalPath(boost::lexical_cast<std::string>(id)); }
		static std::string viewInternalPath(const std::string idStr) { return "/" ADMIN_PATHC "/" ATTENDANCE_PATHC "/" ATTENDANCEENTRY_PREFIX + idStr; }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, timestampIn, "timestampIn");
			Wt::Dbo::field(a, timestampOut, "timestampOut");
			Wt::Dbo::belongsTo(a, entityPtr, "entity", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, attendanceDevicePtr, "attendancedevice", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::belongsTo(a, locationPtr, "location", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
		}
		constexpr static const char *tableName()
		{
			return "attendanceentry";
		}
	};

	void mapDboTree(Wt::Dbo::Session &dboSession);

}

namespace Wt
{
	using namespace GS;

	template<>
	struct boost_any_traits<Money>
	{
		static Wt::WString asString(const Money &value, const Wt::WString &) { return WLocale::currentLocale().toString(value); }
		static double asNumber(const Money &v) { return v.valueDbl(); }
		static int compare(const Money &v1, const Money &v2) { return v1.currency() == v2.currency() ? v1.valueInCents() == v2.valueInCents() ? 0 : (v1.valueInCents() < v2.valueInCents() ? -1 : 1) : (v1.currency() < v2.currency() ? -1 : 1); }
	};

	template<>
	struct boost_any_traits<Entity::Type> : public boost_any_traits<int>
	{
		static Wt::WString asString(const Entity::Type &value, const Wt::WString &)
		{
			switch(value)
			{
			case Entity::PersonType: return Wt::WString::tr("Person");
			case Entity::BusinessType: return Wt::WString::tr("Business");
			default: return Wt::WString::tr("Unknown");
			}
		}
	};

	template<>
	struct boost_any_traits<Account::Type> : public boost_any_traits<int>
	{
		static Wt::WString asString(const Account::Type &value, const Wt::WString &)
		{
			switch(value)
			{
			case Account::EntityBalanceAccount: return Wt::WString::tr("EntityBalanceAccount");
			case Account::EntityPnlAccount: return Wt::WString::tr("EntityPnlAccount");
			case Account::Asset: return Wt::WString::tr("Asset");
			case Account::Liability: return Wt::WString::tr("Liability");
			default: return Wt::WString::tr("Unknown");
			}
		}
	};

	template<>
	struct boost_any_traits<Wt::WFlags<Entity::SpecificType>> : public boost_any_traits<int>
	{
		static Wt::WString asString(const Wt::WFlags<Entity::SpecificType> &value, const Wt::WString &)
		{
			std::string result;
			if(value & Entity::EmployeeType)
				result += Wt::WString::tr("Employee").toUTF8() + ", ";
			if(value & Entity::PersonnelType)
				result += Wt::WString::tr("Personnel").toUTF8() + ", ";
			if(value & Entity::ClientType)
				result += Wt::WString::tr("Client").toUTF8() + ", ";

			result = result.substr(0, result.size() - 2);
			return Wt::WString::fromUTF8(result);
		}
	};

	template<>
	struct boost_any_traits<BloodType> : public boost_any_traits<int>
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
			default: return Wt::WString::tr("Unknown");
			}
		}
	};

	template<>
	struct boost_any_traits<MaritalStatus> : public boost_any_traits<int>
	{
		static Wt::WString asString(const MaritalStatus &value, const Wt::WString &)
		{
			switch(value)
			{
			case Married: return Wt::WString::tr("Married");
			case Unmarried: return Wt::WString::tr("Unmarried");
			default: return Wt::WString::tr("Unknown");
			}
		}
	};

	template<>
	struct boost_any_traits<CycleInterval> : public boost_any_traits<int>
	{
		static Wt::WString asString(const CycleInterval &value, const Wt::WString &)
		{
			switch(value)
			{
			case DailyInterval: return Wt::WString::tr("Daily");
			case WeeklyInterval: return Wt::WString::tr("Weekly");
			case MonthlyInterval: return Wt::WString::tr("Monthly");
			case YearlyInterval: return Wt::WString::tr("Yearly");
			default: return "";
			}
		}
	};

	template<>
	struct boost_any_traits<IncomeCycle::Purpose> : public boost_any_traits<int>
	{
		static Wt::WString asString(const IncomeCycle::Purpose &value, const Wt::WString &)
		{
			switch(value)
			{
			case IncomeCycle::UnspecifiedPurpose: return Wt::WString::tr("UnspecificPurpose");
			case IncomeCycle::Services: return Wt::WString::tr("Services");
			default: return Wt::WString::tr("Unknown");
			}
		}
	};

	template<>
	struct boost_any_traits<ExpenseCycle::Purpose> : public boost_any_traits<int>
	{
		static Wt::WString asString(const ExpenseCycle::Purpose &value, const Wt::WString &)
		{
			switch(value)
			{
			case ExpenseCycle::UnspecifiedPurpose: return Wt::WString::tr("UnspecificPurpose");
			case ExpenseCycle::Salary: return Wt::WString::tr("Salary");
			default: return Wt::WString::tr("Unknown");
			}
		}
	};

	template<>
	struct boost_any_traits<EmployeePosition::Type> : public boost_any_traits<int>
	{
		static Wt::WString asString(const EmployeePosition::Type &value, const Wt::WString &)
		{
			switch(value)
			{
			case EmployeePosition::OtherType: return Wt::WString::tr("Other");
			case EmployeePosition::PersonnelType: return Wt::WString::tr("PersonnelPosition");
			default: return Wt::WString::tr("Unknown");
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

#endif
