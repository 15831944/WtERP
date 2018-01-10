#ifndef GS_WSERVER_H
#define GS_WSERVER_H

#include "Common.h"
#include <Wt/WServer.h>

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/OAuthService.h>

#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/Dbo/Session.h>

namespace WW
{
	class ConfigurationsDatabase;
}

namespace GS
{
	class WApplication;
	class TaskScheduler;
	class PermissionsDatabase;

	typedef std::vector<unique_ptr<const Wt::Auth::OAuthService>> OAuthServiceMap;

	class WServer : public Wt::WServer
	{
	public:
		WServer(int argc, char *argv[], const std::string &wtConfigurationFile = "");
		virtual ~WServer() override;;
		void initialize();

		static WServer *instance() { return dynamic_cast<WServer*>(Wt::WServer::instance()); }
		bool start();

		Dbo::SqlConnectionPool &sqlPool() const { return *_sqlPool; }
		WW::ConfigurationsDatabase &configs() const { return *_configs; }
		PermissionsDatabase &permissionsDatabase() const { return *_permissionsDatabase; }

		const Wt::Auth::AuthService &getAuthService() const { return _authService; }
		const Wt::Auth::PasswordService &getPasswordService() const { return _passwordService; }
		const OAuthServiceMap &getOAuthServices() const { return _oAuthServices; }

	protected:
		void configureAuth();

		Dbo::Session _dboSession;
		unique_ptr<Dbo::SqlConnectionPool> _sqlPool;
		unique_ptr<WW::ConfigurationsDatabase> _configs;
		unique_ptr<PermissionsDatabase> _permissionsDatabase;
		unique_ptr<TaskScheduler> _taskScheduler;

		Wt::Auth::AuthService _authService;
		Wt::Auth::PasswordService _passwordService;
		OAuthServiceMap _oAuthServices;

		steady_clock::time_point _tpBeforeLoad;
		steady_clock::time_point _tpStart;
	};
}

#define SERVER WServer::instance()

#endif