#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Dbo/Configuration.h"
#include "Dbo/PermissionsDatabase.h"

#include "Widgets/FindRecordEdit.h"
#include "Widgets/AdminPages.h"
#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
#include "Widgets/AccountMVC.h"
#include "Widgets/LocationMVC.h"
#include "Widgets/EntryCycleMVC.h"
#include "Widgets/HRMVC.h"
#include "Widgets/AuthWidget.h"

#include <Wt/WNavigationBar>
#include <Wt/WMenu>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WBootstrapTheme>
#include <Wt/WPushButton>
#include <Wt/WLoadingIndicator>
#include <Wt/WSuggestionPopup>
#include <web/WebUtils.h>

#include <Wt/Dbo/QueryModel>

namespace GS
{

AuthLogin::AuthLogin()
{
	resetPermissions();
	beforeChanged_.connect(this, &AuthLogin::resetPermissions);
}

Wt::Dbo::ptr<User> AuthLogin::userPtr()
{
	if(loggedIn())
		return userPtr(user());
	else
		return Wt::Dbo::ptr<User>();
}

Wt::Dbo::ptr<User> AuthLogin::userPtr(const Wt::Auth::User &authUser)
{
	WApplication *app = APP;
	TRANSACTION(app);

	Wt::Dbo::ptr<AuthInfo> authInfo = app->userDatabase().find(authUser);
	Wt::Dbo::ptr<User> user = authInfo->user();

	if(!user)
	{
		user = app->dboSession().add(new User());
		authInfo.modify()->setUser(user);
	}
	return user;
}

void AuthLogin::resetPermissions()
{
	WApplication *app = APP;
	_permissions = SERVER->permissionsDatabase()->getUserPermissions(userPtr(), state(), &app->dboSession());

	//Preloaded permissions
	_recordCreatePermission = checkPermission(Permissions::CreateRecord);
}

AuthLogin::PermissionResult AuthLogin::checkPermission(long long permissionId)
{
	auto fitr = _permissions.find(permissionId);
	if(fitr == _permissions.end())
		return Denied;

	if(fitr->second->requireStrongLogin && state() != Wt::Auth::StrongLogin)
		return RequiresStrongLogin;

	return Permitted;
}

AuthLogin::PermissionResult AuthLogin::checkRecordViewPermission(const BaseAdminRecord *record)
{
	PermissionResult result = checkRecordViewPermission((void*)record);
	if(result != Permitted)
		return result;

	TRANSACTION(APP);
	Wt::Dbo::ptr<User> userSelf = userPtr();
	if(!userSelf)
		return AuthLogin::Denied;

	if(!record)
		return result;

	//Region
	if(!record->regionPtr)
	{
		PermissionResult viewUnassignedRegionRecord = checkPermission(Permissions::ViewUnassignedRegionRecord);
		if(viewUnassignedRegionRecord == Denied)
			return Denied;
		if(viewUnassignedRegionRecord == RequiresStrongLogin)
			result = RequiresStrongLogin;
	}
	else if(userSelf->regionPtr != record->regionPtr)
	{
		PermissionResult viewOtherRegionRecord = checkPermission(Permissions::ViewOtherRegionRecord);
		if(viewOtherRegionRecord == Denied)
			return Denied;
		if(viewOtherRegionRecord == RequiresStrongLogin)
			result = RequiresStrongLogin;
	}

	//User
	if(!record->creatorUserPtr)
	{
		PermissionResult viewUnassignedUserRecord = checkPermission(Permissions::ViewUnassignedUserRecord);
		if(viewUnassignedUserRecord == Denied)
			return Denied;
		if(viewUnassignedUserRecord == RequiresStrongLogin)
			result = RequiresStrongLogin;
	}
	else if(userSelf != record->creatorUserPtr)
	{
		PermissionResult viewOtherUserRecord = checkPermission(Permissions::ViewOtherUserRecord);
		if(viewOtherUserRecord == Denied)
			return Denied;
		if(viewOtherUserRecord == RequiresStrongLogin)
			result = RequiresStrongLogin;
	}
	return result;
}

AuthLogin::PermissionResult AuthLogin::checkRecordViewPermission(const void *record)
{
	return Permitted;
}

AuthLogin::PermissionResult AuthLogin::checkRecordModifyPermission(const BaseAdminRecord *record)
{
	PermissionResult result = checkRecordModifyPermission((void*)record);
	if(result != Permitted)
		return result;

	TRANSACTION(APP);
	Wt::Dbo::ptr<User> userSelf = userPtr();
	if(!userSelf)
		return Denied;

	if(!record)
		return result;

	if(userSelf->regionPtr != record->regionPtr)
	{
		PermissionResult permission = checkPermission(Permissions::ViewOtherRegionRecord);
		if(permission == Denied)
			return Denied;
		if(permission == RequiresStrongLogin)
			result = RequiresStrongLogin;

		permission = checkPermission(Permissions::ModifyOtherRegionRecord);
		if(permission == Denied)
			return Denied;
		if(permission == RequiresStrongLogin)
			result = RequiresStrongLogin;
	}

	if(userSelf != record->creatorUserPtr)
	{
		PermissionResult permission = checkPermission(Permissions::ViewOtherUserRecord);
		if(permission == Denied)
			return Denied;
		if(permission == RequiresStrongLogin)
			result = RequiresStrongLogin;

		permission = checkPermission(Permissions::ModifyOtherUserRecord);
		if(permission == Denied)
			return Denied;
		if(permission == RequiresStrongLogin)
			result = RequiresStrongLogin;
	}
	return result;
}

AuthLogin::PermissionResult AuthLogin::checkRecordModifyPermission(const void *record)
{
	return checkPermission(Permissions::ModifyRecord);
}

WApplication::WApplication(const Wt::WEnvironment& env)
	: Wt::WApplication(env),
	_startTime(boost::posix_time::microsec_clock::local_time()), _entitiesDatabase(_dboSession), _accountsDatabase(_dboSession), _userDatabase(_dboSession), _login()
{
	WServer *server = SERVER;

	_dboSession.setConnectionPool(*server->sqlPool());
	mapDboTree(_dboSession);
	WW::Dbo::mapConfigurationDboTree(_dboSession);

	setTitle(Wt::WString::tr("GS.Title"));
	styleSheet().addRule("div.Wt-loading",
		"position:absolute;top:0px;right:0;z-index:9999999;background:#457044;padding:10px 15px;color:#fff;border-radius:0 0 0 3px;"
		"-webkit-box-shadow: -1px 1px 2px 0px #000;"
		"-moz-box-shadow: -1px 1px 2px 0px #000;"
		"box-shadow: -1px 1px 2px 0px #000;");

	messageResourceBundle().use(appRoot() + "templates", false); //CHECK_BEFORE_RELEASE
	messageResourceBundle().use(appRoot() + "strings", false); //CHECK_BEFORE_RELEASE

	Wt::WLocale newLocale("en");
	newLocale.setDateFormat(Wt::WString::tr("DateFormat"));
	newLocale.setTimeFormat(Wt::WString::tr("TimeFormat"));
	newLocale.setDateTimeFormat(Wt::WString::tr("DateTimeFormat"));
	newLocale.setDecimalPoint(Wt::WString::tr("DecimalPoint").toUTF8());
	newLocale.setGroupSeparator(Wt::WString::tr("GroupSeparator").toUTF8());
	setLocale(newLocale);

	useStyleSheet(Wt::WLink("style.css"));
	useStyleSheet(Wt::WLink(resourcesUrl() + "font-awesome/css/font-awesome.min.css"));

	Wt::WBootstrapTheme *theme = new Wt::WBootstrapTheme();
	theme->setVersion(Wt::WBootstrapTheme::Version3);
	setTheme(theme);

	//Error Dialog
	_errorDialog = new Wt::WDialog(Wt::WString::tr("AnErrorOccurred"), this);
	_errorDialog->setTransient(true);
	_errorDialog->rejectWhenEscapePressed(true);
	_errorDialog->setClosable(true);
	_errorDialogText = new Wt::WText(_errorDialog->contents());
	Wt::WBreak *hr = new Wt::WBreak(_errorDialog->contents());
	hr->setHtmlTagName("hr");
	auto errorOkBtn = new Wt::WPushButton(Wt::WString::tr("Ok"), _errorDialog->contents());
	errorOkBtn->clicked().connect(_errorDialog, &Wt::WDialog::accept);

	//Main Widgets
	_mainStack = new Wt::WStackedWidget(root());
	lazyLoadLoginWidget();

	setInternalPathDefaultValid(true);
	handleInternalPathChanged(internalPath());
	internalPathChanged().connect(this, &WApplication::handleInternalPathChanged);
	authLogin().changed().connect(this, &WApplication::handleAuthChanged);

	//Enable server push
	enableUpdates();
}

WApplication::~WApplication()
{

}

void WApplication::handleAuthChanged()
{
	if(!authLogin().canAccessAdminPanel())
	{
		delete _mainAdminTemplate;
		_mainAdminTemplate = nullptr;
	}

	handleInternalPathChanged(internalPath());
}

long long WApplication::getUsableIdFromPathPrefix(const std::string &pathPrefix, AdminPageWidget *adminPageWidget, const std::string &pathComponentPrefix)
{
	std::string path = internalPath();
	Wt::Utils::append(path, '/');

	if(path.find(pathPrefix.c_str(), 0, pathPrefix.size()) != std::string::npos)
	{
		std::string idStr = path.substr(pathPrefix.size(), path.find('/', pathPrefix.size()) - 1);
		if(!adminPageWidget->checkPathComponentExist(pathComponentPrefix + idStr))
		{
			try
			{
				long long id = boost::lexical_cast<long long>(idStr);
				return id;
			}
			catch(boost::bad_lexical_cast &) {}
		}
	}

	return -1;
}

void WApplication::handleInternalPathChanged(std::string path)
{
	Wt::Utils::append(path, '/');
	std::string firstComponent = internalPathNextPart("/");
	if(firstComponent != ADMIN_PATHC)
	{
		lazyLoadVisitorWidgets();
		_mainStack->setCurrentWidget(_mainVisitorTemplate);
	}
	else
	{
		AuthLogin::PermissionResult adminPanelPermission = authLogin().checkPermission(Permissions::AccessAdminPanel);
		if(!authLogin().loggedIn())
		{
			_mainStack->setCurrentWidget(_authWidget);
		}
		else if(adminPanelPermission == AuthLogin::RequiresStrongLogin)
		{
			lazyLoadDeniedPermissionWidget();
			_mainStack->setCurrentWidget(_deniedPermissionWidget);

			Wt::WDialog *dialog = createPasswordPromptDialog();
			if(dialog) 
				dialog->finished().connect(boost::bind(&WApplication::handlePasswordPromptFinished, this, _1, ""));
		}
		else if(adminPanelPermission == AuthLogin::Denied)
		{
			lazyLoadDeniedPermissionWidget();
			_mainStack->setCurrentWidget(_deniedPermissionWidget);
		}
		else
		{
			lazyLoadAdminWidgets();
			_mainStack->setCurrentWidget(_mainAdminTemplate);

			WApplication *app = APP;
			try
			{
				long long id;

				//entity view
				id = getUsableIdFromPathPrefix(Entity::viewInternalPath(""), _entitiesAdminPage, ENTITY_PREFIX);
				if(id != -1)
				{
					Wt::Dbo::Transaction t(dboSession());
					Wt::Dbo::ptr<Entity> ptr = dboSession().load<Entity>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							EntityView *view = new EntityView(ptr);
							view->load();
							auto menuItem = _entitiesAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
						else
							_entitiesAdminPage->setDeniedPermissionWidget();
					}
				}

				//employee assignemnt view
				id = getUsableIdFromPathPrefix(EmployeeAssignment::viewInternalPath(""), _entitiesAdminPage, EMPLOYEES_PATHC "/" EMPLOYEEASSIGNMENTS_PREFIX);
				if(id != -1)
				{
					Wt::Dbo::Transaction t(dboSession());
					Wt::Dbo::ptr<EmployeeAssignment> ptr = dboSession().load<EmployeeAssignment>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							EmployeeExpenseView *view = new EmployeeExpenseView(ptr);
							view->load();
							auto menuItem = _entitiesAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
						else
							_entitiesAdminPage->setDeniedPermissionWidget();
					}
				}

				//account view
				id = getUsableIdFromPathPrefix(Account::viewInternalPath(""), _accountsAdminPage, ACCOUNT_PREFIX);
				if(id != -1)
				{
					Wt::Dbo::Transaction t(dboSession());
					Wt::Dbo::ptr<Account> ptr = dboSession().load<Account>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							AccountView *view = new AccountView(ptr);
							view->load();
							auto menuItem = _accountsAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
						else
							_accountsAdminPage->setDeniedPermissionWidget();
					}
				}

				//account entry view
				id = getUsableIdFromPathPrefix(AccountEntry::viewInternalPath(""), _accountsAdminPage, ACCOUNTENTRY_PREFIX);
				if(id != -1)
				{
					Wt::Dbo::Transaction t(dboSession());
					Wt::Dbo::ptr<AccountEntry> ptr = dboSession().load<AccountEntry>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							AccountEntryView *view = new AccountEntryView(ptr);
							view->load();
							auto menuItem = _accountsAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
						else
							_accountsAdminPage->setDeniedPermissionWidget();
					}
				}

