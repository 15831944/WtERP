#include "Application/WServer.h"
#include "Dbo/Dbos.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Dbo/PermissionsDatabase.h"
#include "Dbo/InventoryDatabase.h"
#include "Application/TaskScheduler.h"

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
#include <Wt/Date/tz.h>

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
		date::set_install(appRoot() + "tzdata2018e");

		auto resolver = make_shared<Wt::WMessageResourceBundle>();
		resolver->use(appRoot() + "strings", false); //CHECK_BEFORE_RELEASE
		resolver->use(appRoot() + "account_names", false); //CHECK_BEFORE_RELEASE
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

		std::string installDb;
		readConfigurationProperty("InstallDb", installDb);
		if(installDb == "true")
			_createTablesInDb();

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
		
		//Accounts Database
		_accountsDatabase = make_unique<AccountsDatabase>(_dboSession);
		
		//Inventory Database
		_inventoryDatabase = make_unique<InventoryDatabase>(_dboSession);
		
		if(installDb == "true")
			_insertDefaultValuesInDb();

		//Load permissions
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

// 		if(Wt::Auth::GoogleService::configured() && configurations()->getBool("GoogleOAuth", ModuleDatabase::Authentication, false))
// 			_oAuthServices.push_back(make_unique<Wt::Auth::GoogleService>(_authService));
// 		if(Wt::Auth::FacebookService::configured() && configurations()->getBool("FacebookOAuth", ModuleDatabase::Authentication, false))
// 			_oAuthServices.push_back(make_unique<Wt::Auth::FacebookService>(_authService));
	}
	
	void WServer::_createTablesInDb()
	{
		/*//Drop
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
		}*/
		
		//Create
		try
		{
			Dbo::Transaction t(_dboSession);
			_dboSession.createTables();
			createConfigurationDboIndices(_dboSession);
			createERPDboIndices(_dboSession);
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
	}
	
	void WServer::_insertDefaultValuesInDb()
	{
		try
		{
			Dbo::Transaction tr(_dboSession);
			
			insertERPDbos(_dboSession);
			_accountsDatabase->createDefaultAccounts();

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
}
