#include "Application/WServer.h"
#include "Dbo/Dbos.h"
#include "Dbo/ConfigurationsDatabase.h"
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

		Wt::Dbo::SqlConnection *sqlConnection = new Wt::Dbo::backend::MySQL("ghazisecurity", "root", "", "127.0.0.1");
		//Wt::Dbo::SqlConnection *sqlConnection = new Wt::Dbo::backend::Sqlite3(":memory:");
		sqlConnection->setProperty("show-queries", "true");
		_sqlPool = new Wt::Dbo::FixedSqlConnectionPool(sqlConnection, 1);

		log("success") << "Successfully connected to database";
	}
	catch(Wt::Dbo::Exception &e)
	{
		log("fatal") << "Database error connecting to database: " << e.what();
		throw e;
	}
	catch(std::exception &e)
	{
		log("fatal") << "Error connecting to database: " << e.what();
		throw e;
	}

	//Prepare server's Dbo::Session
	_dboSession.setConnectionPool(*_sqlPool);
	MAP_DBO_TREE(_dboSession)
	MAP_CONFIGURATION_DBO_TREE(_dboSession)

	/* *************************************************************************
	* ***************************  Create Tables  *****************************
	* *************************************************************************/
//#define REINSTALL_DBO
#ifdef REINSTALL_DBO
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
		throw e;
	}
	catch(std::exception &e)
	{
		log("fatal") << "Error creating tables: " << e.what();
		throw e;
	}

	{
		Wt::Dbo::Transaction tr(_dboSession);

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

		tr.commit();
	}
#endif

	//Load configurations
	try
	{
		_configs = new WW::ConfigurationsDatabase(_dboSession);
	}
	catch(Wt::Dbo::Exception &e)
	{
		log("fatal") << "Database error loading configurations: " << e.what();
		throw e;
	}
	catch(std::exception &e)
	{
		log("fatal") << "Error loading configurations: " << e.what();
		throw e;
	}

	//Start task scheduler
	_taskScheduler = new TaskScheduler(this, _dboSession);

	//Register boost::any traits
	Wt::registerType<Entity::Type>();
	Wt::registerType<Account::Type>();
	Wt::registerType<Wt::WFlags<Entity::SpecificType>>();
	Wt::registerType<Wt::Dbo::ptr<Entity>>();

	//Configure authorization module
	configureAuth();
}

WServer::~WServer()
{
	for(OAuthServiceMap::size_type i = 0; i < _oAuthServices.size(); ++i)
		delete _oAuthServices[i];

	delete _taskScheduler;
	delete _configs;
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
	_authService.setAuthTokensEnabled(true, "logintoken");
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