				//income cycle view
				id = getUsableIdFromPathPrefix(IncomeCycle::viewInternalPath(""), _accountsAdminPage, INCOMECYCLES_PATHC "/" INCOMECYCLE_PREFIX);
				if(id != -1)
				{
					Wt::Dbo::Transaction t(dboSession());
					Wt::Dbo::ptr<IncomeCycle> ptr = dboSession().load<IncomeCycle>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							IncomeCycleView *view = new IncomeCycleView(ptr);
							view->load();
							auto menuItem = _accountsAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
						else
							_accountsAdminPage->setDeniedPermissionWidget();
					}
				}

				//expense cycle view
				id = getUsableIdFromPathPrefix(ExpenseCycle::viewInternalPath(""), _accountsAdminPage, EXPENSECYCLES_PATHC "/" EXPENSECYCLE_PREFIX);
				if(id != -1)
				{
					Wt::Dbo::Transaction t(dboSession());
					Wt::Dbo::ptr<ExpenseCycle> ptr = dboSession().load<ExpenseCycle>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							EmployeeExpenseView *view = new EmployeeExpenseView(ptr);
							view->load();
							auto menuItem = _accountsAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
						else
							_accountsAdminPage->setDeniedPermissionWidget();
					}
				}
			}
			catch(Wt::Dbo::Exception &e)
			{
				Wt::log("error") << "WApplication::handleInternalPathChanged(): Dbo error(" << e.code() << "): " << e.what();
				showDbBackendError(e.code());
			}
		}
	}
}

