#include "Widgets/AdminPages.h"
#include "Widgets/EntityList.h"
#include "Widgets/AccountMVC.h"
#include "Widgets/EntryCycleMVC.h"

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
		_titleText = new Wt::WText();
		_menu = new Wt::WMenu(_stackWidget);
		_menu->itemSelected().connect(this, &AdminPageWidget::handleItemSelected);
		_menu->addStyleClass("nav-admin-side");
		_sideBar->setResponsive(true);
		_sideBar->addMenu(_menu, Wt::AlignLeft, true);

		bindWidget("sidebar", _sideBar);
		bindWidget("content", _stackWidget);
		bindWidget("title", _titleText);

		if(_basePathComponent.empty())
			_menu->setInternalPathEnabled("/" ADMIN_PATHC "/");
		else
			_menu->setInternalPathEnabled("/" ADMIN_PATHC "/" + _basePathComponent + "/");
	}

	void AdminPageWidget::handleItemSelected(Wt::WMenuItem *item)
	{
		_titleText->setText(item->text());
	}

	Wt::WMenuItem * AdminPageWidget::createMenuItem(int index, const Wt::WString &label, const std::string &pathComponent, Wt::WWidget *contents)
	{
		auto menuItem = new Wt::WMenuItem(label, contents);
		menuItem->setPathComponent(pathComponent);
		menu()->insertItem(index, menuItem);
		return menuItem;
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItem(const Wt::WString &label, const std::string &pathComponent, Wt::WWidget *contents)
	{
		return createMenuItem(menu()->count(), label, pathComponent, contents);
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

		MyTemplateFormView *submittedView = dynamic_cast<MyTemplateFormView*>(submittedItem->contentsSafe());
		if(!submittedView)
		{
			_submitSignalMap[submittedItem].disconnect();
			_submitSignalMap.erase(submittedItem);
			return;
		}

		std::string newPathComponent = Wt::Utils::prepend(Wt::Utils::append(submittedView->viewInternalPath(), '/'), '/');
		std::string basePath = Wt::Utils::append("/" ADMIN_PATHC "/" + basePathComponent(), '/');

		if(newPathComponent.find(basePath.c_str(), 0, basePath.size()) != std::string::npos)
			newPathComponent = newPathComponent.substr(basePath.size());
		if(!newPathComponent.empty() && newPathComponent.back() == '/')
			newPathComponent.pop_back();

		MyTemplateFormView *newFormView = submittedView->createFormView();
		submittedItem->setContents(newFormView, Wt::WMenuItem::LazyLoading, false);
		Wt::WMenuItem *newItem = createMenuItem(submittedView->viewName(), newPathComponent, submittedView);

		_submitSignalMap[submittedItem].disconnect();
		connectFormSubmitted(submittedItem);

		newItem->select();
	}

	void AdminPageWidget::connectFormSubmitted(Wt::WMenuItem *item)
	{
		if(auto view = dynamic_cast<MyTemplateFormView*>(item->contentsSafe()))
			_submitSignalMap[item] = view->submitted().connect(boost::bind(&AdminPageWidget::handleFormSubmitted, this, item));
	}

	AdminPageContentWidget::AdminPageContentWidget(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplate("GS.Admin.Main.Content")
	{

	}

	EntitiesAdminPage::EntitiesAdminPage(Wt::WContainerWidget *parent /*= nullptr*/)
		: AdminPageWidget(ENTITIES_PATHC, parent)
	{
		auto allEntitiesMenuItem = new Wt::WMenuItem(tr("AllEntities"));
		allEntitiesMenuItem->setPathComponent("");
		AllEntityList *allEntityList = new AllEntityList();
		allEntityList->enableFilters();
		allEntitiesMenuItem->setContents(allEntityList);
		allEntitiesMenuItem->triggered().connect(allEntityList, &AllEntityList::reload);
		menu()->addItem(allEntitiesMenuItem);
		handleItemSelected(allEntitiesMenuItem);

		auto personsMenuItem = new Wt::WMenuItem(tr("Persons"));
		personsMenuItem->setPathComponent(PERSONS_PATHC);
		PersonList *personList = new PersonList();
		personList->enableFilters();
		personsMenuItem->setContents(personList);
		personsMenuItem->triggered().connect(personList, &PersonList::reload);
		menu()->addItem(personsMenuItem);

		auto employeesMenuItem = new Wt::WMenuItem(tr("Employees"));
		employeesMenuItem->setPathComponent(EMPLOYEES_PATHC);
		EmployeeList *employeeList = new EmployeeList();
		employeeList->enableFilters();
		employeesMenuItem->setContents(employeeList);
		employeesMenuItem->triggered().connect(employeeList, &EmployeeList::reload);
		menu()->addItem(employeesMenuItem);

		auto personnelMenuItem = new Wt::WMenuItem(tr("Personnel"));
		personnelMenuItem->setPathComponent(PERSONNEL_PATHC);
		PersonnelList *personnelList = new PersonnelList();
		personnelList->enableFilters();
		personnelMenuItem->setContents(personnelList);
		personnelMenuItem->triggered().connect(personnelList, &PersonnelList::reload);
		menu()->addItem(personnelMenuItem);

		auto businessesMenuItem = new Wt::WMenuItem(tr("Businesses"));
		businessesMenuItem->setPathComponent(BUSINESSES_PATHC);
		BusinessList *businessList = new BusinessList();
		businessList->enableFilters();
		businessesMenuItem->setContents(businessList);
		businessesMenuItem->triggered().connect(businessList, &BusinessList::reload);
		menu()->addItem(businessesMenuItem);

		auto clientsMenuItem = new Wt::WMenuItem(tr("Clients"));
		clientsMenuItem->setPathComponent(CLIENTS_PATHC);
		ClientList *clientList = new ClientList();
		clientList->enableFilters();
		clientsMenuItem->setContents(clientList);
		clientsMenuItem->triggered().connect(clientList, &ClientList::reload);
		menu()->addItem(clientsMenuItem);

		menu()->addSeparator();

		_newEntityMenuItem = new Wt::WMenuItem(tr("AddNewEntity"));
		_newEntityMenuItem->setPathComponent(NEW_ENTITY_PATHC);
		_newEntityMenuItem->setContents(new EntityView());
		menu()->addItem(_newEntityMenuItem);
		connectFormSubmitted(_newEntityMenuItem);

		menu()->addSeparator();
	}

	AccountsAdminPage::AccountsAdminPage(Wt::WContainerWidget *parent /*= nullptr*/)
		: AdminPageWidget(ACCOUNTS_PATHC, parent)
	{
		WApplication *app = APP;

		auto accountsMenuItem = new Wt::WMenuItem(tr("Accounts"));
		accountsMenuItem->setPathComponent("");
		AccountList *accountList = new AccountList();
		accountList->enableFilters();
		accountsMenuItem->setContents(accountList);
		accountsMenuItem->triggered().connect(accountList, &AccountList::reload);
		menu()->addItem(accountsMenuItem);
		handleItemSelected(accountsMenuItem);

		Wt::Dbo::ptr<Account> cashAccountPtr = app->accountsDatabase().findOrCreateCashAccount();
		auto transactionsMenuItem = new Wt::WMenuItem(tr("Transactions"));
		transactionsMenuItem->setPathComponent(ACCOUNT_PREFIX + boost::lexical_cast<std::string>(cashAccountPtr.id()));
		AccountEntryList *transactionList = new AccountEntryList(cashAccountPtr);
		transactionList->enableFilters();
		transactionsMenuItem->setContents(transactionList);
		transactionsMenuItem->triggered().connect(transactionList, &AccountEntryList::reload);
		menu()->addItem(transactionsMenuItem);

		auto recurringIncomesMenuItem = new Wt::WMenuItem(tr("RecurringIncomes"));
		recurringIncomesMenuItem->setPathComponent(INCOMECYCLES_PATHC);
		IncomeCycleList *incomeCycleList = new IncomeCycleList();
		incomeCycleList->enableFilters();
		recurringIncomesMenuItem->setContents(incomeCycleList);
		recurringIncomesMenuItem->triggered().connect(incomeCycleList, &IncomeCycleList::reload);
		menu()->addItem(recurringIncomesMenuItem);

		auto recurringExpensesMenuItem = new Wt::WMenuItem(tr("RecurringExpenses"));
		recurringExpensesMenuItem->setPathComponent(EXPENSECYCLES_PATHC);
		ExpenseCycleList *expenseCycleList = new ExpenseCycleList();
		expenseCycleList->enableFilters();
		recurringExpensesMenuItem->setContents(expenseCycleList);
		recurringExpensesMenuItem->triggered().connect(expenseCycleList, &ExpenseCycleList::reload);
		menu()->addItem(recurringExpensesMenuItem);

		menu()->addSeparator();

		auto createAccountMenuItem = new Wt::WMenuItem(tr("CreateAccount"));
		createAccountMenuItem->setPathComponent(NEW_ACCOUNT_PATHC);
		createAccountMenuItem->setContents(new AccountView());
		menu()->addItem(createAccountMenuItem);
		connectFormSubmitted(createAccountMenuItem);

		auto createAccountEntryMenuItem = new Wt::WMenuItem(tr("CreateAccountEntry"));
		createAccountEntryMenuItem->setPathComponent(NEW_ACCOUNTENTRY_PATHC);
		createAccountEntryMenuItem->setContents(new AccountEntryView());
		menu()->addItem(createAccountEntryMenuItem);
		connectFormSubmitted(createAccountEntryMenuItem);

		auto createTransactionMenuItem = new Wt::WMenuItem(tr("CreateTransaction"));
		createTransactionMenuItem->setPathComponent(NEW_TRANSACTION_PATHC);
		createTransactionMenuItem->setContents(new TransactionView());
		menu()->addItem(createTransactionMenuItem);
		connectFormSubmitted(createTransactionMenuItem);

		auto createRecurringIncomeMenuItem = new Wt::WMenuItem(tr("CreateRecurringIncome"));
		createRecurringIncomeMenuItem->setPathComponent(INCOMECYCLES_PATHC "/" NEW_INCOMECYCLE_PATHC);
		createRecurringIncomeMenuItem->setContents(new IncomeCycleView());
		menu()->addItem(createRecurringIncomeMenuItem);
		connectFormSubmitted(createRecurringIncomeMenuItem);

		auto createRecurringExpenseMenuItem = new Wt::WMenuItem(tr("CreateRecurringExpense"));
		createRecurringExpenseMenuItem->setPathComponent(EXPENSECYCLES_PATHC "/" NEW_EXPENSECYCLE_PATHC);
		createRecurringExpenseMenuItem->setContents(new ExpenseCycleView());
		menu()->addItem(createRecurringExpenseMenuItem);
		connectFormSubmitted(createRecurringExpenseMenuItem);

		menu()->addSeparator();
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
// 		_newEntityMenuItem = createMenuItem(newEntityMenuItemIndex, tr("AddNewEntity"), "new", _newEntityView);
// 
// 		submittedEntityItem->select();
// 	}

}