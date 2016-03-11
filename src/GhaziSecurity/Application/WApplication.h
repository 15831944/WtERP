#ifndef GS_WAPPLICATION_H
#define GS_WAPPLICATION_H

#include <Wt/WApplication>
#include <Wt/Auth/Login>
#include <Wt/Auth/Dbo/UserDatabase>

#include "Dbo/Dbos.h"
#include "Dbo/EntitiesDatabase.h"
#include "Dbo/AccountsDatabase.h"

namespace GS
{
	class CountryProxyModel;
	class EntitiesAdminPage;
	class PositionProxyModel;
	class ServiceProxyModel;
	class AccountsAdminPage;
	class FindEntityModel;
	class FindEntityFilterModel;
	class FindAccountModel;
	//class LocationFilterModel;

	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<Location>> LocationQueryModel;
	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<Country>> CountryQueryModel;
	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<City>> CityQueryModel;
	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<EmployeePosition>> PositionQueryModel;
	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<ClientService>> ServiceQueryModel;

	typedef Wt::Auth::Dbo::UserDatabase<GS::AuthInfo> UserDatabase;

	class Session : public Wt::Dbo::Session
	{
	public:
		Session();

		Wt::Dbo::ptr<GS::User> user();
		Wt::Dbo::ptr<GS::User> user(const Wt::Auth::User &authUser);

		UserDatabase &userDatabase() { return _userDatabase; }
		Wt::Auth::Login &login() { return _login; }

	private:
		UserDatabase _userDatabase;
		Wt::Auth::Login _login;
	};

	class WApplication : public Wt::WApplication
	{
	public:
		WApplication(const Wt::WEnvironment& env);
		virtual ~WApplication() override;

		static WApplication *instance() { return dynamic_cast<WApplication*>(Wt::WApplication::instance()); }
		static WApplication *createApplication(const Wt::WEnvironment &env) { return new WApplication(env); }

		Session &session() { return _session; }
		EntitiesDatabase &entitiesDatabase() { return _entitiesDatabase; }
		AccountsDatabase &accountsDatabase() { return _accountsDatabase; }

		//Widgets
		Wt::WTemplate *mainTemplate() const { return _mainTemplate; }
		Wt::WNavigationBar *mainNavBar() const { return _navBar; }
		Wt::WMenu *mainMenu() const { return _mainMenu; }
		Wt::WMenu *adminMenu() const { return _adminMenu; }
		Wt::WStackedWidget *mainStack() const { return _mainStack; }
		Wt::WStackedWidget *adminStack() const { return _adminStack; }
		EntitiesAdminPage *entitiesAdminPage() const { return _entitiesAdminPage; }
		AccountsAdminPage *accountsAdminPage() const { return _accountsAdminPage; }

		//Error handling
		Wt::WDialog *errorDialog() const { return _errorDialog; }
		void showErrorDialog(const Wt::WString &message);
		void showStaleObjectError(const Wt::WString &recordName) { showErrorDialog(Wt::WString::tr("StaleObjectError").arg(recordName)); }
		void showDbBackendError(const std::string &code);

		//QueryModels
		FindEntityModel *findEntityModel() const { return _findEntityModel; }
		FindEntityFilterModel *findPersonModel() const { return _findPersonFilterModel; }
		FindEntityFilterModel *findBusinessModel() const { return _findBusinessFilterModel; }
		void initFindEntityModel();

		FindAccountModel *findAccountModel() const { return _findAccountModel; }
		void initFindAccountModel();

		CountryQueryModel *countryQueryModel() const { return _countryQueryModel; }
		CountryProxyModel *countryProxyModel() const { return _countryProxyModel; }
		void initCountryQueryModel();

		CityQueryModel *cityQueryModel() const { return _cityQueryModel; }
		void initCityQueryModel();

		PositionQueryModel *positionQueryModel() const { return _positionQueryModel; }
		PositionProxyModel *positionProxyModel() const { return _positionProxyModel; }
		void initPositionQueryModel();

		ServiceQueryModel *serviceQueryModel() const { return _serviceQueryModel; }
		ServiceProxyModel *serviceProxyModel() const { return _serviceProxyModel; }
		void initServiceQueryModel();

// 		LocationQueryModel *locationQueryModel() const { return _locationQueryModel; }
// 		LocationFilterModel *findLocationFilter() const { return _locationFilterModel; }
// 		void initLocationQueryModel();

	protected:
		void handleAuthChanged();
		void handleInternalPathChanged(std::string path);

		void lazyLoadAdminWidgets();

		//Widgets
		Wt::WTemplate *_mainTemplate = nullptr;
		Wt::WNavigationBar *_navBar = nullptr;
		Wt::WMenu *_mainMenu = nullptr;
		Wt::WMenu *_adminMenu = nullptr;
		Wt::WStackedWidget *_mainStack = nullptr;
		Wt::WStackedWidget *_adminStack = nullptr;
		Wt::WDialog *_errorDialog = nullptr;
		Wt::WText *_errorDialogText = nullptr;
		EntitiesAdminPage *_entitiesAdminPage = nullptr;
		AccountsAdminPage *_accountsAdminPage = nullptr;

		FindEntityModel *_findEntityModel = nullptr;
		FindEntityFilterModel *_findPersonFilterModel = nullptr;
		FindEntityFilterModel *_findBusinessFilterModel = nullptr;
		FindAccountModel *_findAccountModel = nullptr;
		CountryQueryModel *_countryQueryModel = nullptr;
		CountryProxyModel *_countryProxyModel = nullptr;
		CityQueryModel *_cityQueryModel = nullptr;
		PositionQueryModel *_positionQueryModel = nullptr;
		PositionProxyModel *_positionProxyModel = nullptr;
		ServiceQueryModel *_serviceQueryModel = nullptr;
		ServiceProxyModel *_serviceProxyModel = nullptr;
// 		LocationQueryModel *_locationQueryModel = nullptr;
// 		LocationFilterModel *_locationFilterModel = nullptr;

		Session _session;
		EntitiesDatabase _entitiesDatabase;
		AccountsDatabase _accountsDatabase;
		boost::posix_time::ptime _startTime;
	};
}

#define APP WApplication::instance()

#endif