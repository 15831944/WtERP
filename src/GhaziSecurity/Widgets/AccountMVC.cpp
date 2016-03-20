#include "Widgets/AccountMVC.h"
#include "Widgets/EntityList.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Dbo/ConfigurationsDatabase.h"

#include <Wt/WTableView>
#include <Wt/WPushButton>
#include <Wt/WComboBox>
#include <Wt/WSuggestionPopup>
#include <Wt/WDialog>
#include <Wt/WTextArea>
#include <Wt/WDoubleValidator>

#include <Wt/Dbo/QueryModel>

namespace GS
{

#define IdColumnWidth 80
#define AccountNameColumnWidth 250
#define BalanceColumnWidth 200
#define EntityColumnWidth 200
#define TypeColumnWidth 250

	AccountList::AccountList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
	}

	void AccountList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "acc.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "acc.name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new RangeFilterModel(tr("Balance"), "acc.balance"));
		//filtersTemplate()->addFilterModel(new BitmaskFilterModel(tr("Recurring?"), "entryCycle", std::bind(&FiltersTemplate::initEntryCycleEdit, std::placeholders::_1)));
	}

	void AccountList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>(
			"SELECT acc.id, acc.name, acc.type, e.id e_id, acc.balance, e.name e_name FROM " + std::string(Account::tableName()) + " acc "
			"LEFT JOIN " + Entity::tableName() + " e ON (e.bal_account_id = acc.id OR e.pnl_account_id = acc.id)");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("acc.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("acc.name"), tr("Name"), AccountNameColumnWidth);
		addColumn(ViewType, model->addColumn("acc.type"), tr("Type"), TypeColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.id e_id"), tr("Entity"), EntityColumnWidth);
		addColumn(ViewBalance, model->addColumn("acc.balance"), tr("BalanceRs"), BalanceColumnWidth);

		_proxyModel = new AccountListProxyModel(_model, _model);
	}

	AccountListProxyModel::AccountListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void AccountListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::WFlags<Wt::ItemFlag> AccountListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	boost::any AccountListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	boost::any AccountListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const AccountList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<AccountList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, Account::viewInternalPath(id));
			}
		}

		boost::any viewIndexData = headerData(idx.column(), Wt::Horizontal, Wt::ViewIndexRole);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = boost::any_cast<int>(viewIndexData);

		if(viewIndex == AccountList::ViewBalance && role == Wt::DisplayRole)
		{
			const AccountList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountList::ResultType>*>(sourceModel())->resultRow(idx.row());
			Account::Type type = boost::get<AccountList::ResType>(res);
			double balance = boost::get<AccountList::ResBalance>(res);
			Wt::WString balanceStr = APP->locale().toFixedString(std::abs(balance), 2);

			switch(type)
			{
			default:
				if(balance > 0) return Wt::WString::tr("XDebit").arg(balanceStr);
				else if(balance < 0) return Wt::WString::tr("XCredit").arg(balanceStr);
				else return balanceStr;
			case Account::EntityBalanceAccount:
				if(balance > 0) return Wt::WString::tr("XReceivable").arg(balanceStr);
				else if(balance < 0) return Wt::WString::tr("XPayable").arg(balanceStr);
				else return balanceStr;
			case Account::EntityPnlAccount:
				if(balance > 0) return Wt::WString::tr("XExpense").arg(balanceStr);
				else if(balance < 0) return Wt::WString::tr("XIncome").arg(balanceStr);
				else return balanceStr;
			}
		}

		if(viewIndex == AccountList::ViewEntity)
		{
			if(role == Wt::DisplayRole)
			{
				const AccountList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountList::ResultType>*>(sourceModel())->resultRow(idx.row());
				return boost::get<AccountList::ResEntityName>(res);
			}
			else if(role == Wt::LinkRole)
			{
				const AccountList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountList::ResultType>*>(sourceModel())->resultRow(idx.row());
				const auto &entityId = boost::get<AccountList::ResEntityId>(res);
				if(entityId.is_initialized())
					return Wt::WLink(Wt::WLink::InternalPath, Entity::viewInternalPath(*entityId));
			}
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

#define TimestampColumnWidth 180
#define DescriptionColumnWidth 400
#define AmountColumnWidth 100

	AccountEntryList::AccountEntryList(Wt::Dbo::ptr<Account> accountPtr, Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent), _accountPtr(accountPtr)
	{
		init();

		int timestampColumn = viewIndexToColumn(ViewTimestamp);
		if(timestampColumn != -1)
			_tableView->sortByColumn(timestampColumn, Wt::DescendingOrder);
	}

	void AccountEntryList::initFilters()
	{
		filtersTemplate()->addFilterModel(new RangeFilterModel(tr("Amount"), "amount")); filtersTemplate()->addFilter(1);
	}

	void AccountEntryList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>(
			"SELECT e.timestamp, e.description, e.amount, e.debit_account_id, e.credit_account_id, oAcc.id oAcc_id, oAcc.name oAcc_name, e.id FROM " + 
			std::string(AccountEntry::tableName()) + " e "
			"INNER JOIN " + Account::tableName() + " oAcc ON (oAcc.id <> ? AND (oAcc.id = e.debit_account_id OR oAcc.id = e.credit_account_id))").bind(_accountPtr.id())
			.where("e.debit_account_id = ? OR e.credit_account_id = ?").bind(_accountPtr.id()).bind(_accountPtr.id());
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewTimestamp, model->addColumn("e.timestamp"), tr("Timestamp"), TimestampColumnWidth);
		addColumn(ViewDescription, model->addColumn("e.description"), tr("Description"), DescriptionColumnWidth);
		addColumn(ViewOppositeAccount, model->addColumn("e.amount"), tr("OppositeEntryAccount"), AccountNameColumnWidth);
		addColumn(ViewDebitAmount, model->addColumn("e.debit_account_id"), tr("DebitRs"), AmountColumnWidth);
		addColumn(ViewCreditAmount, model->addColumn("e.credit_account_id"), tr("CreditRs"), AmountColumnWidth);

		_proxyModel = new AccountEntryListProxyModel(_model, _model);
		model->setHeaderData(0, Wt::Horizontal, _accountPtr, Wt::UserRole);
	}

	AccountEntryListProxyModel::AccountEntryListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void AccountEntryListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	boost::any AccountEntryListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	boost::any AccountEntryListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const AccountEntryList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountEntryList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<AccountEntryList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, AccountEntry::viewInternalPath(id));
			}
		}

		boost::any viewIndexData = headerData(idx.column(), Wt::Horizontal, Wt::ViewIndexRole);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = boost::any_cast<int>(viewIndexData);

		//Opposite entry account
		if(viewIndex == AccountEntryList::ViewOppositeAccount)
		{
			if(role == Wt::DisplayRole)
			{
				const AccountEntryList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountEntryList::ResultType>*>(sourceModel())->resultRow(idx.row());
				return boost::get<AccountEntryList::ResOppositeAccountName>(res);
			}
			else if(role == Wt::LinkRole)
			{
				const AccountEntryList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountEntryList::ResultType>*>(sourceModel())->resultRow(idx.row());
				const long long &accountId = boost::get<AccountEntryList::ResOppositeAccountId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, Account::viewInternalPath(accountId));
			}
		}

		//Debit amount
		if(viewIndex == AccountEntryList::ViewDebitAmount && role == Wt::DisplayRole)
		{
			const AccountEntryList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountEntryList::ResultType>*>(sourceModel())->resultRow(idx.row());
			auto accountPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(headerData(0, Wt::Horizontal, Wt::UserRole));
			const long long &debitAccountId = boost::get<AccountEntryList::ResDebitAccountId>(res);

			if(accountPtr.id() == debitAccountId)
				return APP->locale().toFixedString(boost::get<AccountEntryList::ResAmount>(res), 2);
			else
				return boost::any();
		}
		//Credit amount
		if(viewIndex == AccountEntryList::ViewCreditAmount && role == Wt::DisplayRole)
		{
			const AccountEntryList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AccountEntryList::ResultType>*>(sourceModel())->resultRow(idx.row());
			auto accountPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(headerData(0, Wt::Horizontal, Wt::UserRole));
			const long long &creditAccountId = boost::get<AccountEntryList::ResCreditAccountId>(res);

			if(accountPtr.id() == creditAccountId)
				return APP->locale().toFixedString(boost::get<AccountEntryList::ResAmount>(res), 2);
			else
				return boost::any();
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	Wt::WFlags<Wt::ItemFlag> AccountEntryListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::WValidator::Result AccountNameValidator::validate(const Wt::WString &input) const
	{
		Result baseResult = Wt::WValidator::validate(input);
		if(baseResult.state() != Valid)
			return baseResult;

		if(input.empty())
			return baseResult;

		if(!_allowedName.empty() && input == _allowedName)
			return baseResult;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());
		try
		{
			int rows = app->session().query<int>("SELECT COUNT(1) FROM " + std::string(Account::tableName())).where("name = ?").bind(input);
			t.commit();

			if(rows != 0)
				return Result(Invalid, Wt::WString::tr("AccountNameInUse"));
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "AccountNameValidator::validate(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
			return Result(Invalid, Wt::WString::tr("DatabaseValidationFailed"));
		}

		return baseResult;
	}

	const Wt::WFormModel::Field AccountView::typeField = "type";
	const Wt::WFormModel::Field AccountView::nameField = "name";

	AccountView::AccountView(Wt::Dbo::ptr<Account> accountPtr, Wt::WContainerWidget *parent /*= nullptr*/)
		: MyTemplateFormView(tr("GS.Admin.AccountView"), parent), _accountPtr(accountPtr)
	{
		_model = new Wt::WFormModel(this);
		_model->addField(typeField);
		_model->addField(nameField);

		Wt::Dbo::Transaction t(APP->session());
		if(_accountPtr)
		{
			_model->setValue(typeField, (int)_accountPtr->type);
			_model->setValue(nameField, Wt::WString::fromUTF8(_accountPtr->name));

			if(_accountPtr->type == Account::Asset || _accountPtr->type == Account::Liability)
			{
				long long cashAccountId = SERVER->configs()->getLongInt("CashAccountId", -1);
				if(cashAccountId != -1 && _accountPtr.id() == cashAccountId)
					_model->setAllReadOnly(true);
			}
			else
			{
				_model->setAllReadOnly(true);
				_model->setVisible(typeField, false);
			}
		}

		Wt::WLineEdit *name = new Wt::WLineEdit();
		name->setMaxLength(70);
		name->changed().connect(boost::bind(&Wt::WLineEdit::validate, name));
		AccountNameValidator *nameValidator = new AccountNameValidator(true);
		if(_accountPtr)
			nameValidator->setAllowedName(_accountPtr->name);
		_model->setValidator(nameField, nameValidator);
		setFormWidget(nameField, name);

		Wt::WPushButton *submit = new Wt::WPushButton(tr("Submit"));
		submit->clicked().connect(this, &AccountView::submit);
		bindWidget("submit", submit);

		initEntryList();

		updateView(_model);
	}

	Wt::WWidget *AccountView::createFormWidget(Wt::WFormModel::Field field)
	{
		Wt::WComboBox *typeCombo = new Wt::WComboBox();
		typeCombo->insertItem(Account::Asset, Wt::boost_any_traits<Account::Type>::asString(Account::Asset, ""));
		typeCombo->insertItem(Account::Liability, Wt::boost_any_traits<Account::Type>::asString(Account::Liability, ""));
		return typeCombo;
	}

	void AccountView::reloadList()
	{
		if(_entryList)
			_entryList->reload();
	}

	Wt::WString AccountView::viewName() const
	{
		if(_accountPtr)
		{
			Wt::Dbo::Transaction t(APP->session());
			return _accountPtr->name;
		}
		return MyTemplateFormView::viewName();
	}

	void AccountView::submit()
	{
		if(_model->isAllReadOnly())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		updateModel(_model);
		if(!_model->validate())
		{
			updateView(_model);
			return;
		}

		try
		{
			if(!_accountPtr)
				_accountPtr = app->session().add(new Account());

			_accountPtr.modify()->name = _model->valueText(nameField).toUTF8();
			if(_model->isVisible(typeField))
				_accountPtr.modify()->type = Account::Type(boost::any_cast<int>(_model->value(typeField)));
			t.commit();

			auto nameValidator = dynamic_cast<AccountNameValidator*>(_model->validator(nameField));
			nameValidator->setAllowedName(_accountPtr->name);

			_model->setAllReadOnly(true);
			initEntryList();

			updateView(_model);
			submitted().emit();
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "AccountView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	void AccountView::initEntryList()
	{
		if(_entryList || !_accountPtr)
		{
			bindEmpty("entry-list");
			return;
		}

		_entryList = new AccountEntryList(_accountPtr);
		_entryList->enableFilters();
		bindWidget("entry-list", _entryList);
	}

	const Wt::WFormModel::Field AccountEntryView::descriptionField = "description";
	const Wt::WFormModel::Field AccountEntryView::debitAccountField = "debitAccount";
	const Wt::WFormModel::Field AccountEntryView::creditAccountField = "creditAccount";
	const Wt::WFormModel::Field AccountEntryView::amountField = "amount";
	const Wt::WFormModel::Field AccountEntryView::entityField = "entity";

	AccountEntryView::AccountEntryView(Wt::Dbo::ptr<AccountEntry> accountEntryPtr, Wt::WContainerWidget *parent /*= nullptr*/, bool isTransactionView /*= false*/)
		: MyTemplateFormView(tr("GS.Admin.AccountEntryView"), parent), _accountEntryPtr(accountEntryPtr)
	{
		_model = new Wt::WFormModel(this);
		_model->addField(descriptionField);
		_model->addField(debitAccountField);
		_model->addField(creditAccountField);
		_model->addField(amountField);
		_model->addField(entityField);
		
		_model->setReadOnly(entityField, true);
		_model->setVisible(entityField, false);

		Wt::Dbo::Transaction t(APP->session());
		if(_accountEntryPtr)
		{
			_model->setValue(descriptionField, _accountEntryPtr->description);
			_model->setValue(debitAccountField, _accountEntryPtr->debitAccountPtr());
			_model->setValue(creditAccountField, _accountEntryPtr->creditAccountPtr());
			_model->setValue(amountField, _accountEntryPtr->amount());
			_model->setAllReadOnly(true);
		}

		Wt::WTextArea *description = new Wt::WTextArea();
		description->setRows(2);
		_model->setValidator(descriptionField, new Wt::WLengthValidator(0, 255));
		setFormWidget(descriptionField, description);

		FindAccountEdit *debitFindEdit = new FindAccountEdit();
		setFormWidget(debitAccountField, debitFindEdit);
		if(!isTransactionView)
			_model->setValidator(debitAccountField, new FindAccountValidator(debitFindEdit, true));

		FindAccountEdit *creditFindEdit = new FindAccountEdit();
		setFormWidget(creditAccountField, creditFindEdit);
		if(!isTransactionView)
			_model->setValidator(creditAccountField, new FindAccountValidator(creditFindEdit, true));

		Wt::WLineEdit *amount = new Wt::WLineEdit();
		auto amountValidator = new Wt::WDoubleValidator();
		amountValidator->setBottom(0);
		amountValidator->setMandatory(true);
		_model->setValidator(amountField, amountValidator);
		setFormWidget(amountField, amount);

		Wt::WPushButton *submit = new Wt::WPushButton(tr("Submit"));
		submit->clicked().connect(this, &AccountEntryView::submit);
		bindWidget("submit", submit);

		if(!isTransactionView)
		{
			if(_accountEntryPtr)
				handleAccountChanged(false);

			updateView(_model);

			debitFindEdit->valueChanged().connect(boost::bind(&AccountEntryView::handleAccountChanged, this, true));
			creditFindEdit->valueChanged().connect(boost::bind(&AccountEntryView::handleAccountChanged, this, true));
		}
	}

	Wt::WWidget *AccountEntryView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == entityField)
		{
			FindEntityEdit *entityFindEdit = new FindEntityEdit();
			return entityFindEdit;
		}
		return MyTemplateFormView::createFormWidget(field);
	}

	Wt::WWidget *TransactionView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == entityField)
		{
			FindEntityEdit *entityFindEdit = dynamic_cast<FindEntityEdit*>(AccountEntryView::createFormWidget(field));
			entityFindEdit->valueChanged().connect(this, &TransactionView::setAccountsFromEntity);
			_model->setValidator(entityField, new FindEntityValidator(entityFindEdit, true));
			return entityFindEdit;
		}
		return MyTemplateFormView::createFormWidget(field);
	}

	void AccountEntryView::handleAccountChanged(bool update /*= true*/)
{
		long long cashAccountId = SERVER->configs()->getLongInt("CashAccountId", -1);
		if(cashAccountId == -1)
			return;

		if(update)
		{
			updateModelField(_model, debitAccountField);
			updateModelField(_model, creditAccountField);
		}

		const boost::any &debitData = _model->value(debitAccountField);
		const boost::any &creditData = _model->value(creditAccountField);
		if(debitData.empty() || creditData.empty())
		{
			_model->setValue(entityField, Wt::Dbo::ptr<Entity>());
			if(update)
				updateViewField(_model, entityField);
			return;
		}

		Wt::Dbo::ptr<Account> debitPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(debitData);
		Wt::Dbo::ptr<Account> creditPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(creditData);
		if(!debitPtr || !creditPtr)
		{
			_model->setValue(entityField, Wt::Dbo::ptr<Entity>());
			if(update)
				updateViewField(_model, entityField);
			return;
		}

		Wt::Dbo::Transaction t(APP->session());
		Wt::Dbo::ptr<Entity> entityPtr;
		if(debitPtr.id() == cashAccountId)
			entityPtr = creditPtr->balOfEntityWPtr;
		else if(creditPtr.id() == cashAccountId)
			entityPtr = debitPtr->balOfEntityWPtr;

		_model->setValue(entityField, entityPtr);
		_model->setVisible(entityField, true);
		if(update)
			updateViewField(_model, entityField);
	}

	Wt::WString AccountEntryView::viewName() const
	{
		if(!_accountEntryPtr)
			return MyTemplateFormView::viewName();

		const boost::any &entityData = _model->value(entityField);
		if(!entityData.empty())
		{
			if(Wt::Dbo::ptr<Entity> entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(entityData))
			{
				Wt::Dbo::Transaction t(APP->session());
				return tr("TransactionViewName").arg(_accountEntryPtr.id()).arg(entityPtr->name);
			}
		}

		return tr("AccountEntryViewName").arg(_accountEntryPtr.id());
	}

	void AccountEntryView::submit()
	{
		if(_model->isAllReadOnly())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		updateModel(_model);
		if(!_model->validate())
		{
			updateView(_model);
			return;
		}

		try
		{
			double amount = boost::lexical_cast<double>(_model->valueText(amountField).toUTF8());
			Wt::Dbo::ptr<Account> debitAccountPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(_model->value(debitAccountField));
			Wt::Dbo::ptr<Account> creditAccountPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(_model->value(creditAccountField));

			if(!_accountEntryPtr)
				_accountEntryPtr = app->accountsDatabase().createAccountEntry(amount, debitAccountPtr, creditAccountPtr);

			_accountEntryPtr.modify()->type = AccountEntry::Type::UnspecifiedType;
			_accountEntryPtr.modify()->description = _model->valueText(descriptionField).toUTF8();
			_accountEntryPtr.flush(); //for TransactionView::submit()'s Dbo::Transaction
			
			if(t.commit())
			{
				_model->setAllReadOnly(true);
				updateView(_model);
				submitted().emit();
			}
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "AccountEntryView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
			return;
		}
	}

	TransactionView::TransactionView(Wt::WContainerWidget *parent /*= nullptr*/)
		: AccountEntryView(Wt::Dbo::ptr<AccountEntry>(), parent, true)
	{
		setTemplateText(tr("GS.Admin.TransactionView"));

		setCondition("select-direction", true);
		setCondition("direction-selected", false);

		_selectReceipt = new Wt::WPushButton(tr("Receipt"));
		_selectReceipt->clicked().connect(boost::bind(&TransactionView::selectDirection, this, true));
		bindWidget("selectReceipt", _selectReceipt);

		_selectPayment = new Wt::WPushButton(tr("Payment"));
		_selectPayment->clicked().connect(boost::bind(&TransactionView::selectDirection, this, false));
		bindWidget("selectPayment", _selectPayment);

		_model->setReadOnly(entityField, false);
		_model->setVisible(entityField, true);
		_model->setReadOnly(debitAccountField, true);
		_model->setReadOnly(creditAccountField, true);

		//setAccountsFromEntity(); REQUIRES TRANSACTION!!!!!!

		updateView(_model);
	}

	void TransactionView::selectDirection(bool isReceipt)
	{
		if(isReceipt)
		{
			_selectReceipt->addStyleClass("btn-primary");
			_selectPayment->removeStyleClass("btn-primary");
		}
		else
		{
			_selectPayment->addStyleClass("btn-primary");
			_selectReceipt->removeStyleClass("btn-primary");
		}

		setCondition("direction-selected", true);
		_isReceipt = isReceipt;
	}

	void TransactionView::submit()
{
		if(!_isReceipt.is_initialized() || _model->isAllReadOnly() || !conditionValue("direction-selected"))
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		try
		{
			Wt::Dbo::ptr<Entity> entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(_model->value(entityField));
			app->accountsDatabase().createEntityAccountsIfNotFound(entityPtr);
			setAccountsFromEntity();
			AccountEntryView::submit();

			if(t.commit())
			{
				setCondition("select-direction", false);
				_model->setAllReadOnly(true);

				updateView(_model);
				submitted().emit();
			}
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "TransactionView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	void TransactionView::setAccountsFromEntity()
	{
		if(!_isReceipt.is_initialized())
			return;

		long long cashAccountId = SERVER->configs()->getLongInt("CashAccountId", -1);
		if(cashAccountId == -1)
			return;

		updateModelField(_model, entityField);
		const boost::any &entityData = _model->value(entityField);

		if(!entityData.empty())
		{
			if(Wt::Dbo::ptr<Entity> entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(entityData))
			{
				WApplication *app = APP;
				Wt::Dbo::Transaction t(app->session());
				if(*_isReceipt)
				{
					_model->setValue(debitAccountField, app->accountsDatabase().findOrCreateCashAccount());
					_model->setValue(creditAccountField, entityPtr->balAccountPtr);
				}
				else
				{
					_model->setValue(debitAccountField, entityPtr->balAccountPtr);
					_model->setValue(creditAccountField, app->accountsDatabase().findOrCreateCashAccount());
				}

				updateViewField(_model, debitAccountField);
				updateViewField(_model, creditAccountField);
				return;
			}
		}

		_model->setValue(debitAccountField, Wt::Dbo::ptr<Account>());
		_model->setValue(creditAccountField, Wt::Dbo::ptr<Account>());
		updateViewField(_model, debitAccountField);
		updateViewField(_model, creditAccountField);
	}

}

