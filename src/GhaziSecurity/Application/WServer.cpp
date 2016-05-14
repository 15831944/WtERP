#include "Application/WServer.h"
#include "Dbo/Dbos.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Dbo/PermissionsDatabase.h"
#include "Utilities/TaskScheduler.h"

#include <Wt/WMessageResourceBundle>
#include <Wt/Dbo/FixedSqlConnectionPool>
#include <Wt/Dbo/backend/MySQL>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/Exception>

#include <Wt/Auth/HashFunction>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/GoogleService>
#include <Wt/Auth/FacebookService>
#include <Wt/Auth/Dbo/UserDatabase>

namespace GS
{

WServer::WServer(int argc, char *argv[], const std::string &wtConfigurationFile)
	: Wt::WServer(argv[0], wtConfigurationFile), _passwordService(_authService)
{
	_ptBeforeLoad = boost::posix_time::microsec_clock::local_time();
	setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
	initialize();
}

void WServer::initialize()
{
	auto resolver = new Wt::WMessageResourceBundle();
	resolver->use(appRoot() + "strings", false); //CHECK_BEFORE_RELEASE
	setLocalizedStrings(resolver);

	/* *************************************************************************
	* ***********************  Connect to SQL Server  *************************
	* *************************************************************************/
	try
	{
		log("info") << "Connecting to database backend";

		std::string dbBackend;
		if(!readConfigurationProperty("DbBackend", dbBackend))
			dbBackend = "SQLite";

		Wt::Dbo::SqlConnection *sqlConnection;
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

			sqlConnection = new Wt::Dbo::backend::MySQL(db, user, password, host, port);
		}
		else if(dbBackend == "SQLite")
		{
			std::string db = ":memory:";
			readConfigurationProperty("SQLiteDb", db);
			sqlConnection = new Wt::Dbo::backend::Sqlite3(db);
		}
		else
		{
			throw std::runtime_error("Invalid 'DbBackend' configuration: \"" + dbBackend + "\"");
		}

		sqlConnection->setProperty("show-queries", "true");
		_sqlPool = new Wt::Dbo::FixedSqlConnectionPool(sqlConnection, 1);

		log("success") << "Successfully connected to database";
	}
	catch(Wt::Dbo::Exception &e)
	{
		log("fatal") << "Database error connecting to database: " << e.what();
		throw;
	}
	catch(std::exception &e)
	{
		log("fatal") << "Error connecting to database: " << e.what();
		throw;
	}

	//Prepare server's Dbo::Session
	_dboSession.setConnectionPool(*_sqlPool);
	mapDboTree(_dboSession);
	WW::Dbo::mapConfigurationDboTree(_dboSession);

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
		catch(Wt::Dbo::Exception &e)
		{
			log("error") << "Database error dropping tables: " << e.what();
		}
		catch(std::exception &e)
		{
			log("error") << "Error dropping tables: " << e.what();
		}

		//Create
		try
		{
			Wt::Dbo::Transaction t(_dboSession);
			_dboSession.createTables();
			_dboSession.execute("CREATE UNIQUE INDEX unique_configuration ON " + std::string(WW::Dbo::Configuration::tableName()) + " (name, type)");
			t.commit();
		}
		catch(Wt::Dbo::Exception &e)
		{
			log("fatal") << "Database error creating tables: " << e.what();
			throw;
		}
		catch(std::exception &e)
		{
			log("fatal") << "Error creating tables: " << e.what();
			throw;
		}

