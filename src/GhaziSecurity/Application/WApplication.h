#ifndef GS_WAPPLICATION_H
#define GS_WAPPLICATION_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Dbo/EntitiesDatabase.h"
#include "Dbo/AccountsDatabase.h"

#include <Wt/WApplication.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/WDialog.h>
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>

namespace GS
{
	using namespace std::chrono;

	class CountryProxyModel;
	class EntitiesAdminPage;
	class DashboardAdminPage;
	class AttendanceAdminPage;
	class UsersAdminPage;
	class PositionProxyModel;
	class ServiceProxyModel;
	class RegionProxyModel;
	class AccountsAdminPage;
	class AuthWidget;
	class AdminPageWidget;

	typedef Dbo::QueryModel<Dbo::ptr<Country>> CountryQueryModel;
	typedef Dbo::QueryModel<Dbo::ptr<City>> CityQueryModel;
	typedef Dbo::QueryModel<Dbo::ptr<EmployeePosition>> PositionQueryModel;
	typedef Dbo::QueryModel<Dbo::ptr<ClientService>> ServiceQueryModel;
	typedef Dbo::QueryModel<Dbo::ptr<Region>> RegionQueryModel;

	class AuthLogin : public Wt::Auth::Login
	{
	public:
		enum PermissionResult
		{
			Permitted,
			RequiresStrongLogin,
			Denied
		};

		AuthLogin();
		Dbo::ptr<AuthInfo> authInfoPtr() const { return _authInfoPtr; };
		Dbo::ptr<User> userPtr() const { return _authInfoPtr ? _authInfoPtr->user() : nullptr; }

		bool hasPermission(long long permissionId) { return checkPermission(permissionId) == Permitted; }
		PermissionResult checkPermission(long long permissionId);
		PermissionResult checkRecordViewPermission(const BaseAdminRecord *record);
		PermissionResult checkRecordViewPermission(const void *record);
		PermissionResult checkRecordModifyPermission(const BaseAdminRecord *record);
		PermissionResult checkRecordModifyPermission(const void *record);
		PermissionResult checkRecordCreatePermission() { return _recordCreatePermission; }

		bool canAccessAdminPanel() { return loggedIn() && hasPermission(Permissions::AccessAdminPanel); }

		template<class QueryResult>
		void setPermissionConditionsToQuery(Dbo::Query<QueryResult> &query, bool resultsIfLoggedOut = false, const std::string &fieldPrefix = "", bool modifyPermissionRequired = false);

	protected:
		enum class IdOptions
		{
			Default, //Can modify other
			IsNull, //CANT modify other
			NotNull, //Can modify other
			SelfId, //CANT modify other
			SelfIdOrNull //CANT modify other
		};

		void handleBeforeLoginChanged();
		PermissionMap _permissions;
		PermissionResult _recordCreatePermission = Denied;
		Dbo::ptr<AuthInfo> _authInfoPtr;
	};

	class WApplication : public Wt::WApplication
	{
	public:
		WApplication(const Wt::WEnvironment &env);

		static WApplication *instance() { return dynamic_cast<WApplication*>(Wt::WApplication::instance()); }
		static unique_ptr<WApplication> createApplication(const Wt::WEnvironment &env) { return make_unique<WApplication>(env); }

		Dbo::Session &dboSession() { return _dboSession; }
		AuthLogin &authLogin() { return _login; }
		UserDatabase &userDatabase() { return _userDatabase; }
		EntitiesDatabase &entitiesDatabase() { return _entitiesDatabase; }
		AccountsDatabase &accountsDatabase() { return _accountsDatabase; }

		//Widgets
		Wt::WTemplate *mainTemplate() const { return _mainVisitorTemplate; }
		Wt::WNavigationBar *mainNavBar() const { return _visitorNavBar; }
		Wt::WMenu *mainMenu() const { return _visitorMenu; }
		Wt::WMenu *adminMenu() const { return _adminMenu; }
		Wt::WStackedWidget *mainStack() const { return _visitorStack; }
		Wt::WStackedWidget *adminStack() const { return _adminStack; }
		EntitiesAdminPage *entitiesAdminPage() const { return _entitiesAdminPage; }
		AccountsAdminPage *accountsAdminPage() const { return _accountsAdminPage; }

		//Error handling
		Wt::WDialog *errorDialog() const { return _errorDialog.get(); }
		void showErrorDialog(const Wt::WString &message);
		void showStaleObjectError() { showErrorDialog(tr("StaleObjectError")); }
		void showDbBackendError(const std::string &code);

