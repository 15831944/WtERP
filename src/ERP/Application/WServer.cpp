#include "Application/WServer.h"
#include "Dbo/Dbos.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Dbo/PermissionsDatabase.h"
#include "Utilities/TaskScheduler.h"

#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/Dbo/backend/MySQL.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/GoogleService.h>
#include <Wt/Auth/FacebookService.h>
#include <Wt/Auth/Dbo/UserDatabase.h>

#include <boost/lexical_cast.hpp>

namespace ERP
{
	WServer::WServer(int argc, char *argv[], const std::string &wtConfigurationFile)
		: Wt::WServer(argv[0], wtConfigurationFile), _passwordService(_authService)
	{
		_tpBeforeLoad = steady_clock::now();
		setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
	}

	WServer::~WServer() = default;

	void WServer::initialize()
	{
		auto resolver = make_shared<Wt::WMessageResourceBundle>();
		resolver->use(appRoot() + "strings", false); //CHECK_BEFORE_RELEASE
		setLocalizedStrings(resolver);

		/* *************************************************************************
		* ***********************  Connect to SQL Server  *************************
		* *************************************************************************/
		try
		{
			log("erp-info") << "Connecting to database backend";

			std::string dbBackend;
			if(!readConfigurationProperty("DbBackend", dbBackend))
				dbBackend = "SQLite";

			unique_ptr<Dbo::SqlConnection> sqlConnection;
			if(dbBackend == "MySQL")
			{
				std::string host, portStr, db, user, password;
				readConfigurationProperty("MySQLHost", host);
				readConfigurationProperty("MySQLPort", portStr);
				readConfigurationProperty("MySQLDb", db);
				readConfigurationProperty("MySQLUser", user);
				readConfigurationProperty("MySQLPassword", password);

				unsigned int port = 0;
				if(!portStr.empty())
					port = boost::lexical_cast<unsigned int>(portStr);

				sqlConnection = make_unique<Dbo::backend::MySQL>(db, user, password, host, port);
			}
			else if(dbBackend == "SQLite")
			{
				std::string db = ":memory:";
				readConfigurationProperty("SQLiteDb", db);
				sqlConnection = make_unique<Dbo::backend::Sqlite3>(db);
			}
			else
			{
				throw std::runtime_error("Invalid 'DbBackend' configuration: \"" + dbBackend + "\"");
			}

			sqlConnection->setProperty("show-queries", "true");
			_sqlPool = make_unique<Dbo::FixedSqlConnectionPool>(move(sqlConnection), 1);

			log("success") << "Successfully connected to database";
		}
		catch(const Dbo::Exception &e)
		{
			log("fatal") << "Database error connecting to database: " << e.what();
			throw;
		}
		catch(const std::exception &e)
		{
			log("fatal") << "Error connecting to database: " << e.what();
			throw;
		}

		//Prepare server's DboSession
		_dboSession.setConnectionPool(*_sqlPool);
		mapERPDbos(_dboSession);
		mapConfigurationDbos(_dboSession);

		//Configure authorization module
		configureAuth();

		/* *************************************************************************
		* ***************************  Create Tables  *****************************
		* *************************************************************************/

		std::string reinstallDb;
		readConfigurationProperty("ReinstallDb", reinstallDb);
		if(reinstallDb == "true")
		{
			//Drop
			try
			{
				_dboSession.dropTables();
			}
			catch(const Dbo::Exception &e)
			{
				log("error") << "Database error dropping tables: " << e.what();
			}
			catch(const std::exception &e)
			{
				log("error") << "Error dropping tables: " << e.what();
			}

			//Create
			try
			{
				Dbo::Transaction t(_dboSession);
				_dboSession.createTables();
				_dboSession.execute("CREATE UNIQUE INDEX unique_configuration ON " + Configuration::tStr() + " (name, type)");
				t.commit();
			}
			catch(const Dbo::Exception &e)
			{
				log("fatal") << "Database error creating tables: " << e.what();
				throw;
			}
			catch(const std::exception &e)
			{
				log("fatal") << "Error creating tables: " << e.what();
				throw;
			}

			try
			{
				Dbo::Transaction tr(_dboSession);

				//Countries
				auto pakistan = _dboSession.addNew<Country>("PK", "Pakistan");
				//Cities
				auto karachi = _dboSession.addNew<City>(pakistan, "Karachi");
				auto hyderabad = _dboSession.addNew<City>(pakistan, "Hyderabad");
				auto sukhur = _dboSession.addNew<City>(pakistan, "Sukhur");
				auto Quetta = _dboSession.addNew<City>(pakistan, "Quetta");
				auto Multan = _dboSession.addNew<City>(pakistan, "Multan");
				auto Lahore = _dboSession.addNew<City>(pakistan, "Lahore");
				auto Islamabad = _dboSession.addNew<City>(pakistan, "Islamabad");
				auto Peshawar = _dboSession.addNew<City>(pakistan, "Peshawar");
				auto Faisalabad = _dboSession.addNew<City>(pakistan, "Faisalabad");

				//Permissions
				auto accessAdminPanel = _dboSession.addNew<Permission>(Permissions::AccessAdminPanel, "Access Admin Panel");

				auto createRecord = _dboSession.addNew<Permission>(Permissions::CreateRecord, "Create records");
				auto modifyRecord = _dboSession.addNew<Permission>(Permissions::ModifyRecord, "Modify records");
				auto removeRecord = _dboSession.addNew<Permission>(Permissions::RemoveRecord, "Remove records");

				auto viewUser = _dboSession.addNew<Permission>(Permissions::ViewUser, "View users");
				auto createUser = _dboSession.addNew<Permission>(Permissions::CreateUser, "Create users");
				auto modifyUser = _dboSession.addNew<Permission>(Permissions::ModifyUser, "Modify users");
				auto modifyUserPermission = _dboSession.addNew<Permission>(Permissions::ModifyUserPermission, "Modify user permissions");
				auto removeUser = _dboSession.addNew<Permission>(Permissions::RemoveUser, "Remove users");

				auto viewUnassignedUserRecord = _dboSession.addNew<Permission>(Permissions::ViewUnassignedUserRecord, "View records not assigned to any user");
				auto viewOtherUserRecord = _dboSession.addNew<Permission>(Permissions::ViewOtherUserRecord, "View records created by other users");
				auto modifyOtherUserRecord = _dboSession.addNew<Permission>(Permissions::ModifyOtherUserRecord, "Modify records created by other users");
				auto removeOtherUserRecord = _dboSession.addNew<Permission>(Permissions::RemoveOtherUserRecord, "Remove records created by other users");

				auto viewUnassignedRegionRecord = _dboSession.addNew<Permission>(Permissions::ViewUnassignedRegionRecord, "View records not assigned to any region");
				auto viewOtherRegionRecord = _dboSession.addNew<Permission>(Permissions::ViewOtherRegionRecord, "View records created by other regions");
				auto modifyOtherRegionRecord = _dboSession.addNew<Permission>(Permissions::ModifyOtherRegionRecord, "Modify records created by other regions");
				auto removeOtherRegionRecord = _dboSession.addNew<Permission>(Permissions::RemoveOtherRegionRecord, "Remove records created by other regions");

				auto viewRegion = _dboSession.addNew<Permission>(Permissions::ViewRegion, "View regions");
				auto createRegion = _dboSession.addNew<Permission>(Permissions::CreateRegion, "Create regions");
				auto modifyRegion = _dboSession.addNew<Permission>(Permissions::ModifyRegion, "Modify regions");

				auto globalAdministrator = _dboSession.addNew<Permission>(Permissions::GlobalAdministrator, "Global administrator permissions");
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

				auto regionalAdministrator = _dboSession.addNew<Permission>(Permissions::RegionalAdministrator, "Regional administrator permissions");
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

				auto regionalUser = _dboSession.addNew<Permission>(Permissions::RegionalUser, "Regional user permissions");
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
					UserDatabase userDatabase(_dboSession);
					Wt::Auth::User authUser;
					Dbo::ptr<AuthInfo> authInfoPtr;
					Dbo::ptr<User> userPtr;

					authUser = userDatabase.registerNew();
					authUser.setIdentity(Wt::Auth::Identity::LoginName, "admin");
					server->getPasswordService().updatePassword(authUser, "changeme");
					authInfoPtr = userDatabase.find(authUser);
					userPtr = _dboSession.addNew<User>();
					authInfoPtr.modify()->setUser(userPtr);
					_dboSession.addNew<UserPermission>(userPtr, globalAdministrator);

	// 				authUser = userDatabase.registerNew();
	// 				authUser.setIdentity(Wt::Auth::Identity::LoginName, "regionaladmin");
	// 				server->getPasswordService().updatePassword(authUser, "changeme");
	// 				authInfoPtr = userDatabase.find(authUser);
	// 				userPtr = _dboSession.addNew<User>();
	// 				authInfoPtr.modify()->setUser(userPtr);
	// 				_dboSession.addNew<UserPermission>(userPtr, regionalAdministrator);
	// 
	// 				authUser = userDatabase.registerNew();
	// 				authUser.setIdentity(Wt::Auth::Identity::LoginName, "regionaluser");
	// 				server->getPasswordService().updatePassword(authUser, "changeme");
	// 				authInfoPtr = userDatabase.find(authUser);
	// 				userPtr = _dboSession.addNew<User>();
	// 				authInfoPtr.modify()->setUser(userPtr);
	// 				_dboSession.addNew<UserPermission>(userPtr, regionalUser);
	// 
	// 				authUser = userDatabase.registerNew();
	// 				authUser.setIdentity(Wt::Auth::Identity::LoginName, "test");
	// 				server->getPasswordService().updatePassword(authUser, "changeme");
	// 				authInfoPtr = userDatabase.find(authUser);
	// 				userPtr = _dboSession.addNew<User>();
	// 				authInfoPtr.modify()->setUser(userPtr);
				}

				tr.commit();
			}
			catch(const Dbo::Exception &e)
			{
				log("fatal") << "Database error inserting default values: " << e.what();
				throw;
			}
			catch(const std::exception &e)
			{
				log("fatal") << "Error inserting default values: " << e.what();
				throw;
			}
		}

