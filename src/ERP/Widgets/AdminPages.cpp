#include "Widgets/AdminPages.h"
#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
#include "Widgets/AccountMVC.h"
#include "Widgets/EntryCycleMVC.h"
#include "Widgets/HRMVC.h"
#include "Widgets/AttendanceMVC.h"
#include "Widgets/UserMVC.h"
#include "Widgets/DashboardWidgets.h"

#include <Wt/WNavigationBar.h>
#include <Wt/WMenu.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>
#include <Wt/Utils.h>
#include <web/WebUtils.h>

namespace ERP
{

	AdminPageWidget::AdminPageWidget(const std::string basePathComponent)
		: Wt::WTemplate(tr("ERP.Admin.Main")), _basePathComponent(basePathComponent)
	{
		_sideBar = bindNew<Wt::WNavigationBar>("sidebar");
		_stackWidget = bindNew<Wt::WStackedWidget>("content");

		_sideBar->setResponsive(true);
		_menu = _sideBar->addMenu(make_unique<Wt::WMenu>(_stackWidget), Wt::AlignmentFlag::Left, true);
		_menu->addStyleClass("nav-admin-side");

		if(_basePathComponent.empty())
			_menu->setInternalPathEnabled("/" ADMIN_PATHC "/");
		else
			_menu->setInternalPathEnabled("/" ADMIN_PATHC "/" + _basePathComponent + "/");
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItemWrapped(unique_ptr<RecordFormView> contents)
	{
		auto name = contents->viewName();
		auto internalPath = contents->viewInternalPath();
		return createMenuItem(name, internalPath, move(contents), true);
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItemWrapped(const Wt::WString &label, const std::string &pathComponent, unique_ptr<AbstractFilteredList> contents)
	{
		contents->enableFilters();
		return createMenuItemWrapped(label, pathComponent, move(contents), false);
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItemWrapped(const Wt::WString &label, const std::string &path, unique_ptr<Wt::WWidget> contents, bool isInternalPath)
	{
		return createMenuItemWrapped(menu()->count(), label, path, move(contents), isInternalPath);
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItemWrapped(int index, const Wt::WString &label, const std::string &path, unique_ptr<Wt::WWidget> contents, bool isInternalPath)
	{
		return createMenuItem(index, label, path, make_unique<AdminPageContentWidget>(label, move(contents)), isInternalPath);
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItem(const Wt::WString &label, const std::string &path, unique_ptr<Wt::WWidget> contents, bool isInternalPath)
	{
		return createMenuItem(menu()->count(), label, path, move(contents), isInternalPath);
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItem(int index, const Wt::WString &label, const std::string &path, unique_ptr<Wt::WWidget> contents, bool isInternalPath)
	{
		std::string pathComponent = path;
		if(isInternalPath)
		{
			pathComponent = Wt::Utils::prepend(Wt::Utils::append(pathComponent, '/'), '/');
			std::string basePath = Wt::Utils::append("/" ADMIN_PATHC "/" + basePathComponent(), '/');
			if(pathComponent.find(basePath.c_str(), 0, basePath.size()) != std::string::npos)
				pathComponent = pathComponent.substr(basePath.size());
		}

		if(!pathComponent.empty() && pathComponent.back() == '/')
			pathComponent.pop_back();
		if(!pathComponent.empty() && pathComponent.front() == '/')
			pathComponent = pathComponent.substr(1);

		auto menuItem = make_unique<Wt::WMenuItem>(label, move(contents));
		menuItem->setPathComponent(pathComponent);
		return menu()->insertItem(index, move(menuItem));
	}

	bool AdminPageWidget::checkPathComponentExist(const std::string &pathComponent) const
	{
		for(int i = 0; i < menu()->count(); ++i)
		{
			if(menu()->itemAt(i)->pathComponent() == pathComponent)
				return true;
		}
		return false;
	}

	void AdminPageWidget::handleFormSubmitted(Wt::WMenuItem *submittedItem)
	{
		if(!submittedItem)
			return;

		SubmittableRecordWidget *submittedView = nullptr;
		AdminPageContentWidget *contentContainer = dynamic_cast<AdminPageContentWidget*>(submittedItem->contentsSafe());
		if(contentContainer)
			submittedView = dynamic_cast<SubmittableRecordWidget*>(contentContainer->content());

		if(!submittedView)
		{
			_submitSignalMap[submittedItem].disconnect();
			_submitSignalMap.erase(submittedItem);
			return;
		}

		Wt::WMenuItem *newItem = createMenuItem(submittedView->viewName(), submittedView->viewInternalPath(), submittedItem->removeContents(), true);
		contentContainer->title()->setText(submittedView->viewName());

		auto newFormView = submittedView->createFormView();
		auto newPageContainer = make_unique<AdminPageContentWidget>(submittedItem->text(), move(newFormView));
		submittedItem->setContents(move(newPageContainer), Wt::ContentLoading::Lazy);

		_submitSignalMap[submittedItem].disconnect();
		connectFormSubmitted(submittedItem);

		newItem->select();
	}

	void AdminPageWidget::connectFormSubmitted(Wt::WMenuItem *item)
	{
		if(auto view = dynamic_cast<SubmittableRecordWidget*>(itemContent(item)))
			_submitSignalMap[item] = view->submitted().connect(this, std::bind(&AdminPageWidget::handleFormSubmitted, this, item));
	}

	Wt::WWidget *AdminPageWidget::itemContent(Wt::WMenuItem *item)
	{
		return dynamic_cast<AdminPageContentWidget*>(item->contentsSafe())->content();
	}

	void AdminPageWidget::setDeniedPermissionWidget()
	{
		if(!_deniedPermissionWidget)
			_deniedPermissionWidget = _stackWidget->addNew<Wt::WTemplate>(tr("ERP.DeniedPermission"));

		_stackWidget->setCurrentWidget(_deniedPermissionWidget);
	}

	AdminPageContentWidget::AdminPageContentWidget(const Wt::WString &title, unique_ptr<Wt::WWidget> content)
		: Wt::WTemplate(tr("ERP.Admin.Main.Content"))
	{
		_content = content.get();
		bindWidget("content", move(content));
		_title = bindNew<Wt::WText>("title", title);
	}

	DashboardAdminPage::DashboardAdminPage()
		: AdminPageWidget("")
	{
		auto overviewMenuItem = createMenuItemWrapped(tr("Overview"), "", make_unique<DashboardOverviewTemplate>(), false);
		menu()->addSeparator();
	}

	EntitiesAdminPage::EntitiesAdminPage()
		: AdminPageWidget(ENTITIES_PATHC)
	{
		auto allEntitiesMenuItem = createMenuItemWrapped(tr("AllEntities"), "", make_unique<AllEntityList>());
		auto personsMenuItem = createMenuItemWrapped(tr("Persons"), PERSONS_PATHC, make_unique<PersonList>());
		auto businessesMenuItem = createMenuItemWrapped(tr("Businesses"), BUSINESSES_PATHC, make_unique<BusinessList>());
		auto employeesMenuItem = createMenuItemWrapped(tr("Employees"), EMPLOYEES_PATHC, make_unique<EmployeeList>());
		auto personnelMenuItem = createMenuItemWrapped(tr("Personnel"), PERSONNEL_PATHC, make_unique<PersonnelList>());
		auto clientsMenuItem = createMenuItemWrapped(tr("Clients"), CLIENTS_PATHC, make_unique<ClientList>());
		menu()->addSeparator();

		auto empAsnMenuItem = createMenuItemWrapped(tr("EmployeeAssignments"), EMPLOYEES_PATHC "/" EMPLOYEEASSIGNMENTS_PATHC,
			make_unique<EmployeeAssignmentList>());
		auto clAsnMenuItem = createMenuItemWrapped(tr("ClientAssignments"), CLIENTS_PATHC "/" CLIENTASSIGNMENTS_PATHC,
			make_unique<ClientAssignmentList>());
		menu()->addSeparator();

		if(APP->authLogin().hasPermission(Permissions::CreateRecord))
		{
			_newEntityMenuItem = createMenuItemWrapped(tr("AddNewX").arg(tr("entity")), NEW_ENTITY_PATHC, make_unique<EntityView>(), false);
			connectFormSubmitted(_newEntityMenuItem);

			auto assignEmployeeMenuItem = createMenuItemWrapped(tr("AssignEmployee"), EMPLOYEES_PATHC "/" NEW_EMPLOYEEASSIGNMENT_PATHC,
				make_unique<EmployeeAssignmentView>(), false);
			connectFormSubmitted(assignEmployeeMenuItem);

			auto assignClientMenuItem = createMenuItemWrapped(tr("AssignClient"), CLIENTS_PATHC "/" NEW_CLIENTASSIGNMENT_PATHC,
				make_unique<ClientAssignmentView>(), false);
			connectFormSubmitted(assignClientMenuItem);
			
			menu()->addSeparator();
		}
	}

	AccountsAdminPage::AccountsAdminPage()
		: AdminPageWidget(ACCOUNTS_PATHC)
	{
		WApplication *app = APP;
		Dbo::ptr<Account> cashAccountPtr = app->accountsDatabase().findOrCreateCashAccount(true);

		auto accountsMenuItem = createMenuItemWrapped(tr("Accounts"), "", make_unique<AccountList>());
		auto transactionsMenuItem = createMenuItemWrapped(tr("Transactions"), ACCOUNT_PREFIX + std::to_string(cashAccountPtr.id()),
			make_unique<AccountView>(cashAccountPtr), false);
		auto recurringIncomesMenuItem = createMenuItemWrapped(tr("RecurringIncomes"), INCOMECYCLES_PATHC, make_unique<IncomeCycleList>());
		auto recurringExpensesMenuItem = createMenuItemWrapped(tr("RecurringExpenses"), EXPENSECYCLES_PATHC, make_unique<ExpenseCycleList>());
		menu()->addSeparator();

		if(app->authLogin().hasPermission(Permissions::CreateRecord))
		{
			auto createAccountMenuItem = createMenuItemWrapped(tr("CreateAccount"), NEW_ACCOUNT_PATHC, make_unique<AccountView>(), false);
			connectFormSubmitted(createAccountMenuItem);

			auto createAccountEntryMenuItem = createMenuItemWrapped(tr("CreateAccountEntry"), NEW_ACCOUNTENTRY_PATHC,
				make_unique<AccountEntryView>(), false);
			connectFormSubmitted(createAccountEntryMenuItem);

			auto createTransactionMenuItem = createMenuItemWrapped(tr("CreateTransaction"), NEW_TRANSACTION_PATHC,
				make_unique<TransactionView>(), false);
			connectFormSubmitted(createTransactionMenuItem);

			auto createRecurringIncomeMenuItem = createMenuItemWrapped(tr("CreateRecurringIncome"), INCOMECYCLES_PATHC "/" NEW_INCOMECYCLE_PATHC,
				make_unique<IncomeCycleView>(), false);
			connectFormSubmitted(createRecurringIncomeMenuItem);

			auto createRecurringExpenseMenuItem = createMenuItemWrapped(tr("CreateRecurringExpense"), EXPENSECYCLES_PATHC "/" NEW_EXPENSECYCLE_PATHC,
				make_unique<ExpenseCycleView>(), false);
			connectFormSubmitted(createRecurringExpenseMenuItem);

			menu()->addSeparator();
		}
	}

	AttendanceAdminPage::AttendanceAdminPage()
		: AdminPageWidget(ATTENDANCE_PATHC)
	{
		auto attendanceEntriesMenuItem = createMenuItemWrapped(tr("AttendanceEntries"), "", make_unique<AttendanceEntryList>());
		auto attendanceDevicesMenuItem = createMenuItemWrapped(tr("AttendanceDevices"), ATTENDANCEDEVICES_PATHC, make_unique<AttendanceDeviceList>());
		menu()->addSeparator();

		if(APP->authLogin().hasPermission(Permissions::CreateRecord))
		{
			auto newAttendanceEntryMenuItem = createMenuItemWrapped(tr("CreateAttendanceEntry"), NEW_ATTENDANCEENTRY_PATHC,
				make_unique<AttendanceEntryView>(), false);
			connectFormSubmitted(newAttendanceEntryMenuItem);

			auto newAttendanceDeviceMenuItem = createMenuItemWrapped(tr("AddAttendanceDevice"), ATTENDANCEDEVICES_PATHC "/" NEW_ATTENDANCEDEVICE_PATHC,
				make_unique<AttendanceDeviceView>(), false);
			connectFormSubmitted(newAttendanceDeviceMenuItem);

			menu()->addSeparator();
		}
	}

	UsersAdminPage::UsersAdminPage()
		: AdminPageWidget(USERS_PATHC)
	{
		WApplication *app = APP;

		if(app->authLogin().hasPermission(Permissions::ViewUser))
			auto usersMenuItem = createMenuItemWrapped(tr("Users"), "", make_unique<UserList>());

		if(app->authLogin().hasPermission(Permissions::ViewRegion))
			auto regionsMenuItem = createMenuItemWrapped(tr("Regions"), REGIONS_PATHC, make_unique<RegionList>());

		menu()->addSeparator();

		if(app->authLogin().hasPermission(Permissions::CreateRecord))
		{
			if(app->authLogin().hasPermission(Permissions::CreateUser))
			{
				auto newUserMenuItem = createMenuItemWrapped(tr("CreateNewUser"), NEW_USER_PATHC, make_unique<UserView>(), false);
				connectFormSubmitted(newUserMenuItem);
			}

			if(app->authLogin().hasPermission(Permissions::CreateRegion))
			{
				auto newRegionMenuItem = createMenuItemWrapped(tr("AddRegion"), REGIONS_PATHC "/" NEW_REGION_PATHC, make_unique<RegionView>(), false);
				connectFormSubmitted(newRegionMenuItem);
			}

			menu()->addSeparator();
		}
	}

}