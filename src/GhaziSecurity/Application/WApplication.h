#ifndef GS_WAPPLICATION_H
#define GS_WAPPLICATION_H

#include <Wt/WApplication>
#include <Wt/Auth/Login>
#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/WDialog>

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
	class AuthWidget;
	class AdminPageWidget;

	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<Location>> LocationQueryModel;
	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<Country>> CountryQueryModel;
	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<City>> CityQueryModel;
	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<EmployeePosition>> PositionQueryModel;
	typedef Wt::Dbo::QueryModel<Wt::Dbo::ptr<ClientService>> ServiceQueryModel;

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
		Wt::Dbo::ptr<User> userPtr();
		Wt::Dbo::ptr<User> userPtr(const Wt::Auth::User &authUser);

		bool hasPermission(long long permissionId) { return checkPermission(permissionId) == Permitted; }
		PermissionResult checkPermission(long long permissionId);
		PermissionResult checkRecordViewPermission(const BaseAdminRecord *record);
		PermissionResult checkRecordViewPermission(const void *record);
		PermissionResult checkRecordModifyPermission(const BaseAdminRecord *record);
		PermissionResult checkRecordModifyPermission(const void *record);
		PermissionResult checkRecordCreatePermission() { return _recordCreatePermission; }

		bool canAccessAdminPanel() { return loggedIn() && hasPermission(Permissions::AccessAdminPanel); }

		template<class QueryResult>
		void setPermissionConditionsToQuery(Wt::Dbo::Query<QueryResult> &query, bool resultsIfLoggedOut = false, const std::string &fieldPrefix = "", bool modifyPermissionRequired = false);

	protected:
		enum class IdOptions
		{
			Default, //Can modify other
			IsNull, //CANT modify other
			NotNull, //Can modify other
			SelfId, //CANT modify other
			SelfIdOrNull //CANT modify other
		};

		void resetPermissions();
		//void resetPermissionConditions();
		PermissionMap _permissions;
		PermissionResult _recordCreatePermission;
	};

	class WApplication : public Wt::WApplication
	{
	public:
		WApplication(const Wt::WEnvironment& env);
		virtual ~WApplication() override;

		static WApplication *instance() { return dynamic_cast<WApplication*>(Wt::WApplication::instance()); }
		static WApplication *createApplication(const Wt::WEnvironment &env) { return new WApplication(env); }

		Wt::Dbo::Session &dboSession() { return _dboSession; }
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
		Wt::WDialog *errorDialog() const { return _errorDialog; }
		void showErrorDialog(const Wt::WString &message);
		void showStaleObjectError(const Wt::WString &recordName) { showErrorDialog(Wt::WString::tr("StaleObjectError").arg(recordName)); }
		void showDbBackendError(const std::string &code);

		//SuggestionPopups
		Wt::WSuggestionPopup *findEntitySuggestion() const { return _findEntitySuggestion; }
		void initFindEntitySuggestion();
		Wt::WSuggestionPopup *findAccountSuggestion() const { return _findAccountSuggestion; }
		void initFindAccountSuggestion();
		Wt::WSuggestionPopup *findLocationSuggestion() const { return _findLocationSuggestion; }
		void initFindLocationSuggestion();
// 		Wt::WSuggestionPopup *findPersonSuggestion() const { return _findPersonSuggestion; }
// 		void initFindPersonSuggestion();
// 		Wt::WSuggestionPopup *findBusinessSuggestion() const { return _findBusinessSuggestion; }
// 		void initFindBusinessSuggestion();

		//QueryModels
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

	protected:
		long long getUsableIdFromPathPrefix(const std::string &pathPrefix, AdminPageWidget *adminPageWidget, const std::string &pathComponentPrefix);
		void handleAuthChanged();
		void handleInternalPathChanged(std::string path);

		void lazyLoadVisitorWidgets();
		void lazyLoadAdminWidgets();
		void lazyLoadLoginWidget();
		void lazyLoadDeniedPermissionWidget();

		Wt::WDialog *createPasswordPromptDialog();
		void handlePasswordPromptFinished(Wt::WDialog::DialogCode code, const std::string &rejectInternalPath);

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

		EntitiesAdminPage *_entitiesAdminPage = nullptr;
		AccountsAdminPage *_accountsAdminPage = nullptr;

		Wt::WDialog *_errorDialog = nullptr;
		Wt::WText *_errorDialogText = nullptr;

		Wt::WSuggestionPopup *_findEntitySuggestion = nullptr;
		Wt::WSuggestionPopup *_findAccountSuggestion = nullptr;
		Wt::WSuggestionPopup *_findLocationSuggestion = nullptr;
// 		Wt::WSuggestionPopup *_findPersonSuggestion = nullptr;
// 		Wt::WSuggestionPopup *_findBusinessSuggestion = nullptr;

		//Models
		CountryQueryModel *_countryQueryModel = nullptr;
		CountryProxyModel *_countryProxyModel = nullptr;
		CityQueryModel *_cityQueryModel = nullptr;
		PositionQueryModel *_positionQueryModel = nullptr;
		PositionProxyModel *_positionProxyModel = nullptr;
		ServiceQueryModel *_serviceQueryModel = nullptr;
		ServiceProxyModel *_serviceProxyModel = nullptr;

		//Session related
		Wt::Dbo::Session _dboSession;
		AuthLogin _login;

		//Some databases
		UserDatabase _userDatabase;
		EntitiesDatabase _entitiesDatabase;
		AccountsDatabase _accountsDatabase;

		boost::posix_time::ptime _startTime;
	};

	//TEMPLATE FUNCTIONS
	template<class QueryResult>
	void AuthLogin::setPermissionConditionsToQuery(Wt::Dbo::Query<QueryResult> &query, bool resultsIfLoggedOut /*= false*/, const std::string &fieldPrefix /*= ""*/, bool modifyPermissionRequired /*= false*/)
	{
		if(modifyPermissionRequired && !hasPermission(Permissions::ModifyRecord))
		{
			query.where("false");
			return;
		}

		Wt::Dbo::ptr<User> selfUser = userPtr();
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