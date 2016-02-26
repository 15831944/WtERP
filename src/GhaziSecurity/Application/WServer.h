#ifndef GS_WSERVER_H
#define GS_WSERVER_H

#include <Wt/WServer>

#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/OAuthService>

#include <Wt/Dbo/SqlConnectionPool>
#include <Wt/Dbo/Session>

namespace WW
{
	class ConfigurationsDatabase;
}

namespace GS
{
	class WApplication;
	class TaskScheduler;

	typedef std::vector<const Wt::Auth::OAuthService*> OAuthServiceMap;

	class WServer : public Wt::WServer
	{
	public:
		WServer(const std::string& wtApplicationPath = std::string(), const std::string& wtConfigurationFile = std::string());
		virtual ~WServer() override;

		static WServer *instance() { return dynamic_cast<WServer*>(Wt::WServer::instance()); }
		bool start();

		Wt::Dbo::SqlConnectionPool *sqlPool() const { return _sqlPool; }
		WW::ConfigurationsDatabase *configs() const { return _configs; }

		const Wt::Auth::AuthService &getAuthService() const { return _authService; }
		const Wt::Auth::PasswordService &getPasswordService() const { return _passwordService; }
		const OAuthServiceMap &getOAuthServices() const { return _oAuthServices; }

	protected:
		void configureAuth();
		void initialize();

		Wt::Dbo::Session _dboSession;
		Wt::Dbo::SqlConnectionPool *_sqlPool = nullptr;
		WW::ConfigurationsDatabase *_configs = nullptr;
		TaskScheduler *_taskScheduler = nullptr;

		Wt::Auth::AuthService _authService;
		Wt::Auth::PasswordService _passwordService;
		OAuthServiceMap _oAuthServices;

		boost::posix_time::ptime _ptBeforeLoad;
		boost::posix_time::ptime _ptStart;
	};
}

#define SERVER WServer::instance()

#endif