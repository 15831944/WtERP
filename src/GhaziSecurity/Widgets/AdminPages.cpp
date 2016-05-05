#include "Widgets/AdminPages.h"
#include "Widgets/EntityList.h"
#include "Widgets/AccountMVC.h"
#include "Widgets/EntryCycleMVC.h"
#include "Widgets/HRMVC.h"

#include <Wt/WNavigationBar>
#include <Wt/WMenu>
#include <Wt/WStackedWidget>
#include <Wt/WText>
#include <Wt/Utils>
#include <web/WebUtils.h>

namespace GS
{

	AdminPageWidget::AdminPageWidget(const std::string basePathComponent, Wt::WContainerWidget *parent)
		: Wt::WTemplate(tr("GS.Admin.Main"), parent), _basePathComponent(basePathComponent)
	{
		_sideBar = new Wt::WNavigationBar();
		_stackWidget = new Wt::WStackedWidget();
		_menu = new Wt::WMenu(_stackWidget);
		//_menu->itemSelected().connect(this, &AdminPageWidget::handleItemSelected);
		_menu->addStyleClass("nav-admin-side");
		_sideBar->setResponsive(true);
		_sideBar->addMenu(_menu, Wt::AlignLeft, true);

		bindWidget("sidebar", _sideBar);
		bindWidget("content", _stackWidget);

		if(_basePathComponent.empty())
			_menu->setInternalPathEnabled("/" ADMIN_PATHC "/");
		else
			_menu->setInternalPathEnabled("/" ADMIN_PATHC "/" + _basePathComponent + "/");
	}

// 	void AdminPageWidget::handleItemSelected(Wt::WMenuItem *item)
// 	{
// 		_titleText->setText(item->text());
// 	}

