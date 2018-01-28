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
#include "Widgets/AttendanceMVC.h"
#include "Widgets/UserMVC.h"
#include "Widgets/AuthWidget.h"

#include <Wt/WNavigationBar.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WLoadingIndicator.h>
#include <web/WebUtils.h>

namespace ERP
{
	AuthLogin::AuthLogin()
	{
		handleBeforeLoginChanged();
		beforeChanged_.connect(this, &AuthLogin::handleBeforeLoginChanged);
	}

	void AuthLogin::handleBeforeLoginChanged()
	{
		WApplication *app = APP;

		if(loggedIn())
			_authInfoPtr = app->userDatabase().find(user());
		else
			_authInfoPtr = nullptr;
		
		//Make sure user entry for authinfo exists
		if(_authInfoPtr && !_authInfoPtr->user())
		{
			TRANSACTION(app);
			_authInfoPtr.modify()->setUser(app->dboSession().addNew<User>());
		}

		_permissions = SERVER->permissionsDatabase().getUserPermissions(userPtr(), state(), &app->dboSession());

		//Preloaded permissions
		_recordCreatePermission = checkPermission(Permissions::CreateRecord);
	}

	AuthLogin::PermissionResult AuthLogin::checkPermission(long long permissionId)
	{
		auto fitr = _permissions.find(permissionId);
		if(fitr == _permissions.end())
			return Denied;

		if(fitr->second->requireStrongLogin && state() != Wt::Auth::LoginState::Strong)
			return RequiresStrongLogin;

		return Permitted;
	}