void WApplication::lazyLoadLoginWidget()
{
	if(_authWidget)
		return;

	_authWidget = new AuthWidget(_mainStack);
}

void WApplication::lazyLoadDeniedPermissionWidget()
{
	if(_deniedPermissionWidget)
		return;
		
	_deniedPermissionWidget = new Wt::WTemplate(Wt::WString::tr("GS.DeniedPermission"), _mainStack);
}

void WApplication::lazyLoadVisitorWidgets()
{
	if(_mainVisitorTemplate)
		return;

	_mainVisitorTemplate = new Wt::WTemplate(Wt::WString::tr("GS.Main"), _mainStack);

	_visitorNavBar = new Wt::WNavigationBar();
	_visitorNavBar->setTemplateText(Wt::WString::tr("GS.NavigationBar"));
	_visitorNavBar->bindString("container-class", "container");
	_visitorNavBar->setResponsive(true);
	_visitorNavBar->addMenu(_visitorMenu = new Wt::WMenu(_visitorStack = new Wt::WStackedWidget()));
	_visitorMenu->setInternalPathEnabled("/");

	//Logo
	Wt::WImage *logoImage = new Wt::WImage(Wt::WLink("images/logo.png"), Wt::WString::tr("GS.Logo.Alt"));
	_visitorNavBar->setTitle(logoImage, Wt::WLink(Wt::WLink::InternalPath, "/"));

	//Page widgets
	auto homeMenuItem = new Wt::WMenuItem(Wt::WString::tr("Home"), new Wt::WText(Wt::WString::tr("Home")));
	homeMenuItem->setPathComponent("");
	_visitorMenu->addItem(homeMenuItem);
	auto aboutMenuItem = new Wt::WMenuItem(Wt::WString::tr("About"), new Wt::WText(Wt::WString::tr("About")));
	aboutMenuItem->setPathComponent("about");
	_visitorMenu->addItem(aboutMenuItem);
	auto contactMenuItem = new Wt::WMenuItem(Wt::WString::tr("Contact"), new Wt::WText(Wt::WString::tr("Contact")));
	contactMenuItem->setPathComponent("contact");
	_visitorMenu->addItem(contactMenuItem);

	_mainVisitorTemplate->bindWidget("content", _visitorStack);
	_mainVisitorTemplate->bindWidget("navigation", _visitorNavBar);
}