	Wt::WMenuItem *AdminPageWidget::createMenuItemWrapped(int index, const Wt::WString &label, const std::string &internalPath, Wt::WWidget *contents)
	{
		return createMenuItem(index, label, internalPath, new AdminPageContentWidget(label, contents));
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItemWrapped(const Wt::WString &label, const std::string &internalPath, Wt::WWidget *contents)
	{
		return createMenuItemWrapped(menu()->count(), label, internalPath, contents);
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItem(const Wt::WString &label, const std::string &internalPath, Wt::WWidget *contents)
	{
		return createMenuItem(menu()->count(), label, internalPath, contents);
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItem(int index, const Wt::WString &label, const std::string &internalPath, Wt::WWidget *contents)
	{
		std::string pathComponent = Wt::Utils::prepend(Wt::Utils::append(internalPath, '/'), '/');
		std::string basePath = Wt::Utils::append("/" ADMIN_PATHC "/" + basePathComponent(), '/');

		if(pathComponent.find(basePath.c_str(), 0, basePath.size()) != std::string::npos)
			pathComponent = pathComponent.substr(basePath.size());
		if(!pathComponent.empty() && pathComponent.back() == '/')
			pathComponent.pop_back();
		if(!pathComponent.empty() && pathComponent.front() == '/')
			pathComponent = pathComponent.substr(1);

		auto menuItem = new Wt::WMenuItem(label, contents);
		menuItem->setPathComponent(pathComponent);
		menu()->insertItem(index, menuItem);

		return menuItem;
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

		SubmittableRecordWidget *submittedView = dynamic_cast<SubmittableRecordWidget*>(itemContent(submittedItem));
		if(!submittedView)
		{
			_submitSignalMap[submittedItem].disconnect();
			_submitSignalMap.erase(submittedItem);
			return;
		}

		Wt::WMenuItem *newItem = createMenuItem(submittedView->viewName(), submittedView->viewInternalPath(), submittedItem->takeContents());

		SubmittableRecordWidget *newFormView = submittedView->createFormView();
		AdminPageContentWidget* newPageContainer = new AdminPageContentWidget(submittedItem->text(), dynamic_cast<Wt::WWidget*>(newFormView));
		submittedItem->setContents(newPageContainer, Wt::WMenuItem::LazyLoading);

		_submitSignalMap[submittedItem].disconnect();
		connectFormSubmitted(submittedItem);

		newItem->select();
	}

	void AdminPageWidget::connectFormSubmitted(Wt::WMenuItem *item)
	{
		if(auto view = dynamic_cast<SubmittableRecordWidget*>(itemContent(item)))
			_submitSignalMap[item] = view->submitted().connect(boost::bind(&AdminPageWidget::handleFormSubmitted, this, item));
	}

	Wt::WWidget * AdminPageWidget::itemContent(Wt::WMenuItem *item)
	{
		return dynamic_cast<AdminPageContentWidget*>(item->contentsSafe())->content();
	}

	void AdminPageWidget::setDeniedPermissionWidget()
	{
		if(!_deniedPermissionWidget)
			_deniedPermissionWidget = new Wt::WTemplate(tr("GS.DeniedPermission"), _stackWidget);

		_stackWidget->setCurrentWidget(_deniedPermissionWidget);
	}

	AdminPageContentWidget::AdminPageContentWidget(const Wt::WString &title, Wt::WWidget *content, Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplate(tr("GS.Admin.Main.Content"), parent)
	{
		_title = new Wt::WText(title);
		_content = content;

		bindWidget("title", _title);
		bindWidget("content", _content);
	}

	EntitiesAdminPage::EntitiesAdminPage(Wt::WContainerWidget *parent /*= nullptr*/)
		: AdminPageWidget(ENTITIES_PATHC, parent)
	{
		auto allEntitiesMenuItem = new Wt::WMenuItem(tr("AllEntities"));
		allEntitiesMenuItem->setPathComponent("");
		AllEntityList *allEntityList = new AllEntityList();
		allEntityList->enableFilters();
		allEntitiesMenuItem->setContents(new AdminPageContentWidget(allEntitiesMenuItem->text(), allEntityList));
		allEntitiesMenuItem->triggered().connect(allEntityList, &AllEntityList::reload);
		menu()->addItem(allEntitiesMenuItem);

		auto personsMenuItem = new Wt::WMenuItem(tr("Persons"));
		personsMenuItem->setPathComponent(PERSONS_PATHC);
		PersonList *personList = new PersonList();
		personList->enableFilters();
		personsMenuItem->setContents(new AdminPageContentWidget(personsMenuItem->text(), personList));
		personsMenuItem->triggered().connect(personList, &PersonList::reload);
		menu()->addItem(personsMenuItem);

		auto employeesMenuItem = new Wt::WMenuItem(tr("Employees"));
		employeesMenuItem->setPathComponent(EMPLOYEES_PATHC);
		EmployeeList *employeeList = new EmployeeList();
		employeeList->enableFilters();
		employeesMenuItem->setContents(new AdminPageContentWidget(employeesMenuItem->text(), employeeList));
		employeesMenuItem->triggered().connect(employeeList, &EmployeeList::reload);
		menu()->addItem(employeesMenuItem);

		auto personnelMenuItem = new Wt::WMenuItem(tr("Personnel"));
		personnelMenuItem->setPathComponent(PERSONNEL_PATHC);
		PersonnelList *personnelList = new PersonnelList();
		personnelList->enableFilters();
		personnelMenuItem->setContents(new AdminPageContentWidget(personnelMenuItem->text(), personnelList));
		personnelMenuItem->triggered().connect(personnelList, &PersonnelList::reload);
		menu()->addItem(personnelMenuItem);

		auto businessesMenuItem = new Wt::WMenuItem(tr("Businesses"));
		businessesMenuItem->setPathComponent(BUSINESSES_PATHC);
		BusinessList *businessList = new BusinessList();
		businessList->enableFilters();
		businessesMenuItem->setContents(new AdminPageContentWidget(businessesMenuItem->text(), businessList));
		businessesMenuItem->triggered().connect(businessList, &BusinessList::reload);
		menu()->addItem(businessesMenuItem);

		auto clientsMenuItem = new Wt::WMenuItem(tr("Clients"));
		clientsMenuItem->setPathComponent(CLIENTS_PATHC);
		ClientList *clientList = new ClientList();
		clientList->enableFilters();
		clientsMenuItem->setContents(new AdminPageContentWidget(clientsMenuItem->text(), clientList));
		clientsMenuItem->triggered().connect(clientList, &ClientList::reload);
		menu()->addItem(clientsMenuItem);

		menu()->addSeparator();

		auto assignmentsMenuItem = new Wt::WMenuItem(tr("EmployeeAssignments"));
		assignmentsMenuItem->setPathComponent(EMPLOYEES_PATHC "/" EMPLOYEEASSIGNMENTS_PATHC);
		EmployeeAssignmentList *assignmentList = new EmployeeAssignmentList();
		assignmentList->enableFilters();
		assignmentsMenuItem->setContents(new AdminPageContentWidget(assignmentsMenuItem->text(), assignmentList));
		assignmentsMenuItem->triggered().connect(assignmentList, &EmployeeAssignmentList::reload);
		menu()->addItem(assignmentsMenuItem);

		menu()->addSeparator();

		if(APP->authLogin().hasPermission(Permissions::CreateRecord))
		{
			_newEntityMenuItem = new Wt::WMenuItem(tr("AddNewX").arg(tr("entity")));
			_newEntityMenuItem->setPathComponent(NEW_ENTITY_PATHC);
			EntityView *entityView = new EntityView();
			_newEntityMenuItem->setContents(new AdminPageContentWidget(_newEntityMenuItem->text(), entityView));
			menu()->addItem(_newEntityMenuItem);
			connectFormSubmitted(_newEntityMenuItem);

			auto assignEmployeeMenuItem = new Wt::WMenuItem(tr("AssignEmployee"));
			assignEmployeeMenuItem->setPathComponent(EMPLOYEES_PATHC "/" NEW_EMPLOYEEASSIGNMENT_PATHC);
			EmployeeExpenseView *employeeAssignmentView = new EmployeeExpenseView(true);
			assignEmployeeMenuItem->setContents(new AdminPageContentWidget(assignEmployeeMenuItem->text(), employeeAssignmentView));
			menu()->addItem(assignEmployeeMenuItem);
			connectFormSubmitted(assignEmployeeMenuItem);

			menu()->addSeparator();
		}
	}

	AccountsAdminPage::AccountsAdminPage(Wt::WContainerWidget *parent /*= nullptr*/)
		: AdminPageWidget(ACCOUNTS_PATHC, parent)
	{
		WApplication *app = APP;

		auto accountsMenuItem = new Wt::WMenuItem(tr("Accounts"));
		accountsMenuItem->setPathComponent("");
		AccountList *accountList = new AccountList();
		accountList->enableFilters();
		accountsMenuItem->setContents(new AdminPageContentWidget(accountsMenuItem->text(), accountList));
		accountsMenuItem->triggered().connect(accountList, &AccountList::reload);
		menu()->addItem(accountsMenuItem);

		Wt::Dbo::ptr<Account> cashAccountPtr = app->accountsDatabase().findOrCreateCashAccount();
		auto transactionsMenuItem = new Wt::WMenuItem(tr("Transactions"));
		transactionsMenuItem->setPathComponent(ACCOUNT_PREFIX + boost::lexical_cast<std::string>(cashAccountPtr.id()));
		AccountView *cashAccountView = new AccountView(cashAccountPtr);
		transactionsMenuItem->setContents(new AdminPageContentWidget(transactionsMenuItem->text(), cashAccountView));
		transactionsMenuItem->triggered().connect(cashAccountView, &AccountView::reloadList);
		menu()->addItem(transactionsMenuItem);

		auto recurringIncomesMenuItem = new Wt::WMenuItem(tr("RecurringIncomes"));
		recurringIncomesMenuItem->setPathComponent(INCOMECYCLES_PATHC);
		IncomeCycleList *incomeCycleList = new IncomeCycleList();
		incomeCycleList->enableFilters();
		recurringIncomesMenuItem->setContents(new AdminPageContentWidget(recurringIncomesMenuItem->text(), incomeCycleList));
		recurringIncomesMenuItem->triggered().connect(incomeCycleList, &IncomeCycleList::reload);
		menu()->addItem(recurringIncomesMenuItem);

		auto recurringExpensesMenuItem = new Wt::WMenuItem(tr("RecurringExpenses"));
		recurringExpensesMenuItem->setPathComponent(EXPENSECYCLES_PATHC);
		ExpenseCycleList *expenseCycleList = new ExpenseCycleList();
		expenseCycleList->enableFilters();
		recurringExpensesMenuItem->setContents(new AdminPageContentWidget(recurringExpensesMenuItem->text(), expenseCycleList));
		recurringExpensesMenuItem->triggered().connect(expenseCycleList, &ExpenseCycleList::reload);
		menu()->addItem(recurringExpensesMenuItem);

		menu()->addSeparator();

		if(APP->authLogin().hasPermission(Permissions::CreateRecord))
		{
			auto createAccountMenuItem = new Wt::WMenuItem(tr("CreateAccount"));
			createAccountMenuItem->setPathComponent(NEW_ACCOUNT_PATHC);
			AccountView *accountView = new AccountView();
			createAccountMenuItem->setContents(new AdminPageContentWidget(createAccountMenuItem->text(), accountView));
			menu()->addItem(createAccountMenuItem);
			connectFormSubmitted(createAccountMenuItem);

			auto createAccountEntryMenuItem = new Wt::WMenuItem(tr("CreateAccountEntry"));
			createAccountEntryMenuItem->setPathComponent(NEW_ACCOUNTENTRY_PATHC);
			AccountEntryView* accountEntryView = new AccountEntryView();
			createAccountEntryMenuItem->setContents(new AdminPageContentWidget(createAccountEntryMenuItem->text(), accountEntryView));
			menu()->addItem(createAccountEntryMenuItem);
			connectFormSubmitted(createAccountEntryMenuItem);

			auto createTransactionMenuItem = new Wt::WMenuItem(tr("CreateTransaction"));
			createTransactionMenuItem->setPathComponent(NEW_TRANSACTION_PATHC);
			TransactionView* transactionView = new TransactionView();
			createTransactionMenuItem->setContents(new AdminPageContentWidget(createTransactionMenuItem->text(), transactionView));
			menu()->addItem(createTransactionMenuItem);
			connectFormSubmitted(createTransactionMenuItem);

			auto createRecurringIncomeMenuItem = new Wt::WMenuItem(tr("CreateRecurringIncome"));
			createRecurringIncomeMenuItem->setPathComponent(INCOMECYCLES_PATHC "/" NEW_INCOMECYCLE_PATHC);
			IncomeCycleView* incomeCycleView = new IncomeCycleView();
			createRecurringIncomeMenuItem->setContents(new AdminPageContentWidget(createRecurringIncomeMenuItem->text(), incomeCycleView));
			menu()->addItem(createRecurringIncomeMenuItem);
			connectFormSubmitted(createRecurringIncomeMenuItem);

			auto createRecurringExpenseMenuItem = new Wt::WMenuItem(tr("CreateRecurringExpense"));
			createRecurringExpenseMenuItem->setPathComponent(EXPENSECYCLES_PATHC "/" NEW_EXPENSECYCLE_PATHC);
			EmployeeExpenseView *expenseCycleView = new EmployeeExpenseView(false);
			createRecurringExpenseMenuItem->setContents(new AdminPageContentWidget(createRecurringExpenseMenuItem->text(), expenseCycleView));
			menu()->addItem(createRecurringExpenseMenuItem);
			connectFormSubmitted(createRecurringExpenseMenuItem);

			menu()->addSeparator();
		}
	}

// 	void EntitiesAdminPage::newEntityViewSubmitted()
// 	{
// 		if(!_newEntityView || _newEntityView->entityPtr().id() == -1)
// 			return;
// 
// 		auto submittedEntityView = _newEntityView;
// 		auto submittedEntityItem = _newEntityMenuItem;
// 		int newEntityMenuItemIndex = menu()->indexOf(submittedEntityItem);
// 		std::string newPathComponent = ENTITY_PREFIX + boost::lexical_cast<std::string>(submittedEntityView->entityPtr().id());
// 
// 		menu()->removeItem(submittedEntityItem);
// 		submittedEntityItem->setText(submittedEntityView->viewName());
// 		submittedEntityItem->setPathComponent(newPathComponent);
// 		menu()->addItem(submittedEntityItem);
// 
// 		_newEntityView = new EntityView(Entity::InvalidType);
// 		_submittedConnection.disconnect();
// 		_submittedConnection = _newEntityView->submitted().connect(this, &EntitiesAdminPage::newEntityViewSubmitted);
// 		_newEntityMenuItem = createMenuItem(newEntityMenuItemIndex, tr("AddNewX").arg(tr("entity")), "new", _newEntityView);
// 
// 		submittedEntityItem->select();
// 	}

}