		try
		{
			Wt::Dbo::Transaction tr(_dboSession);

			//Countries and cities
			auto pakistan = new Country("PK");
			pakistan->name = "Pakistan";
			auto pakistanPtr = _dboSession.add(pakistan);

			auto karachi = new City(pakistanPtr);
			karachi->name = "Karachi";
			_dboSession.add(karachi);

			auto hyderabad = new City(pakistanPtr);
			hyderabad->name = "Hyderabad";
			_dboSession.add(hyderabad);

			auto sukhur = new City(pakistanPtr);
			sukhur->name = "Sukhur";
			_dboSession.add(sukhur);

			auto Quetta = new City(pakistanPtr);
			Quetta->name = "Quetta";
			_dboSession.add(Quetta);

			auto Multan = new City(pakistanPtr);
			Multan->name = "Multan";
			_dboSession.add(Multan);

			auto Lahore = new City(pakistanPtr);
			Lahore->name = "Lahore";
			_dboSession.add(Lahore);

			auto Islamabad = new City(pakistanPtr);
			Islamabad->name = "Islamabad";
			_dboSession.add(Islamabad);

			auto Peshawar = new City(pakistanPtr);
			Peshawar->name = "Peshawar";
			_dboSession.add(Peshawar);

			auto Faisalabad = new City(pakistanPtr);
			Faisalabad->name = "Faisalabad";
			_dboSession.add(Faisalabad);

			//Permissions
			auto accessAdminPanel = _dboSession.add(new Permission(Permissions::AccessAdminPanel, "Access Admin Panel"));

			auto createRecord = _dboSession.add(new Permission(Permissions::CreateRecord, "Create records"));
			auto modifyRecord = _dboSession.add(new Permission(Permissions::ModifyRecord, "Modify records"));
			auto removeRecord = _dboSession.add(new Permission(Permissions::RemoveRecord, "Remove records"));

			auto createUser = _dboSession.add(new Permission(Permissions::CreateUser, "Create users"));
			auto modifyUser = _dboSession.add(new Permission(Permissions::ModifyUser, "Modify users"));
			auto modifyUserPermission = _dboSession.add(new Permission(Permissions::ModifyUserPermission, "Modify user permissions"));
			auto removeUser = _dboSession.add(new Permission(Permissions::RemoveUser, "Remove users"));

			auto viewUnassignedUserRecord = _dboSession.add(new Permission(Permissions::ViewUnassignedUserRecord, "View records not assigned to any user"));
			auto viewOtherUserRecord = _dboSession.add(new Permission(Permissions::ViewOtherUserRecord, "View records created by other users"));
			auto modifyOtherUserRecord = _dboSession.add(new Permission(Permissions::ModifyOtherUserRecord, "Modify records created by other users"));
			auto removeOtherUserRecord = _dboSession.add(new Permission(Permissions::RemoveOtherUserRecord, "Remove records created by other users"));

			auto viewUnassignedRegionRecord = _dboSession.add(new Permission(Permissions::ViewUnassignedRegionRecord, "View records not assigned to any region"));
			auto viewOtherRegionRecord = _dboSession.add(new Permission(Permissions::ViewOtherRegionRecord, "View records created by other regions"));
			auto modifyOtherRegionRecord = _dboSession.add(new Permission(Permissions::ModifyOtherRegionRecord, "Modify records created by other regions"));
			auto removeOtherRegionRecord = _dboSession.add(new Permission(Permissions::RemoveOtherRegionRecord, "Remove records created by other regions"));

			auto globalAdministrator = _dboSession.add(new Permission(Permissions::GlobalAdministrator, "Global administrator permissions"));
			globalAdministrator.modify()->linkedToCollection.insert(accessAdminPanel);
			globalAdministrator.modify()->linkedToCollection.insert(createRecord);
			globalAdministrator.modify()->linkedToCollection.insert(modifyRecord);
			globalAdministrator.modify()->linkedToCollection.insert(removeRecord);
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

			auto regionalAdministrator = _dboSession.add(new Permission(Permissions::RegionalAdministrator, "Regional administrator permissions"));
			regionalAdministrator.modify()->linkedToCollection.insert(accessAdminPanel);
			regionalAdministrator.modify()->linkedToCollection.insert(createRecord);
			regionalAdministrator.modify()->linkedToCollection.insert(modifyRecord);
			regionalAdministrator.modify()->linkedToCollection.insert(removeRecord);
			regionalAdministrator.modify()->linkedToCollection.insert(createUser);
			regionalAdministrator.modify()->linkedToCollection.insert(modifyUser);
			regionalAdministrator.modify()->linkedToCollection.insert(modifyUserPermission);
			regionalAdministrator.modify()->linkedToCollection.insert(removeUser);
			regionalAdministrator.modify()->linkedToCollection.insert(viewUnassignedUserRecord);
			regionalAdministrator.modify()->linkedToCollection.insert(viewOtherUserRecord);
			regionalAdministrator.modify()->linkedToCollection.insert(modifyOtherUserRecord);
			regionalAdministrator.modify()->linkedToCollection.insert(removeOtherUserRecord);
			regionalAdministrator.modify()->linkedToCollection.insert(viewUnassignedRegionRecord);
			regionalAdministrator.modify()->linkedToCollection.insert(viewOtherRegionRecord);

			auto regionalUser = _dboSession.add(new Permission(Permissions::RegionalUser, "Regional user permissions"));
			regionalUser.modify()->linkedToCollection.insert(accessAdminPanel);
			regionalUser.modify()->linkedToCollection.insert(createRecord);
			regionalUser.modify()->linkedToCollection.insert(modifyRecord);
			regionalUser.modify()->linkedToCollection.insert(removeRecord);
			regionalUser.modify()->linkedToCollection.insert(viewUnassignedUserRecord);
			regionalUser.modify()->linkedToCollection.insert(viewOtherUserRecord);
			regionalUser.modify()->linkedToCollection.insert(viewUnassignedRegionRecord);
			regionalUser.modify()->linkedToCollection.insert(viewOtherRegionRecord);

			//Users
			{
				WServer *server = SERVER;
				UserDatabase userDatabase(_dboSession);
				Wt::Auth::User authUser;
				Wt::Dbo::ptr<AuthInfo> authInfoPtr;
				Wt::Dbo::ptr<User> userPtr;

				authUser = userDatabase.registerNew();
				authUser.setIdentity(Wt::Auth::Identity::LoginName, "admin");
				server->getPasswordService().updatePassword(authUser, "changeme");
				authInfoPtr = userDatabase.find(authUser);
				userPtr = _dboSession.add(new User());
				authInfoPtr.modify()->setUser(userPtr);
				_dboSession.add(new UserPermission(userPtr, globalAdministrator));

				authUser = userDatabase.registerNew();
				authUser.setIdentity(Wt::Auth::Identity::LoginName, "regionaladmin");
				server->getPasswordService().updatePassword(authUser, "changeme");
				authInfoPtr = userDatabase.find(authUser);
				userPtr = _dboSession.add(new User());
				authInfoPtr.modify()->setUser(userPtr);
				_dboSession.add(new UserPermission(userPtr, regionalAdministrator));

				authUser = userDatabase.registerNew();
				authUser.setIdentity(Wt::Auth::Identity::LoginName, "regionaluser");
				server->getPasswordService().updatePassword(authUser, "changeme");
				authInfoPtr = userDatabase.find(authUser);
				userPtr = _dboSession.add(new User());
				authInfoPtr.modify()->setUser(userPtr);
				_dboSession.add(new UserPermission(userPtr, regionalUser));

				authUser = userDatabase.registerNew();
				authUser.setIdentity(Wt::Auth::Identity::LoginName, "test");
				server->getPasswordService().updatePassword(authUser, "changeme");
				authInfoPtr = userDatabase.find(authUser);
				userPtr = _dboSession.add(new User());
				authInfoPtr.modify()->setUser(userPtr);
			}

			tr.commit();
		}
		catch(Wt::Dbo::Exception &e)
		{
			log("fatal") << "Database error inserting default values: " << e.what();
			throw;
		}
		catch(std::exception &e)
		{
			log("fatal") << "Error inserting default values: " << e.what();
			throw;
		}
	}

	//Load configurations
	try
	{
		_configs = new WW::ConfigurationsDatabase(_dboSession);
	}
	catch(Wt::Dbo::Exception &e)
	{
		log("fatal") << "Database error loading configurations: " << e.what();
		throw;
	}
	catch(std::exception &e)
	{
		log("fatal") << "Error loading configurations: " << e.what();
		throw;
	}

	//Load configurations
	try
	{
		_permissionsDatabase = new PermissionsDatabase(_dboSession);
	}
	catch(Wt::Dbo::Exception &e)
	{
		log("fatal") << "Database error loading permissions: " << e.what();
		throw;
	}
	catch(std::exception &e)
	{
		log("fatal") << "Error loading permissions: " << e.what();
		throw;
	}

	//Start task scheduler
	_taskScheduler = new TaskScheduler(this, _dboSession);

	//Register boost::any traits
	Wt::registerType<Money>();
	Wt::registerType<Entity::Type>();
	Wt::registerType<Account::Type>();
	Wt::registerType<EmployeePosition::Type>();
	Wt::registerType<Wt::WFlags<Entity::SpecificType>>();
	//Wt::registerType<Wt::Dbo::ptr<Entity>>();
}