void WApplication::lazyLoadAdminWidgets()
{
	if(_mainAdminTemplate)
		return;

	_mainAdminTemplate = new Wt::WTemplate(Wt::WString::tr("GS.Main"), _mainStack);

	_adminNavBar = new Wt::WNavigationBar();
	_adminNavBar->setTemplateText(Wt::WString::tr("GS.NavigationBar"));
	_adminNavBar->addStyleClass("navbar-admin");
	_adminNavBar->bindString("container-class", "container-fluid");
	_adminNavBar->setResponsive(true);
	_adminNavBar->addMenu(_adminMenu = new Wt::WMenu(_adminStack = new Wt::WStackedWidget()));
	_adminMenu->setInternalPathEnabled("/" ADMIN_PATHC "/");

	auto logoutBtn = new Wt::WPushButton(Wt::WString::tr("LogoutButton"), Wt::XHTMLText);
	logoutBtn->clicked().connect(&authLogin(), &Wt::Auth::Login::logout);
	_adminNavBar->addWidget(logoutBtn, Wt::AlignRight);

	Wt::WImage *logoImage = new Wt::WImage(Wt::WLink("images/logo.png"), Wt::WString::tr("GS.Logo.Alt"));
	_adminNavBar->setTitle(logoImage, Wt::WLink(Wt::WLink::InternalPath, "/"));

	//Admin page widgets
	//Dashboard
	auto dashbaordAdminPage = new DashboardAdminPage();
	auto dashboardMenuItem = new Wt::WMenuItem(Wt::WString::tr("Dashboard"), dashbaordAdminPage);
	dashboardMenuItem->setPathComponent(dashbaordAdminPage->basePathComponent());
	_adminMenu->addItem(dashboardMenuItem);

	//Entites
	auto entitiesMenuItem = new Wt::WMenuItem(Wt::WString::tr("Entities"), _entitiesAdminPage = new EntitiesAdminPage());
	entitiesMenuItem->setPathComponent(_entitiesAdminPage->basePathComponent());
	_adminMenu->addItem(entitiesMenuItem);

	//Accounts
	auto accountsMenuItem = new Wt::WMenuItem(Wt::WString::tr("FinancialRecords"), _accountsAdminPage = new AccountsAdminPage());
	accountsMenuItem->setPathComponent(_accountsAdminPage->basePathComponent());
	_adminMenu->addItem(accountsMenuItem);

	//System
	auto systemAdminPage = new SystemAdminPage();
	auto systemMenuItem = new Wt::WMenuItem(Wt::WString::tr("System"), systemAdminPage);
	systemMenuItem->setPathComponent(systemAdminPage->basePathComponent());
	_adminMenu->addItem(systemMenuItem);

	_mainAdminTemplate->bindWidget("content", _adminStack);
	_mainAdminTemplate->bindWidget("navigation", _adminNavBar);
}

