#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Dbo/Configuration.h"

#include "Utilities/FindRecordEdit.h"
#include "Widgets/AdminPages.h"
#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
#include "Widgets/AccountMVC.h"
#include "Widgets/LocationMVC.h"
#include "Widgets/EntryCycleMVC.h"

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

Session::Session()
	: _userDatabase(*this)
{
	setConnectionPool(*WServer::instance()->sqlPool());

	MAP_DBO_TREE((*this))
	MAP_CONFIGURATION_DBO_TREE((*this))
}

Wt::Dbo::ptr<User> Session::user()
{
	if(_login.loggedIn())
		return user(_login.user());
	else
		return Wt::Dbo::ptr<GS::User>();
}

Wt::Dbo::ptr<User> Session::user(const Wt::Auth::User &authUser)
{
	Wt::Dbo::ptr<GS::AuthInfo> authInfo = _userDatabase.find(authUser);
	Wt::Dbo::ptr<GS::User> user = authInfo->user();

	if(!user)
	{
		user = add(new GS::User());
		authInfo.modify()->setUser(user);
	}
	return user;
}

WApplication::WApplication(const Wt::WEnvironment& env)
	: Wt::WApplication(env),
	_startTime(boost::posix_time::microsec_clock::local_time()), _entitiesDatabase(_session), _accountsDatabase(_session)
{
	//Enable server push
	enableUpdates();

	setTitle(Wt::WString::tr("GS.Title"));
	styleSheet().addRule("div.Wt-loading",
		"position:absolute;top:0px;right:0;background:#457044;padding:10px 15px;color:#fff;border-radius:0 0 0 3px;"
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

	setInternalPathDefaultValid(true);
	internalPathChanged().connect(this, &WApplication::handleInternalPathChanged);
	session().login().changed().connect(this, &WApplication::handleAuthChanged);

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

	//Visitor widgets
	_mainVisitorTemplate = new Wt::WTemplate(Wt::WString::tr("GS.Main"), _mainStack);
	_visitorStack = new Wt::WStackedWidget(_mainStack);
	_mainVisitorTemplate->bindWidget("content", _visitorStack);

	_visitorNavBar = new Wt::WNavigationBar();
	_visitorNavBar->setTemplateText(Wt::WString::tr("GS.NavigationBar"));
	_visitorNavBar->bindString("container-class", "container");
	_visitorNavBar->setResponsive(true);
	_visitorNavBar->addMenu(_visitorMenu = new Wt::WMenu(_visitorStack));
	_visitorMenu->setInternalPathEnabled("/");
	_mainVisitorTemplate->bindWidget("navigation", _visitorNavBar);

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

	handleInternalPathChanged(internalPath());
	//WServer *server = WServer::instance();
}

WApplication::~WApplication()
{

}

void WApplication::handleAuthChanged()
{

}

void WApplication::handleInternalPathChanged(std::string path)
{
	Wt::Utils::append(path, '/');
	std::string firstComponent = internalPathNextPart("/");
	if(firstComponent != ADMIN_PATHC)
	{
		_mainStack->setCurrentWidget(_mainVisitorTemplate);
	}
	else
	{
		if(!session().login().loggedIn())
		{
			lazyLoadLoginWidget();
		}
		else
		{
			lazyLoadAdminWidgets();
			_mainStack->setCurrentWidget(_mainAdminTemplate);

			try
			{
				std::string pathPrefix;

				//entity view
				pathPrefix = Entity::viewInternalPath("");
				if(path.find(pathPrefix.c_str(), 0, pathPrefix.size()) != std::string::npos)
				{
					std::string idStr = path.substr(pathPrefix.size(), path.find('/', pathPrefix.size()) - 1);
					if(!_entitiesAdminPage->checkPathComponentExist(ENTITY_PREFIX + idStr))
					{
						long long id = boost::lexical_cast<long long>(idStr);

						Wt::Dbo::Transaction t(session());
						Wt::Dbo::ptr<Entity> ptr = session().load<Entity>(id, true);
						if(ptr)
						{
							EntityView *view = new EntityView(ptr);
							auto menuItem = _entitiesAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
					}
				}

				//account view
				pathPrefix = Account::viewInternalPath("");
				if(path.find(pathPrefix.c_str(), 0, pathPrefix.size()) != std::string::npos)
				{
					std::string idStr = path.substr(pathPrefix.size(), path.find('/', pathPrefix.size()) - 1);
					if(!_accountsAdminPage->checkPathComponentExist(ACCOUNT_PREFIX + idStr))
					{
						long long id = boost::lexical_cast<long long>(idStr);

						Wt::Dbo::Transaction t(session());
						Wt::Dbo::ptr<Account> ptr = session().load<Account>(id, true);
						if(ptr)
						{
							AccountView *view = new AccountView(ptr);
							auto menuItem = _accountsAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
					}
				}

				//account entry view
				pathPrefix = AccountEntry::viewInternalPath("");
				if(path.find(pathPrefix.c_str(), 0, pathPrefix.size()) != std::string::npos)
				{
					std::string idStr = path.substr(pathPrefix.size(), path.find('/', pathPrefix.size()) - 1);
					if(!_accountsAdminPage->checkPathComponentExist(ACCOUNTENTRY_PREFIX + idStr))
					{
						long long id = boost::lexical_cast<long long>(idStr);

						Wt::Dbo::Transaction t(session());
						Wt::Dbo::ptr<AccountEntry> ptr = session().load<AccountEntry>(id, true);
						if(ptr)
						{
							AccountEntryView *view = new AccountEntryView(ptr);
							auto menuItem = _accountsAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
					}
				}

				//income cycle view
				pathPrefix = IncomeCycle::viewInternalPath("");
				if(path.find(pathPrefix.c_str(), 0, pathPrefix.size()) != std::string::npos)
				{
					std::string idStr = path.substr(pathPrefix.size(), path.find('/', pathPrefix.size()) - 1);
					if(!_accountsAdminPage->checkPathComponentExist(INCOMECYCLES_PATHC "/" INCOMECYCLE_PREFIX + idStr))
					{
						long long id = boost::lexical_cast<long long>(idStr);

						Wt::Dbo::Transaction t(session());
						Wt::Dbo::ptr<IncomeCycle> ptr = session().load<IncomeCycle>(id, true);
						if(ptr)
						{
							IncomeCycleView *view = new IncomeCycleView(ptr);
							auto menuItem = _accountsAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
					}
				}

				//expense cycle view
				pathPrefix = ExpenseCycle::viewInternalPath("");
				if(path.find(pathPrefix.c_str(), 0, pathPrefix.size()) != std::string::npos)
				{
					std::string idStr = path.substr(pathPrefix.size(), path.find('/', pathPrefix.size()) - 1);
					if(!_accountsAdminPage->checkPathComponentExist(EXPENSECYCLES_PATHC "/" EXPENSECYCLE_PREFIX + idStr))
					{
						long long id = boost::lexical_cast<long long>(idStr);

						Wt::Dbo::Transaction t(session());
						Wt::Dbo::ptr<ExpenseCycle> ptr = session().load<ExpenseCycle>(id, true);
						if(ptr)
						{
							ExpenseCycleView *view = new ExpenseCycleView(ptr);
							auto menuItem = _accountsAdminPage->createMenuItemWrapped(view->viewName(), view->viewInternalPath(), view);
						}
					}
				}
			}
			catch(boost::bad_lexical_cast &) {}
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

}

void WApplication::lazyLoadAdminWidgets()
{
	if(_mainAdminTemplate)
		return;

	_mainAdminTemplate = new Wt::WTemplate(Wt::WString::tr("GS.Main"), _mainStack);
	_adminStack = new Wt::WStackedWidget(_mainStack);
	_mainAdminTemplate->bindWidget("content", _adminStack);

	_adminNavBar = new Wt::WNavigationBar();
	_adminNavBar->setTemplateText(Wt::WString::tr("GS.NavigationBar"));
	_adminNavBar->addStyleClass("navbar-admin");
	_adminNavBar->bindString("container-class", "container-fluid");
	_adminNavBar->setResponsive(true);
	_adminNavBar->addMenu(_adminMenu = new Wt::WMenu(_adminStack));
	_adminMenu->setInternalPathEnabled("/" ADMIN_PATHC "/");
	_mainAdminTemplate->bindWidget("navigation", _adminNavBar);

	Wt::WImage *logoImage = new Wt::WImage(Wt::WLink("images/logo.png"), Wt::WString::tr("GS.Logo.Alt"));
	_adminNavBar->setTitle(logoImage, Wt::WLink(Wt::WLink::InternalPath, "/"));

	//Admin page widgets
	//Dashboard
	AdminPageWidget *dashboardTemplate = new AdminPageWidget("");

	auto overviewMenuItem = new Wt::WMenuItem(Wt::WString::tr("Overview"), new Wt::WText(Wt::WString::tr("Admin.OverviewPage")));
	overviewMenuItem->setPathComponent("");
	dashboardTemplate->menu()->addItem(overviewMenuItem);

	auto dashboardMenuItem = new Wt::WMenuItem(Wt::WString::tr("Dashboard"), dashboardTemplate);
	dashboardMenuItem->setPathComponent("");
	_adminMenu->addItem(dashboardMenuItem);

	//Entites
	auto entitiesMenuItem = new Wt::WMenuItem(Wt::WString::tr("Entities"), _entitiesAdminPage = new EntitiesAdminPage());
	entitiesMenuItem->setPathComponent(_entitiesAdminPage->basePathComponent());
	_adminMenu->addItem(entitiesMenuItem);

	//Entites
	auto accountsMenuItem = new Wt::WMenuItem(Wt::WString::tr("FinancialRecords"), _accountsAdminPage = new AccountsAdminPage());
	accountsMenuItem->setPathComponent(_accountsAdminPage->basePathComponent());
	_adminMenu->addItem(accountsMenuItem);
}

void WApplication::initFindEntitySuggestion()
{
	if(!_findEntitySuggestion)
		_findEntitySuggestion = new FindEntitySuggestionPopup(Entity::InvalidType, this);
}

void WApplication::initFindPersonSuggestion()
{
	if(!_findPersonSuggestion)
		_findPersonSuggestion = new FindEntitySuggestionPopup(Entity::PersonType, this);
}

void WApplication::initFindBusinessSuggestion()
{
	if(!_findBusinessSuggestion)
		_findBusinessSuggestion = new FindEntitySuggestionPopup(Entity::BusinessType, this);
}

void WApplication::initFindAccountSuggestion()
{
	if(!_findAccountSuggestion)
		_findAccountSuggestion = new FindAccountSuggestionPopup(this);
}

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
	_countryQueryModel->setQuery(session().find<Country>().orderBy("CASE WHEN code = 'PK' THEN 1 ELSE 2 END ASC"));
	_countryQueryModel->addColumn("name");
	_countryQueryModel->addColumn("code");

	_countryProxyModel = new CountryProxyModel(_countryQueryModel, _countryQueryModel);
}

void WApplication::initCityQueryModel()
{
	if(_cityQueryModel)
		return;

	_cityQueryModel = new CityQueryModel(this);
	_cityQueryModel->setQuery(session().find<City>());
	_cityQueryModel->addColumn("name");
	_cityQueryModel->addColumn("id");
	_cityQueryModel->addColumn("country_code");
}

void WApplication::initPositionQueryModel()
{
	if(_positionQueryModel)
		return;

	_positionQueryModel = new PositionQueryModel(this);
	_positionQueryModel->setQuery(session().find<EmployeePosition>());
	_positionQueryModel->addColumn("title");
	_positionQueryModel->addColumn("id");

	_positionProxyModel = new PositionProxyModel(_positionQueryModel, _positionQueryModel);
}

void WApplication::initServiceQueryModel()
{
	if(_serviceQueryModel)
		return;

	_serviceQueryModel = new ServiceQueryModel(this);
	_serviceQueryModel->setQuery(session().find<ClientService>());
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

}