	AuthLogin::PermissionResult AuthLogin::checkRecordViewPermission(const BaseAdminRecord *record)
	{
		PermissionResult result = checkRecordViewPermission((void*)record);
		if(result != Permitted)
			return result;

		TRANSACTION(APP);
		Dbo::ptr<User> userSelf = userPtr();
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

	AuthLogin::PermissionResult AuthLogin::checkRecordViewPermission(const void *)
	{
		return Permitted;
	}

	AuthLogin::PermissionResult AuthLogin::checkRecordModifyPermission(const BaseAdminRecord *record)
	{
		PermissionResult result = checkRecordModifyPermission((void*)record);
		if(result != Permitted)
			return result;

		TRANSACTION(APP);
		Dbo::ptr<User> userSelf = userPtr();
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

	AuthLogin::PermissionResult AuthLogin::checkRecordModifyPermission(const void *)
	{
		return checkPermission(Permissions::ModifyRecord);
	}

	WApplication::WApplication(const Wt::WEnvironment& env)
		: Wt::WApplication(env),
		_startTime(system_clock::now()), _entitiesDatabase(_dboSession), _accountsDatabase(_dboSession), _userDatabase(_dboSession), _login()
	{
		//Initialize Dbo session
		WServer *server = SERVER;
		_dboSession.setConnectionPool(server->sqlPool());
		mapERPDbos(_dboSession);
		mapConfigurationDbos(_dboSession);

		//Localization and Strings
		messageResourceBundle().use(appRoot() + "templates", false); //CHECK_BEFORE_RELEASE
		messageResourceBundle().use(appRoot() + "strings", false); //CHECK_BEFORE_RELEASE

		Wt::WLocale newLocale("en");
		newLocale.setDateFormat(tr("DateFormat"));
		newLocale.setTimeFormat(tr("TimeFormat"));
		newLocale.setDateTimeFormat(tr("DateTimeFormat"));
		newLocale.setDecimalPoint(tr("DecimalPoint").toUTF8());
		newLocale.setGroupSeparator(tr("GroupSeparator").toUTF8());
		setLocale(newLocale);

		//Styling
		setTitle(tr("ERP.Title"));
		styleSheet().addRule("div.Wt-loading",
			"position:absolute;top:0px;right:0;z-index:9999999;background:#457044;padding:10px 15px;color:#fff;border-radius:0 0 0 3px;"
			"-webkit-box-shadow: -1px 1px 2px 0px #000;"
			"-moz-box-shadow: -1px 1px 2px 0px #000;"
			"box-shadow: -1px 1px 2px 0px #000;");

		useStyleSheet(Wt::WLink("style.css"));
		useStyleSheet(Wt::WLink(resourcesUrl() + "font-awesome/css/font-awesome.min.css"));

		auto theme = make_unique<Wt::WBootstrapTheme>();
		theme->setVersion(Wt::WBootstrapTheme::Version::v3);
		setTheme(move(theme));

		//Error Dialog
		_errorDialog = make_unique<Wt::WDialog>(tr("AnErrorOccurred"));
		_errorDialog->setTransient(true);
		_errorDialog->rejectWhenEscapePressed(true);
		_errorDialog->setClosable(true);
		_errorDialogText = _errorDialog->contents()->addNew<Wt::WText>();
		_errorDialog->contents()->addNew<Wt::WBreak>()->setHtmlTagName("hr");
		auto errorOkBtn = _errorDialog->contents()->addNew<Wt::WPushButton>(tr("Ok"));
		errorOkBtn->clicked().connect(_errorDialog.get(), &Wt::WDialog::accept);

		//Main Widgets
		_mainStack = root()->addNew<Wt::WStackedWidget>();
		lazyLoadLoginWidget();

		//Internal paths and auth
		setInternalPathDefaultValid(true);
		handleInternalPathChanged(internalPath());
		internalPathChanged().connect(this, &WApplication::handleInternalPathChanged);
		authLogin().changed().connect(this, &WApplication::handleAuthChanged);

		//Enable server push
		enableUpdates();
	}

	void WApplication::handleAuthChanged()
	{
		if(!authLogin().canAccessAdminPanel() && _mainAdminTemplate)
		{
			_mainAdminTemplate->removeFromParent();
			_mainAdminTemplate = nullptr;
		}

		handleInternalPathChanged(internalPath());
	}

	long long WApplication::getUsableIdFromPathPrefix(const std::string &pathPrefix, AdminPageWidget *adminPageWidget, const std::string &pathComponentPrefix)
	{
		if(!adminPageWidget)
			return -1;

		std::string path = internalPath();
		Wt::Utils::append(path, '/');

		if(path.find(pathPrefix.c_str(), 0, pathPrefix.size()) != std::string::npos)
		{
			std::string idStr = path.substr(pathPrefix.size(), path.find('/', pathPrefix.size()) - 1);
			if(!adminPageWidget->checkPathComponentExist(pathComponentPrefix + idStr))
			{
				try
				{
					return std::stoll(idStr);
				}
				catch(const std::invalid_argument &) {}
				catch(const std::out_of_range &) {}
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
			setInternalPath("/" ADMIN_PATHC, true);
			return;
		}

		AuthLogin::PermissionResult adminPanelPermission = authLogin().checkPermission(Permissions::AccessAdminPanel);
		if(!authLogin().loggedIn())
		{
			_mainStack->setCurrentWidget(_authWidget);
		}
		else if(adminPanelPermission == AuthLogin::RequiresStrongLogin)
		{
			lazyLoadDeniedPermissionWidget();
			_mainStack->setCurrentWidget(_deniedPermissionWidget);
			createPasswordPromptDialog();
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

			try
			{
				long long id;

				//entity view
				id = getUsableIdFromPathPrefix(Entity::viewInternalPath(""), _entitiesAdminPage, ENTITY_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<Entity> ptr = dboSession().load<Entity>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<EntityView>(ptr);
							view->load();
							_entitiesAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_entitiesAdminPage->setDeniedPermissionWidget();
					}
				}

				//employee assignment view
				id = getUsableIdFromPathPrefix(EmployeeAssignment::viewInternalPath(""), _entitiesAdminPage, EMPLOYEES_PATHC "/" EMPLOYEEASSIGNMENTS_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<EmployeeAssignment> ptr = dboSession().load<EmployeeAssignment>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<EmployeeAssignmentView>(ptr);
							view->load();
							_entitiesAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_entitiesAdminPage->setDeniedPermissionWidget();
					}
				}

				//client assignment view
				id = getUsableIdFromPathPrefix(ClientAssignment::viewInternalPath(""), _entitiesAdminPage, CLIENTS_PATHC "/" CLIENTASSIGNMENTS_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<ClientAssignment> ptr = dboSession().load<ClientAssignment>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<ClientAssignmentView>(ptr);
							view->load();
							_entitiesAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_entitiesAdminPage->setDeniedPermissionWidget();
					}
				}

				//account view
				id = getUsableIdFromPathPrefix(Account::viewInternalPath(""), _accountsAdminPage, ACCOUNT_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<Account> ptr = dboSession().load<Account>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<AccountView>(ptr);
							view->load();
							_accountsAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_accountsAdminPage->setDeniedPermissionWidget();
					}
				}

				//account entry view
				id = getUsableIdFromPathPrefix(AccountEntry::viewInternalPath(""), _accountsAdminPage, ACCOUNTENTRY_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<AccountEntry> ptr = dboSession().load<AccountEntry>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<AccountEntryView>(ptr);
							view->load();
							_accountsAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_accountsAdminPage->setDeniedPermissionWidget();
					}
				}

				//income cycle view
				id = getUsableIdFromPathPrefix(IncomeCycle::viewInternalPath(""), _accountsAdminPage, INCOMECYCLES_PATHC "/" INCOMECYCLE_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<IncomeCycle> ptr = dboSession().load<IncomeCycle>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<IncomeCycleView>(ptr);
							view->load();
							_accountsAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_accountsAdminPage->setDeniedPermissionWidget();
					}
				}