void WApplication::initFindEntitySuggestion()
{
	if(!_findEntitySuggestion)
		_findEntitySuggestion = new FindEntitySuggestionPopup(this);
}

void WApplication::initFindAccountSuggestion()
{
	if(!_findAccountSuggestion)
		_findAccountSuggestion = new FindAccountSuggestionPopup(this);
}

void WApplication::initFindLocationSuggestion()
{
	if(!_findLocationSuggestion)
		_findLocationSuggestion = new FindLocationSuggestionPopup(this);
}

// void WApplication::initFindPersonSuggestion()
// {
// 	if(!_findPersonSuggestion)
// 		_findPersonSuggestion = new FindEntitySuggestionPopup(Entity::PersonType, this);
// }
// 
// void WApplication::initFindBusinessSuggestion()
// {
// 	if(!_findBusinessSuggestion)
// 		_findBusinessSuggestion = new FindEntitySuggestionPopup(Entity::BusinessType, this);
// }

// void WApplication::initFindPersonModel()
// {
// 	if(_findPersonFilterModel)
// 		return;
// 
// 	_findPersonFilterModel = new FindEntityModel(this);
// 	_findPersonFilterModel->setQuery(session().query<_FEMTuple>("SELECT name, id FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::PersonType));
// 	_findPersonFilterModel->addColumn("name");
// 	_findPersonFilterModel->addColumn("id");
// }
// 
// void WApplication::initFindBusinessModel()
// {
// 	if(_findBusinessFilterModel)
// 		return;
// 
// 	_findBusinessFilterModel = new FindEntityModel(this);
// 	_findBusinessFilterModel->setQuery(session().query<_FEMTuple>("SELECT name, id FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::BusinessType));
// 	_findBusinessFilterModel->addColumn("name");
// 	_findBusinessFilterModel->addColumn("id");
// }