		//Load configurations
		try
		{
			_configs = make_unique<ConfigurationsDatabase>(_dboSession);
		}
		catch(const Dbo::Exception &e)
		{
			log("fatal") << "Database error loading configurations: " << e.what();
			throw;
		}
		catch(const std::exception &e)
		{
			log("fatal") << "Error loading configurations: " << e.what();
			throw;
		}

		//Load configurations
		try
		{
			_permissionsDatabase = make_unique<PermissionsDatabase>(_dboSession);
		}
		catch(const Dbo::Exception &e)
		{
			log("fatal") << "Database error loading permissions: " << e.what();
			throw;
		}
		catch(const std::exception &e)
		{
			log("fatal") << "Error loading permissions: " << e.what();
			throw;
		}

		//Start task scheduler
		_taskScheduler = make_unique<TaskScheduler>(this, _dboSession);

		//Register Wt::any traits
		Wt::registerType<Money>();
		Wt::registerType<Entity::Type>();
		Wt::registerType<Account::Type>();
		Wt::registerType<Wt::WFlags<Entity::SpecificType>>();
	}

	bool WServer::start()
	{
		if(Wt::WServer::start())
		{
			//Load Finish Time
			_tpStart = steady_clock::now();

			log("success") << "Server successfully started! Time taken to start: "
				<< duration_cast<milliseconds>(_tpStart - _tpBeforeLoad).count()
				<< " ms";
			return true;
		}
		return false;
	}

	void WServer::configureAuth()
	{
		_authService.setAuthTokensEnabled(true, "authtoken");
		_authService.setAuthTokenValidity(24 * 60);
		_authService.setIdentityPolicy(Wt::Auth::IdentityPolicy::LoginName);
		_authService.setEmailVerificationEnabled(false);

		//Hash and throttling
		auto verifier = make_unique<Wt::Auth::PasswordVerifier>();
		verifier->addHashFunction(make_unique<Wt::Auth::BCryptHashFunction>(7));
		_passwordService.setVerifier(move(verifier));
		_passwordService.setAttemptThrottlingEnabled(true);

		//Password strength
		auto strengthValidator = make_unique<Wt::Auth::PasswordStrengthValidator>();
		strengthValidator->setMinimumLength(Wt::Auth::PasswordStrengthType::OneCharClass, 6);
		_passwordService.setStrengthValidator(move(strengthValidator));

	// 	if(Wt::Auth::GoogleService::configured() && configurations()->getBool("GoogleOAuth", ModuleDatabase::Authentication, false))
	// 		_oAuthServices.push_back(make_unique<Wt::Auth::GoogleService>(_authService));
	// 	if(Wt::Auth::FacebookService::configured() && configurations()->getBool("FacebookOAuth", ModuleDatabase::Authentication, false))
	// 		_oAuthServices.push_back(make_unique<Wt::Auth::FacebookService>(_authService));
	}
}
