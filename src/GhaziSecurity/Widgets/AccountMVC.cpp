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

#define AccountNameColumnWidth 250
#define BalanceColumnWidth 200
#define EntityColumnWidth 200
#define TypeColumnWidth 250
#define AmountColumnWidth 100

	AccountList::AccountList()
		: QueryModelFilteredList<ResultType>()
	{ }

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
		
		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT acc.id, acc.name, acc.type, e.id e_id, acc.balance, e.name e_name FROM " + std::string(Account::tableName()) + " acc "
			"LEFT JOIN " + Entity::tableName() + " e ON (e.bal_account_id = acc.id OR e.pnl_account_id = acc.id)");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "acc.");

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
			long long balanceInCents = boost::get<AccountList::ResBalance>(res);
			Wt::WString balanceStr = Wt::WLocale::currentLocale().toString(Money(std::abs(balanceInCents), DEFAULT_CURRENCY));

			switch(type)
			{
			default:
				if(balanceInCents > 0) return Wt::WString::tr("XDebit").arg(balanceStr);
				else if(balanceInCents < 0) return Wt::WString::tr("XCredit").arg(balanceStr);
				else return balanceStr;
			case Account::EntityBalanceAccount:
				if(balanceInCents > 0) return Wt::WString::tr("XReceivable").arg(balanceStr);
				else if(balanceInCents < 0) return Wt::WString::tr("XPayable").arg(balanceStr);
				else return balanceStr;
			case Account::EntityPnlAccount:
				if(balanceInCents > 0) return Wt::WString::tr("XExpense").arg(balanceStr);
				else if(balanceInCents < 0) return Wt::WString::tr("XIncome").arg(balanceStr);
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

	AccountEntryList::AccountEntryList(Wt::Dbo::ptr<Account> accountPtr)
		: QueryModelFilteredList(), _accountPtr(accountPtr)
	{ }

	void AccountEntryList::load()
	{
		bool ld = !loaded();
		QueryModelFilteredList::load();

		if(ld)
		{
			int timestampColumn = viewIndexToColumn(ViewTimestamp);
			if(timestampColumn != -1)
				_tableView->sortByColumn(timestampColumn, Wt::DescendingOrder);
		}
	}

	void AccountEntryList::initFilters()
	{
		filtersTemplate()->addFilterModel(new RangeFilterModel(tr("Amount"), "amount")); filtersTemplate()->addFilter(1);
	}

	void AccountEntryList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.timestamp, e.description, e.amount, e.debit_account_id, e.credit_account_id, oAcc.id oAcc_id, oAcc.name oAcc_name, e.id FROM " + 
			std::string(AccountEntry::tableName()) + " e "
			"INNER JOIN " + Account::tableName() + " oAcc ON (oAcc.id <> ? AND (oAcc.id = e.debit_account_id OR oAcc.id = e.credit_account_id))").bind(_accountPtr.id())
			.where("e.debit_account_id = ? OR e.credit_account_id = ?").bind(_accountPtr.id()).bind(_accountPtr.id());
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewTimestamp, model->addColumn("e.timestamp"), tr("Timestamp"), DateTimeColumnWidth);
		addColumn(ViewDescription, model->addColumn("e.description"), tr("Description"), 400);
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
				return Money(boost::get<AccountEntryList::ResAmount>(res), DEFAULT_CURRENCY);
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
				return Money(boost::get<AccountEntryList::ResAmount>(res), DEFAULT_CURRENCY);
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
		TRANSACTION(app);
		try
		{
			int rows = app->dboSession().query<int>("SELECT COUNT(1) FROM " + std::string(Account::tableName())).where("name = ?").bind(input);
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

	const Wt::WFormModel::Field AccountFormModel::typeField = "type";
	const Wt::WFormModel::Field AccountFormModel::nameField = "name";

	AccountFormModel::AccountFormModel(AccountView *view, Wt::Dbo::ptr<Account> accountPtr /*= Wt::Dbo::ptr<Account>()*/)
		: RecordFormModel(view, accountPtr), _view(view)
	{
		addField(typeField);
		addField(nameField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(typeField, (int)_recordPtr->type);
			setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));

			if(_recordPtr->type != Account::Asset && _recordPtr->type != Account::Liability)
				setVisible(typeField, false);
		}
	}

	AuthLogin::PermissionResult AccountFormModel::checkModifyPermission() const
	{
		auto res = RecordFormModel::checkModifyPermission();
		if(res != AuthLogin::Permitted)
			return res;

		long long cashAccountId = SERVER->configs()->getLongInt("CashAccountId", -1);
		if(cashAccountId != -1 && _recordPtr.id() == cashAccountId)
			return AuthLogin::Denied;

		return res;
	}

	Wt::WWidget * AccountFormModel::createFormWidget(Field field)
	{
		if(field == nameField)
		{
			Wt::WLineEdit *name = new Wt::WLineEdit();
			name->setMaxLength(70);
			AccountNameValidator *nameValidator = new AccountNameValidator(true);
			if(isRecordPersisted())
				nameValidator->setAllowedName(_recordPtr->name);
			setValidator(nameField, nameValidator);
			name->changed().connect(boost::bind(&AbstractRecordFormModel::validateUpdateField, this, nameField));
			return name;
		}
		if(field == typeField)
		{
			Wt::WComboBox *typeCombo = new Wt::WComboBox();
			typeCombo->insertItem(Account::Asset, Wt::boost_any_traits<Account::Type>::asString(Account::Asset, ""));
			typeCombo->insertItem(Account::Liability, Wt::boost_any_traits<Account::Type>::asString(Account::Liability, ""));
			return typeCombo;
		}
		return RecordFormModel::createFormWidget(field);
	}

	bool AccountFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);
		
		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().add(new Account());
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->name = valueText(nameField).toUTF8();
		if(isVisible(typeField))
			_recordPtr.modify()->type = Account::Type(boost::any_cast<int>(value(typeField)));

		t.commit();

		app->dboSession().flush();
		auto nameValidator = dynamic_cast<AccountNameValidator*>(validator(nameField));
		nameValidator->setAllowedName(_recordPtr->name);
		return true;
	}

	AccountView::AccountView(Wt::Dbo::ptr<Account> accountPtr)
		: RecordFormView(tr("GS.Admin.AccountView")), _tempPtr(accountPtr)
	{ }

	void AccountView::initView()
	{
		_model = new AccountFormModel(this, _tempPtr);
		addFormModel("account", _model);
		initEntryList();
	}

	void AccountView::reloadList()
	{
		if(_entryList)
			_entryList->reload();
	}

	Wt::WString AccountView::viewName() const
	{
		if(accountPtr())
		{
			TRANSACTION(APP);
			return accountPtr()->name;
		}
		return RecordFormView::viewName();
	}

	void AccountView::afterSubmitHandler()
	{
		initEntryList();
	}

	void AccountView::initEntryList()
	{
		if(_entryList || !_model->isRecordPersisted())
		{
			bindEmpty("entry-list");
			return;
		}

		_entryList = new AccountEntryList(accountPtr());
		_entryList->enableFilters();
		bindWidget("entry-list", _entryList);
	}

	const Wt::WFormModel::Field BaseAccountEntryFormModel::descriptionField = "description";
	const Wt::WFormModel::Field BaseAccountEntryFormModel::debitAccountField = "debitAccount";
	const Wt::WFormModel::Field BaseAccountEntryFormModel::creditAccountField = "creditAccount";
	const Wt::WFormModel::Field BaseAccountEntryFormModel::amountField = "amount";
	const Wt::WFormModel::Field BaseAccountEntryFormModel::entityField = "entity";

	BaseAccountEntryFormModel::BaseAccountEntryFormModel(AccountEntryView *view, Wt::Dbo::ptr<AccountEntry> accountEntryPtr /*= Wt::Dbo::ptr<AccountEntry>()*/)
		: RecordFormModel(view, accountEntryPtr), _view(view)
	{
		addField(descriptionField);
		addField(debitAccountField);
		addField(creditAccountField);
		addField(amountField);
		addField(entityField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(descriptionField, _recordPtr->description);
			setValue(debitAccountField, _recordPtr->debitAccountPtr());
			setValue(creditAccountField, _recordPtr->creditAccountPtr());
			setValue(amountField, _recordPtr->amount());
		}
	}

	AccountEntryFormModel::AccountEntryFormModel(AccountEntryView *view, Wt::Dbo::ptr<AccountEntry> accountEntryPtr /*= Wt::Dbo::ptr<AccountEntry>()*/)
		: BaseAccountEntryFormModel(view, accountEntryPtr)
	{
		setReadOnly(entityField, true);
		setVisible(entityField, false);

		if(_recordPtr)
			handleAccountChanged(false);
	}

	TransactionFormModel::TransactionFormModel(AccountEntryView *view, Wt::Dbo::ptr<AccountEntry> accountEntryPtr /*= Wt::Dbo::ptr<AccountEntry>()*/)
		: BaseAccountEntryFormModel(view, accountEntryPtr)
	{
		setReadOnly(entityField, false);
		setVisible(entityField, true);
		setReadOnly(debitAccountField, true);
		setReadOnly(creditAccountField, true);
	}

	bool TransactionFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		Wt::Dbo::ptr<Entity> entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		app->accountsDatabase().createEntityAccountsIfNotFound(entityPtr);
		setAccountsFromEntity();

		return BaseAccountEntryFormModel::saveChanges();
	}

	Wt::WWidget * BaseAccountEntryFormModel::createFormWidget(Field field)
	{
		if(field == descriptionField)
		{
			Wt::WTextArea *description = new Wt::WTextArea();
			description->setRows(2);
			setValidator(descriptionField, new Wt::WLengthValidator(0, 255));
			return description;
		}
		if(field == debitAccountField)
		{
			FindAccountEdit *debitFindEdit = new FindAccountEdit();
			return debitFindEdit;
		}
		if(field == creditAccountField)
		{
			FindAccountEdit *creditFindEdit = new FindAccountEdit();
			return creditFindEdit;
		}
		if(field == amountField)
		{
			Wt::WLineEdit *amount = new Wt::WLineEdit();
			auto amountValidator = new Wt::WDoubleValidator();
			amountValidator->setBottom(0);
			amountValidator->setMandatory(true);
			setValidator(amountField, amountValidator);
			return amount;
		}
		if(field == entityField)
		{
			FindEntityEdit *entityFindEdit = new FindEntityEdit();
			return entityFindEdit;
		}
		return RecordFormModel::createFormWidget(field);
	}

	Wt::WWidget *AccountEntryFormModel::createFormWidget(Field field)
	{
		auto w = BaseAccountEntryFormModel::createFormWidget(field);
		if(w)
		{
			if(field == debitAccountField)
			{
				auto debitFindEdit = dynamic_cast<FindAccountEdit*>(w);
				debitFindEdit->valueChanged().connect(boost::bind(&BaseAccountEntryFormModel::handleAccountChanged, this, true));
				setValidator(debitAccountField, new FindAccountValidator(debitFindEdit, true));
			}
			if(field == creditAccountField)
			{
				auto creditFindEdit = dynamic_cast<FindAccountEdit*>(w);
				creditFindEdit->valueChanged().connect(boost::bind(&BaseAccountEntryFormModel::handleAccountChanged, this, true));
				setValidator(creditAccountField, new FindAccountValidator(creditFindEdit, true));
			}
		}
		return w;
	}

	Wt::WWidget *TransactionFormModel::createFormWidget(Field field)
	{
		auto w = BaseAccountEntryFormModel::createFormWidget(field);
		if(w)
		{
			if(field == entityField)
			{
				FindEntityEdit *entityFindEdit = dynamic_cast<FindEntityEdit*>(w);
				entityFindEdit->valueChanged().connect(this, &TransactionFormModel::setAccountsFromEntity);
				setValidator(entityField, new FindEntityValidator(entityFindEdit, true));
			}
		}
		return w;
	}

	void BaseAccountEntryFormModel::handleAccountChanged(bool update /*= true*/)
	{
		long long cashAccountId = SERVER->configs()->getLongInt("CashAccountId", -1);
		if(cashAccountId == -1)
			return;

		if(update)
		{
			_view->updateModelField(this, debitAccountField);
			_view->updateModelField(this, creditAccountField);
		}

		const boost::any &debitData = value(debitAccountField);
		const boost::any &creditData = value(creditAccountField);
		if(debitData.empty() || creditData.empty())
		{
			setValue(entityField, Wt::Dbo::ptr<Entity>());
			if(update)
				_view->updateViewField(this, entityField);
			return;
		}

		Wt::Dbo::ptr<Account> debitPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(debitData);
		Wt::Dbo::ptr<Account> creditPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(creditData);
		if(!debitPtr || !creditPtr)
		{
			setValue(entityField, Wt::Dbo::ptr<Entity>());
			if(update)
				_view->updateViewField(this, entityField);
			return;
		}

		TRANSACTION(APP);
		Wt::Dbo::ptr<Entity> entityPtr;
		if(debitPtr.id() == cashAccountId)
			entityPtr = creditPtr->balOfEntityWPtr;
		else if(creditPtr.id() == cashAccountId)
			entityPtr = debitPtr->balOfEntityWPtr;

		setValue(entityField, entityPtr);
		if(entityPtr)
			setVisible(entityField, true);

		if(update)
			_view->updateViewField(this, entityField);
	}

	void TransactionFormModel::setAccountsFromEntity()
	{
		if(!_isReceipt.is_initialized())
			return;

		long long cashAccountId = SERVER->configs()->getLongInt("CashAccountId", -1);
		if(cashAccountId == -1)
			return;

		_view->updateModelField(this, entityField);
		const boost::any &entityData = value(entityField);

		if(!entityData.empty())
		{
			if(Wt::Dbo::ptr<Entity> entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(entityData))
			{
				WApplication *app = APP;
				TRANSACTION(app);
				if(*_isReceipt)
				{
					setValue(debitAccountField, app->accountsDatabase().findOrCreateCashAccount());
					setValue(creditAccountField, entityPtr->balAccountPtr);
				}
				else
				{
					setValue(debitAccountField, entityPtr->balAccountPtr);
					setValue(creditAccountField, app->accountsDatabase().findOrCreateCashAccount());
				}

				_view->updateViewField(this, debitAccountField);
				_view->updateViewField(this, creditAccountField);
				return;
			}
		}

		setValue(debitAccountField, Wt::Dbo::ptr<Account>());
		setValue(creditAccountField, Wt::Dbo::ptr<Account>());
		_view->updateViewField(this, debitAccountField);
		_view->updateViewField(this, creditAccountField);
	}

	bool BaseAccountEntryFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		Money amount = Money(valueText(amountField).toUTF8(), DEFAULT_CURRENCY);
		Wt::Dbo::ptr<Account> debitAccountPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(value(debitAccountField));
		Wt::Dbo::ptr<Account> creditAccountPtr = boost::any_cast<Wt::Dbo::ptr<Account>>(value(creditAccountField));

		if(!_recordPtr)
			_recordPtr = app->accountsDatabase().createAccountEntry(amount, debitAccountPtr, creditAccountPtr);

		_recordPtr.modify()->type = AccountEntry::UnspecifiedType;
		_recordPtr.modify()->description = valueText(descriptionField).toUTF8();
		_recordPtr.flush();

		t.commit();
		return true;
	}

	AccountEntryView::AccountEntryView(Wt::Dbo::ptr<AccountEntry> accountEntryPtr, bool isTransactionView /*= false*/)
		: RecordFormView(tr("GS.Admin.AccountEntryView")), _tempPtr(accountEntryPtr)
	{ }

	void AccountEntryView::initView()
	{
		_model = new AccountEntryFormModel(this, _tempPtr);
		addFormModel("account", _model);
	}

	Wt::WString AccountEntryView::viewName() const
	{
		if(!accountEntryPtr())
			return RecordFormView::viewName();

		const boost::any &entityData = _model->value(BaseAccountEntryFormModel::entityField);
		if(!entityData.empty())
		{
			if(Wt::Dbo::ptr<Entity> entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(entityData))
			{
				TRANSACTION(APP);
				return tr("TransactionViewName").arg(accountEntryPtr().id()).arg(entityPtr->name);
			}
		}

		return tr("AccountEntryViewName").arg(accountEntryPtr().id());
	}

	TransactionView::TransactionView()
		: AccountEntryView(Wt::Dbo::ptr<AccountEntry>(), true)
	{
		setTemplateText(tr("GS.Admin.TransactionView"));
	}

	void TransactionView::initView()
	{
		_model = new TransactionFormModel(this, _tempPtr);
		addFormModel("account", _model);

		setCondition("select-direction", true);
		setCondition("direction-selected", false);

		_selectReceipt = new Wt::WPushButton(tr("Receipt"));
		_selectReceipt->clicked().connect(boost::bind(&TransactionView::selectDirection, this, true));
		bindWidget("selectReceipt", _selectReceipt);

		_selectPayment = new Wt::WPushButton(tr("Payment"));
		_selectPayment->clicked().connect(boost::bind(&TransactionView::selectDirection, this, false));
		bindWidget("selectPayment", _selectPayment);
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
		model()->_isReceipt = isReceipt;
	}

	void TransactionView::afterSubmitHandler()
	{
		setCondition("select-direction", false);
	}

	void TransactionView::submit()
	{
		auto m = model();
		if(!m || !m->_isReceipt.is_initialized() || !conditionValue("direction-selected"))
			return;

		AccountEntryView::submit();
	}

}