				//expense cycle view
				id = getUsableIdFromPathPrefix(ExpenseCycle::viewInternalPath(""), _accountsAdminPage, EXPENSECYCLES_PATHC "/" EXPENSECYCLE_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<ExpenseCycle> ptr = dboSession().load<ExpenseCycle>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<ExpenseCycleView>(ptr);
							view->load();
							_accountsAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_accountsAdminPage->setDeniedPermissionWidget();
					}
				}

				//attendance entry view
				id = getUsableIdFromPathPrefix(AttendanceEntry::viewInternalPath(""), _attendanceAdminPage, ATTENDANCEENTRY_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<AttendanceEntry> ptr = dboSession().load<AttendanceEntry>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<AttendanceEntryView>(ptr);
							view->load();
							_attendanceAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_attendanceAdminPage->setDeniedPermissionWidget();
					}
				}

				//attendance device view
				id = getUsableIdFromPathPrefix(AttendanceDevice::viewInternalPath(""), _attendanceAdminPage, ATTENDANCEDEVICES_PATHC "/" ATTENDANCEDEVICE_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<AttendanceDevice> ptr = dboSession().load<AttendanceDevice>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted)
						{
							auto view = make_unique<AttendanceDeviceView>(ptr);
							view->load();
							_attendanceAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_attendanceAdminPage->setDeniedPermissionWidget();
					}
				}

				//user view
				id = getUsableIdFromPathPrefix(User::viewInternalPath(""), _usersAdminPage, USER_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<AuthInfo> ptr = dboSession().load<AuthInfo>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted && authLogin().hasPermission(Permissions::ViewUser))
						{
							auto view = make_unique<UserView>(ptr);
							view->load();
							_usersAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_usersAdminPage->setDeniedPermissionWidget();
					}
				}

				//region view
				id = getUsableIdFromPathPrefix(Region::viewInternalPath(""), _usersAdminPage, REGIONS_PATHC "/" REGION_PREFIX);
				if(id != -1)
				{
					TRANSACTION(this);
					Dbo::ptr<Region> ptr = dboSession().load<Region>(id, true);
					if(ptr)
					{
						if(authLogin().checkRecordViewPermission(ptr.get()) == AuthLogin::Permitted && authLogin().hasPermission(Permissions::ViewRegion))
						{
							auto view = make_unique<RegionView>(ptr);
							view->load();
							_usersAdminPage->createMenuItemWrapped(move(view));
						}
						else
							_usersAdminPage->setDeniedPermissionWidget();
					}
				}
			}
			catch(const Dbo::ObjectNotFoundException &) { }
			catch(const Dbo::Exception &e)
			{
				Wt::log("error") << "WApplication::handleInternalPathChanged(): Dbo error(" << e.code() << "): " << e.what();
				showDbBackendError(e.code());
			}
		}
	}

	void WApplication::lazyLoadLoginWidget()
	{
		if(_authWidget)
			return;

		_authWidget = _mainStack->addNew<AuthWidget>();
	}

	void WApplication::lazyLoadDeniedPermissionWidget()
	{
		if(_deniedPermissionWidget)
			return;
		
		_deniedPermissionWidget = _mainStack->addNew<Wt::WTemplate>(tr("ERP.DeniedPermission"));
	}

	void WApplication::lazyLoadVisitorWidgets()
	{
		if(_mainVisitorTemplate)
			return;

		_mainVisitorTemplate = _mainStack->addNew<Wt::WTemplate>(tr("ERP.Main"));

		_visitorStack = _mainVisitorTemplate->bindNew<Wt::WStackedWidget>("content");

		_visitorNavBar = _mainVisitorTemplate->bindNew<Wt::WNavigationBar>("navigation");
		_visitorNavBar->setTemplateText(tr("ERP.NavigationBar"));
		_visitorNavBar->bindString("container-class", "container");
		_visitorNavBar->setResponsive(true);

		auto menu = make_unique<Wt::WMenu>(_visitorStack);
		_visitorMenu = menu.get();
		_visitorMenu->setInternalPathEnabled("/");
		_visitorNavBar->addMenu(move(menu));

		//Logo
		auto logoImage = make_unique<Wt::WImage>(Wt::WLink("images/logo.png"), tr("ERP.Logo.Alt"));
		_visitorNavBar->setTitle(move(logoImage), Wt::WLink(Wt::LinkType::InternalPath, "/"));

		//Page widgets
		//...
	}

	void WApplication::lazyLoadAdminWidgets()
	{
		if(_mainAdminTemplate)
			return;

		_mainAdminTemplate = _mainStack->addNew<Wt::WTemplate>(tr("ERP.Main"));

		_adminStack = _mainAdminTemplate->bindNew<Wt::WStackedWidget>("content");

		_adminNavBar = _mainAdminTemplate->bindNew<Wt::WNavigationBar>("navigation");
		_adminNavBar->setTemplateText(tr("ERP.NavigationBar"));
		_adminNavBar->addStyleClass("navbar-admin");
		_adminNavBar->bindString("container-class", "container-fluid");
		_adminNavBar->setResponsive(true);

		auto menu = make_unique<Wt::WMenu>(_adminStack);
		_adminMenu = menu.get();
		_adminMenu->setInternalPathEnabled("/" ADMIN_PATHC "/");
		_adminNavBar->addMenu(move(menu));

		auto logoutBtn = make_unique<Wt::WPushButton>(tr("LogoutButton"), Wt::TextFormat::XHTML);
		logoutBtn->clicked().connect(&authLogin(), &Wt::Auth::Login::logout);
		_adminNavBar->addWidget(move(logoutBtn), Wt::AlignmentFlag::Right);

		auto logoImage = make_unique<Wt::WImage>(Wt::WLink("images/logo.png"), tr("ERP.Logo.Alt"));
		_adminNavBar->setTitle(move(logoImage), Wt::WLink(Wt::LinkType::InternalPath, "/"));

		//Admin page widgets
		_dashboardAdminPage = addMenuItem(_adminMenu, tr("Dashboard"), make_unique<DashboardAdminPage>());
		_entitiesAdminPage = addMenuItem(_adminMenu, tr("Entities"), make_unique<EntitiesAdminPage>());
		_accountsAdminPage = addMenuItem(_adminMenu, tr("FinancialRecords"), make_unique<AccountsAdminPage>());
		_attendanceAdminPage = addMenuItem(_adminMenu, tr("Attendance"), make_unique<AttendanceAdminPage>());

		//Users
		if(authLogin().hasPermission(Permissions::ViewUser))
			_usersAdminPage = addMenuItem(_adminMenu, tr("Users"), make_unique<UsersAdminPage>());
	}

	void WApplication::initFindEntitySuggestion()
	{
		if(!_findEntitySuggestion)
			_findEntitySuggestion = root()->addNew<FindEntitySuggestionPopup>();
	}

	void WApplication::initFindAccountSuggestion()
	{
		if(!_findAccountSuggestion)
			_findAccountSuggestion = root()->addNew<FindAccountSuggestionPopup>();
	}

	void WApplication::initFindLocationSuggestion()
	{
		if(!_findLocationSuggestion)
			_findLocationSuggestion = root()->addNew<FindLocationSuggestionPopup>();
	}

	void WApplication::initCountryQueryModel()
	{
		if(_countryQueryModel)
			return;

		_countryQueryModel = make_shared<CountryQueryModel>();
		_countryQueryModel->setBatchSize(1000);
		_countryQueryModel->setQuery(dboSession().find<Country>().orderBy("CASE WHEN code = 'PK' THEN 1 ELSE 2 END ASC"));
		_countryQueryModel->addColumn("name");
		_countryQueryModel->addColumn("code");

		_countryProxyModel = make_shared<CountryProxyModel>(_countryQueryModel);
	}

	void WApplication::initCityQueryModel()
	{
		if(_cityQueryModel)
			return;

		_cityQueryModel = make_shared<CityQueryModel>();
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

		_positionQueryModel = make_shared<PositionQueryModel>();
		_positionQueryModel->setBatchSize(1000);
		_positionQueryModel->setQuery(dboSession().find<EmployeePosition>());
		_positionQueryModel->addColumn("title");
		_positionQueryModel->addColumn("id");

		_positionProxyModel = make_shared<PositionProxyModel>(_positionQueryModel);
	}

	void WApplication::initServiceQueryModel()
	{
		if(_serviceQueryModel)
			return;

		_serviceQueryModel = make_shared<ServiceQueryModel>();
		_serviceQueryModel->setBatchSize(1000);
		_serviceQueryModel->setQuery(dboSession().find<ClientService>());
		_serviceQueryModel->addColumn("title");
		_serviceQueryModel->addColumn("id");

		_serviceProxyModel = make_shared<ServiceProxyModel>(_serviceQueryModel);
	}

	void WApplication::initRegionQueryModel()
	{
		if(_regionQueryModel)
			return;

		_regionQueryModel = make_shared<RegionQueryModel>();
		_regionQueryModel->setBatchSize(1000);
		_regionQueryModel->setQuery(dboSession().find<Region>());
		_regionQueryModel->addColumn("name");
		_regionQueryModel->addColumn("id");

		_regionProxyModel = make_shared<RegionProxyModel>(_regionQueryModel);
	}

	void WApplication::showErrorDialog(const Wt::WString &message)
	{
		_errorDialogText->setText(message);
		_errorDialog->show();
	}

	void WApplication::showDbBackendError(const std::string &)
	{
		showErrorDialog(tr("DbInternalError"));
	}

	Wt::WDialog *WApplication::createPasswordPromptDialog()
	{
		if(_passwordPromptDialog)
			return nullptr;

		lazyLoadLoginWidget();
		_passwordPromptDialog = _authWidget->createPasswordPromptDialog(authLogin());
		_passwordPromptDialog->finished().connect(this, std::bind(&WApplication::handlePasswordPromptFinished, this, _1, ""));
		_passwordPromptDialog->rejectWhenEscapePressed(true);
		_passwordPromptDialog->show();
		return _passwordPromptDialog;
	}

	void WApplication::handlePasswordPromptFinished(Wt::DialogCode code, const std::string &rejectInternalPath)
	{
		if(code == Wt::DialogCode::Rejected)
		{
			lazyLoadDeniedPermissionWidget();
			_mainStack->setCurrentWidget(_deniedPermissionWidget);
			if(!rejectInternalPath.empty())
				setInternalPath(rejectInternalPath, true);
		}	

		_passwordPromptDialog->removeFromParent();
		_passwordPromptDialog = nullptr;
	}
}