		//SuggestionPopups
		Wt::WSuggestionPopup *findEntitySuggestion() const { return _findEntitySuggestion; }
		void initFindEntitySuggestion();
		Wt::WSuggestionPopup *findAccountSuggestion() const { return _findAccountSuggestion; }
		void initFindAccountSuggestion();
		Wt::WSuggestionPopup *findLocationSuggestion() const { return _findLocationSuggestion; }
		void initFindLocationSuggestion();

		//QueryModels
		shared_ptr<CountryQueryModel> countryQueryModel() const { return _countryQueryModel; }
		shared_ptr<CountryProxyModel> countryProxyModel() const { return _countryProxyModel; }
		void initCountryQueryModel();

		shared_ptr<CityQueryModel> cityQueryModel() const { return _cityQueryModel; }
		void initCityQueryModel();

		shared_ptr<PositionQueryModel> positionQueryModel() const { return _positionQueryModel; }
		shared_ptr<PositionProxyModel> positionProxyModel() const { return _positionProxyModel; }
		void initPositionQueryModel();

		shared_ptr<ServiceQueryModel> serviceQueryModel() const { return _serviceQueryModel; }
		shared_ptr<ServiceProxyModel> serviceProxyModel() const { return _serviceProxyModel; }
		void initServiceQueryModel();

		shared_ptr<RegionQueryModel> regionQueryModel() const { return _regionQueryModel; }
		shared_ptr<RegionProxyModel> regionProxyModel() const { return _regionProxyModel; }
		void initRegionQueryModel();

	protected:
		long long getUsableIdFromPathPrefix(const std::string &pathPrefix, AdminPageWidget *adminPageWidget, const std::string &pathComponentPrefix);
		void handleAuthChanged();
		void handleInternalPathChanged(std::string path);

		void lazyLoadVisitorWidgets();
		void lazyLoadAdminWidgets();
		void lazyLoadLoginWidget();
		void lazyLoadDeniedPermissionWidget();

		Wt::WDialog *createPasswordPromptDialog();
		void handlePasswordPromptFinished(Wt::DialogCode code, const std::string &rejectInternalPath);

		template<class PageClass>
		PageClass *addMenuItem(Wt::WMenu *menu, const Wt::WString &title, unique_ptr<PageClass> pageContents)
		{
			PageClass *ret = pageContents.get();
			auto menuItem = make_unique<Wt::WMenuItem>(title, move(pageContents));
			menuItem->setPathComponent(ret->basePathComponent());
			menu->addItem(move(menuItem));
			return ret;
		}

		//Widgets
		Wt::WStackedWidget *_mainStack = nullptr;

		Wt::WTemplate *_mainVisitorTemplate = nullptr;
		Wt::WStackedWidget *_visitorStack = nullptr;
		Wt::WNavigationBar *_visitorNavBar = nullptr;
		Wt::WMenu *_visitorMenu = nullptr;

		Wt::WTemplate *_mainAdminTemplate = nullptr;
		Wt::WStackedWidget *_adminStack = nullptr;
		Wt::WNavigationBar *_adminNavBar = nullptr;
		Wt::WMenu *_adminMenu = nullptr;

		AuthWidget *_authWidget = nullptr;
		Wt::WDialog *_passwordPromptDialog = nullptr;
		Wt::WTemplate *_deniedPermissionWidget = nullptr;

		DashboardAdminPage *_dashboardAdminPage = nullptr;
		EntitiesAdminPage *_entitiesAdminPage = nullptr;
		AccountsAdminPage *_accountsAdminPage = nullptr;
		AttendanceAdminPage *_attendanceAdminPage = nullptr;
		UsersAdminPage *_usersAdminPage = nullptr;

		unique_ptr<Wt::WDialog> _errorDialog;
		Wt::WText *_errorDialogText = nullptr;

		Wt::WSuggestionPopup *_findEntitySuggestion = nullptr;
		Wt::WSuggestionPopup *_findAccountSuggestion = nullptr;
		Wt::WSuggestionPopup *_findLocationSuggestion = nullptr;