// void WApplication::initLocationQueryModel()
// {
// 	if(_locationQueryModel)
// 		return;
// 
// 	_locationQueryModel = new LocationQueryModel(this);
// 	_locationQueryModel->setQuery(session().find<Location>());
// 	_locationQueryModel->addColumn("address");
// 	_locationQueryModel->addColumn("country_code");
// 	_locationQueryModel->addColumn("city_id");
// 	_locationQueryModel->addColumn("id");
// 
// 	_locationFilterModel = new LocationFilterModel(_locationQueryModel);
// 	_locationFilterModel->setSourceModel(_locationQueryModel);
// }

void WApplication::initCountryQueryModel()
{
	if(_countryQueryModel)
		return;

	_countryQueryModel = new CountryQueryModel(this);
	_countryQueryModel->setBatchSize(1000);
	_countryQueryModel->setQuery(dboSession().find<Country>().orderBy("CASE WHEN code = 'PK' THEN 1 ELSE 2 END ASC"));
	_countryQueryModel->addColumn("name");
	_countryQueryModel->addColumn("code");

	_countryProxyModel = new CountryProxyModel(_countryQueryModel, _countryQueryModel);
}

void WApplication::initCityQueryModel()
{
	if(_cityQueryModel)
		return;

	_cityQueryModel = new CityQueryModel(this);
	_cityQueryModel->setBatchSize(10000);
	_cityQueryModel->setQuery(dboSession().find<City>());
	_cityQueryModel->addColumn("name");
	_cityQueryModel->addColumn("id");
	_cityQueryModel->addColumn("country_code");
}

void WApplication::initPositionQueryModel()
{
	if(_positionQueryModel)
		return;

	_positionQueryModel = new PositionQueryModel(this);
	_positionQueryModel->setBatchSize(1000);
	_positionQueryModel->setQuery(dboSession().find<EmployeePosition>());
	_positionQueryModel->addColumn("title");
	_positionQueryModel->addColumn("id");

	_positionProxyModel = new PositionProxyModel(_positionQueryModel, _positionQueryModel);
}

void WApplication::initServiceQueryModel()
{
	if(_serviceQueryModel)
		return;

	_serviceQueryModel = new ServiceQueryModel(this);
	_serviceQueryModel->setBatchSize(1000);
	_serviceQueryModel->setQuery(dboSession().find<ClientService>());
	_serviceQueryModel->addColumn("title");
	_serviceQueryModel->addColumn("id");

	_serviceProxyModel = new ServiceProxyModel(_serviceQueryModel, _serviceQueryModel);
}

void WApplication::showErrorDialog(const Wt::WString &message)
{
	_errorDialogText->setText(message);
	_errorDialog->show();
}

void WApplication::showDbBackendError(const std::string &)
{
	showErrorDialog(Wt::WString::tr("DbInternalError"));
}

Wt::WDialog * WApplication::createPasswordPromptDialog()
{
	if(_passwordPromptDialog)
		return nullptr;

	lazyLoadLoginWidget();
	_passwordPromptDialog = _authWidget->createPasswordPromptDialog(authLogin());
	_passwordPromptDialog->rejectWhenEscapePressed(true);
	_passwordPromptDialog->show();
	return _passwordPromptDialog;
}

void WApplication::handlePasswordPromptFinished(Wt::WDialog::DialogCode code, const std::string &rejectInternalPath)
{
	if(code == Wt::WDialog::Rejected)
	{
		lazyLoadDeniedPermissionWidget();
		_mainStack->setCurrentWidget(_deniedPermissionWidget);
		if(!rejectInternalPath.empty())
			setInternalPath(rejectInternalPath, true);
	}	

	delete _passwordPromptDialog;
	_passwordPromptDialog = nullptr;
}

}