WServer::~WServer()
{
	for(OAuthServiceMap::size_type i = 0; i < _oAuthServices.size(); ++i)
		delete _oAuthServices[i];

	delete _taskScheduler;
	delete _configs;
	delete _permissionsDatabase;
	delete _sqlPool; //Also deletes SQLConnections
	delete localizedStrings();
}

bool WServer::start()
{
	if(Wt::WServer::start())
	{
		//Load Finish Time
		_ptStart = boost::posix_time::microsec_clock::local_time();

		log("success") << "Server successfully started! Time taken to start: "
			<< boost::posix_time::time_duration(_ptStart - _ptBeforeLoad).total_milliseconds()
			<< " ms";
		return true;
	}
	return false;
}

void WServer::configureAuth()
{
	_authService.setAuthTokensEnabled(true, "authtoken");
	_authService.setAuthTokenValidity(24 * 60);
	_authService.setIdentityPolicy(Wt::Auth::LoginNameIdentity);
	_authService.setEmailVerificationEnabled(false);

	//Hash and throttling
	Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
	verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
	_passwordService.setVerifier(verifier);
	_passwordService.setAttemptThrottlingEnabled(true);

	//Password strength
	Wt::Auth::PasswordStrengthValidator *strengthValidator = new Wt::Auth::PasswordStrengthValidator();
	strengthValidator->setMinimumLength(Wt::Auth::PasswordStrengthValidator::OneCharClass, 6);
	_passwordService.setStrengthValidator(strengthValidator);

// 	if(Wt::Auth::GoogleService::configured() && configurations()->getBool("GoogleOAuth", ModuleDatabase::Authentication, false))
// 		_oAuthServices.push_back(new Wt::Auth::GoogleService(_authService));
// 	if(Wt::Auth::FacebookService::configured() && configurations()->getBool("FacebookOAuth", ModuleDatabase::Authentication, false))
// 		_oAuthServices.push_back(new Wt::Auth::FacebookService(_authService));
}


}