		//Models
		shared_ptr<CountryQueryModel> _countryQueryModel;
		shared_ptr<CountryProxyModel> _countryProxyModel;
		shared_ptr<CityQueryModel> _cityQueryModel;
		shared_ptr<PositionQueryModel> _positionQueryModel;
		shared_ptr<PositionProxyModel> _positionProxyModel;
		shared_ptr<ServiceQueryModel> _serviceQueryModel;
		shared_ptr<ServiceProxyModel> _serviceProxyModel;
		shared_ptr<RegionQueryModel> _regionQueryModel;
		shared_ptr<RegionProxyModel> _regionProxyModel;
		
		//Session related
		Dbo::Session _dboSession;
		AuthLogin _login;

		//Some databases
		UserDatabase _userDatabase;
		EntitiesDatabase _entitiesDatabase;
		AccountsDatabase _accountsDatabase;

		system_clock::time_point _startTime;
	};

	//TEMPLATE FUNCTIONS
	template<class QueryResult>
	void AuthLogin::setPermissionConditionsToQuery(Dbo::Query<QueryResult> &query, bool resultsIfLoggedOut, const std::string &fieldPrefix, bool modifyPermissionRequired)
	{
		if(modifyPermissionRequired && !hasPermission(Permissions::ModifyRecord))
		{
			query.where("false");
			return;
		}

		Dbo::ptr<User> selfUser = userPtr();
		if(selfUser)
		{
			IdOptions regionIdOptions = IdOptions::Default;
			IdOptions userIdOptions = IdOptions::Default;

			//Region condition
			if(hasPermission(Permissions::ViewOtherRegionRecord))
			{
				if(!hasPermission(Permissions::ViewUnassignedRegionRecord))
					regionIdOptions = IdOptions::NotNull;
			}
			else
			{
				if(!selfUser->regionPtr)
					regionIdOptions = IdOptions::IsNull;
				else
				{
					if(hasPermission(Permissions::ViewUnassignedRegionRecord))
						regionIdOptions = IdOptions::SelfIdOrNull;
					else
						regionIdOptions = IdOptions::SelfId;
				}
			}

			//User view condition
			if(hasPermission(Permissions::ViewOtherUserRecord))
			{
				if(!hasPermission(Permissions::ViewUnassignedUserRecord))
					userIdOptions = IdOptions::NotNull;
			}
			else
			{
				if(hasPermission(Permissions::ViewUnassignedUserRecord))
					userIdOptions = IdOptions::SelfIdOrNull;
				else
					userIdOptions = IdOptions::SelfId;
			}

			if(modifyPermissionRequired)
			{
				//Region modify condition
				if(!hasPermission(Permissions::ModifyOtherRegionRecord))
				{
					if(regionIdOptions != IdOptions::SelfIdOrNull) //If regionIdOptions == SelfIdOrNull for view permission then user can also modify unassigned records
						regionIdOptions = IdOptions::SelfId;
				}

				//User modify condition
				if(!hasPermission(Permissions::ModifyOtherUserRecord))
				{
					if(userIdOptions != IdOptions::SelfIdOrNull) //If regionIdOptions == SelfIdOrNull for view permission then user can also modify unassigned records
						userIdOptions = IdOptions::SelfId;
				}
			}

			//Add conditions
			if(regionIdOptions == IdOptions::NotNull)
				query.where(fieldPrefix + "region_id IS NOT null");
			else if(regionIdOptions == IdOptions::IsNull)
				query.where(fieldPrefix + "region_id IS null");
			else if(regionIdOptions == IdOptions::SelfId)
				query.where(fieldPrefix + "region_id = ?").bind(selfUser->regionPtr.id());
			else if(regionIdOptions == IdOptions::SelfIdOrNull)
				query.where(fieldPrefix + "region_id = ? OR " + fieldPrefix + "region_id IS null").bind(selfUser->regionPtr.id());

			if(userIdOptions == IdOptions::NotNull)
				query.where(fieldPrefix + "creator_user_id IS NOT null");
// 			else if(userIdOptions == IdOptions::IsNull)
// 				query.where(fieldPrefix + "creator_user_id IS null");
			else if(userIdOptions == IdOptions::SelfId)
				query.where(fieldPrefix + "creator_user_id = ?").bind(selfUser.id());
			else if(userIdOptions == IdOptions::SelfIdOrNull)
				query.where(fieldPrefix + "creator_user_id = ? OR " + fieldPrefix + "creator_user_id IS null").bind(selfUser.id());
		}
		else if(!resultsIfLoggedOut)
			query.where("false");
	}
}

#define APP WApplication::instance()

#endif