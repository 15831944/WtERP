#include "ModelView/AccountMVC.h"
#include "ModelView/EntityList.h"
#include "Application/WServer.h"
#include "Dbo/ConfigurationsDatabase.h"

#include <Wt/WTableView.h>
#include <Wt/WTextArea.h>
#include <Wt/WDoubleValidator.h>

namespace ERP
{

#define AccountNameColumnWidth 250
#define BalanceColumnWidth 200
#define EntityColumnWidth 200
#define TypeColumnWidth 250
#define AmountColumnWidth 100

	void AccountList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "acc.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Name"), "acc.name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(make_shared<RangeFilterModel>(tr("Balance"), "acc.balance"));
		//filtersTemplate()->addFilterModel(make_shared<BitmaskFilterModel>(tr("Recurring?"), "entryCycle", std::bind(&FiltersTemplate::initEntryCycleEdit, _1)));
	}

	void AccountList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();
		
		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT acc.id, acc.name, acc.type, e.id, acc.balance, e.name FROM " + Account::tStr() + " acc "
			"LEFT JOIN " + Entity::tStr() + " e ON (e.bal_account_id = acc.id OR e.pnl_account_id = acc.id)");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "acc.");

		model->setQuery(generateFilteredQuery());
		addColumn(ViewId, model->addColumn("acc.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("acc.name"), tr("Name"), AccountNameColumnWidth);
		addColumn(ViewType, model->addColumn("acc.type"), tr("Type"), TypeColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.id"), tr("Entity"), EntityColumnWidth);
		addColumn(ViewBalance, model->addColumn("acc.balance"), tr("BalanceRs"), BalanceColumnWidth);

		_proxyModel = make_shared<AccountListProxyModel>(_model);
	}

	AccountListProxyModel::AccountListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
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
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::any AccountListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any AccountListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const AccountList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<AccountList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, Account::viewInternalPath(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		auto viewIndex = Wt::any_cast<int>(viewIndexData);

		if(viewIndex == AccountList::ViewBalance && role == Wt::ItemDataRole::Display)
		{
			const AccountList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountList::ResultType>>(sourceModel())->resultRow(idx.row());
			Account::Type type = std::get<AccountList::ResType>(res);
			long long balanceInCents = std::get<AccountList::ResBalance>(res);
			Wt::WString balanceStr = Wt::WLocale::currentLocale().toString(Money(std::abs(balanceInCents), DEFAULT_CURRENCY));

			switch(type)
			{
			default:
				if(balanceInCents > 0) return tr("XDebit").arg(balanceStr);
				else if(balanceInCents < 0) return tr("XCredit").arg(balanceStr);
				else return balanceStr;
			case Account::EntityBalanceAccount:
				if(balanceInCents > 0) return tr("XReceivable").arg(balanceStr);
				else if(balanceInCents < 0) return tr("XPayable").arg(balanceStr);
				else return balanceStr;
			case Account::EntityPnlAccount:
				if(balanceInCents > 0) return tr("XExpense").arg(balanceStr);
				else if(balanceInCents < 0) return tr("XIncome").arg(balanceStr);
				else return balanceStr;
			}
		}

		if(viewIndex == AccountList::ViewEntity)
		{
			if(role == Wt::ItemDataRole::Display)
			{
				const AccountList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountList::ResultType>>(sourceModel())->resultRow(idx.row());
				return std::get<AccountList::ResEntityName>(res);
			}
			else if(role == Wt::ItemDataRole::Link)
			{
				const AccountList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountList::ResultType>>(sourceModel())->resultRow(idx.row());
				const auto &entityId = std::get<AccountList::ResEntityId>(res);
				if(entityId.is_initialized())
					return Wt::WLink(Wt::LinkType::InternalPath, Entity::viewInternalPath(*entityId));
			}
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	void AccountChildrenEntryList::load()
	{
		bool ld = !loaded();
		QueryModelFilteredList::load();

		if(ld)
		{
			int timestampColumn = viewIndexToColumn(ViewTimestamp);
			if(timestampColumn != -1)
				_tableView->sortByColumn(timestampColumn, Wt::SortOrder::Descending);
		}
	}

	void AccountChildrenEntryList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<RangeFilterModel>(tr("Amount"), "amount")); filtersTemplate()->addFilter(1);
	}

	void AccountChildrenEntryList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.timestamp, e.description, e.amount, e.debit_account_id, e.credit_account_id, oAcc.id, oAcc.name, e.id "
			"FROM " + AccountEntry::tStr() + " e "
			"INNER JOIN " + Account::tStr() + " oAcc ON (oAcc.id <> ? AND (oAcc.id = e.debit_account_id OR oAcc.id = e.credit_account_id))")
			.bind(_accountPtr.id())
			.where("e.debit_account_id = ? OR e.credit_account_id = ?").bind(_accountPtr.id()).bind(_accountPtr.id());

		if(_incomeCyclePtr.id() != -1)
			_baseQuery.where("e.incomecycle_id = ?").bind(_incomeCyclePtr.id());

		if(_expenseCyclePtr.id() != -1)
			_baseQuery.where("e.expensecycle_id = ?").bind(_expenseCyclePtr.id());

		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateFilteredQuery());
		addColumn(ViewTimestamp, model->addColumn("e.timestamp"), tr("Timestamp"), DateTimeColumnWidth);
		addColumn(ViewDescription, model->addColumn("e.description"), tr("Description"), 300);
		addColumn(ViewOppositeAccount, model->addColumn("e.amount"), tr("OppositeEntryAccount"), AccountNameColumnWidth);
		addColumn(ViewDebitAmount, model->addColumn("e.debit_account_id"), tr("DebitAccount"), AmountColumnWidth);
		addColumn(ViewCreditAmount, model->addColumn("e.credit_account_id"), tr("CreditAccount"), AmountColumnWidth);

		_proxyModel = make_shared<AccountChildrenEntryListProxyModel>(_model);
		model->setHeaderData(0, Wt::Orientation::Horizontal, _accountPtr, Wt::ItemDataRole::User);
	}

	void AccountEntryList::load()
	{
		bool ld = !loaded();
		QueryModelFilteredList::load();

		if(ld)
		{
			int timestampColumn = viewIndexToColumn(ViewTimestamp);
			if(timestampColumn != -1)
				_tableView->sortByColumn(timestampColumn, Wt::SortOrder::Descending);
		}
	}

	void AccountEntryList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<RangeFilterModel>(tr("Amount"), "amount")); filtersTemplate()->addFilter(1);
	}

	void AccountEntryList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.timestamp, e.description, e.amount, e.debit_account_id, e.credit_account_id, dAcc.name, cAcc.name, e.id "
			"FROM " + AccountEntry::tStr() + " e "
			"INNER JOIN " + Account::tStr() + " dAcc ON (dAcc.id = e.debit_account_id) "
			"INNER JOIN " + Account::tStr() + " cAcc ON (cAcc.id = e.credit_account_id)");

		if(_incomeCyclePtr.id() != -1)
			_baseQuery.where("e.incomecycle_id = ?").bind(_incomeCyclePtr.id());

		if(_expenseCyclePtr.id() != -1)
			_baseQuery.where("e.expensecycle_id = ?").bind(_expenseCyclePtr.id());

		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateFilteredQuery());
		addColumn(ViewTimestamp, model->addColumn("e.timestamp"), tr("Timestamp"), DateTimeColumnWidth);
		addColumn(ViewDescription, model->addColumn("e.description"), tr("Description"), 300);
		addColumn(ViewAmount, model->addColumn("e.amount"), tr("AmountRs"), AmountColumnWidth);
		addColumn(ViewDebitAccount, model->addColumn("dAcc.name dAcc_name"), tr("DebitRs"), AccountNameColumnWidth);
		addColumn(ViewCreditAccount, model->addColumn("cAcc.name cAcc_name"), tr("CreditRs"), AccountNameColumnWidth);

		_proxyModel = make_shared<AccountEntryListProxyModel>(_model);
	}

	AccountChildrenEntryListProxyModel::AccountChildrenEntryListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void AccountChildrenEntryListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::any AccountChildrenEntryListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}
		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any AccountChildrenEntryListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const AccountChildrenEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountChildrenEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<AccountChildrenEntryList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, AccountEntry::viewInternalPath(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		auto viewIndex = Wt::any_cast<int>(viewIndexData);

		//Opposite entry account
		if(viewIndex == AccountChildrenEntryList::ViewOppositeAccount)
		{
			if(role == Wt::ItemDataRole::Display)
			{
				const AccountChildrenEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountChildrenEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
				return std::get<AccountChildrenEntryList::ResOppositeAccountName>(res);
			}
			else if(role == Wt::ItemDataRole::Link)
			{
				const AccountChildrenEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountChildrenEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
				const long long &accountId = std::get<AccountChildrenEntryList::ResOppositeAccountId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, Account::viewInternalPath(accountId));
			}
		}

		//Debit amount
		if(viewIndex == AccountChildrenEntryList::ViewDebitAmount && role == Wt::ItemDataRole::Display)
		{
			const AccountChildrenEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountChildrenEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
			auto accountPtr = Wt::any_cast<Dbo::ptr<Account>>(headerData(0, Wt::Orientation::Horizontal, Wt::ItemDataRole::User));
			const long long &debitAccountId = std::get<AccountChildrenEntryList::ResDebitAccountId>(res);

			if(accountPtr.id() == debitAccountId)
				return Money(std::get<AccountChildrenEntryList::ResAmount>(res), DEFAULT_CURRENCY);
			else
				return Wt::any();
		}
		//Credit amount
		if(viewIndex == AccountChildrenEntryList::ViewCreditAmount && role == Wt::ItemDataRole::Display)
		{
			const AccountChildrenEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountChildrenEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
			auto accountPtr = Wt::any_cast<Dbo::ptr<Account>>(headerData(0, Wt::Orientation::Horizontal, Wt::ItemDataRole::User));
			const long long &creditAccountId = std::get<AccountChildrenEntryList::ResCreditAccountId>(res);

			if(accountPtr.id() == creditAccountId)
				return Money(std::get<AccountChildrenEntryList::ResAmount>(res), DEFAULT_CURRENCY);
			else
				return Wt::any();
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	Wt::WFlags<Wt::ItemFlag> AccountChildrenEntryListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	AccountEntryListProxyModel::AccountEntryListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
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

	Wt::any AccountEntryListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}
		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any AccountEntryListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const AccountEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<AccountEntryList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, AccountEntry::viewInternalPath(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		auto viewIndex = Wt::any_cast<int>(viewIndexData);

		//Debit entry account
		if(viewIndex == AccountEntryList::ViewDebitAccount && role == Wt::ItemDataRole::Link)
		{
			const AccountEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
			const long long &accountId = std::get<AccountEntryList::ResDebitAccountId>(res);
			return Wt::WLink(Wt::LinkType::InternalPath, Account::viewInternalPath(accountId));
		}

		//Credit entry account
		if(viewIndex == AccountEntryList::ViewCreditAccount && role == Wt::ItemDataRole::Link)
		{
			const AccountEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
			const long long &accountId = std::get<AccountEntryList::ResCreditAccountId>(res);
			return Wt::WLink(Wt::LinkType::InternalPath, Account::viewInternalPath(accountId));
		}

		//Amount
		if(viewIndex == AccountEntryList::ViewAmount && role == Wt::ItemDataRole::Display)
		{
			const AccountEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AccountEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
			return Money(std::get<AccountEntryList::ResAmount>(res), DEFAULT_CURRENCY);
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	Wt::WFlags<Wt::ItemFlag> AccountEntryListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::WValidator::Result AccountNameValidator::validate(const Wt::WString &input) const
	{
		Result baseResult = Wt::WValidator::validate(input);
		if(baseResult.state() != Wt::ValidationState::Valid)
			return baseResult;

		if(input.empty())
			return baseResult;

		if(!_allowedName.empty() && input == _allowedName)
			return baseResult;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);
		try
		{
			int rows = app->dboSession().query<int>("SELECT COUNT(1) FROM " + Account::tStr()).where("name = ?").bind(input);
			t.commit();

			if(rows != 0)
				return Result(Wt::ValidationState::Invalid, tr("AccountNameInUse"));
		}
		catch(Dbo::Exception &e)
		{
			Wt::log("error") << "AccountNameValidator::validate(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
			return Result(Wt::ValidationState::Invalid, tr("DatabaseValidationFailed"));
		}

		return baseResult;
	}

	const Wt::WFormModel::Field AccountFormModel::typeField = "type";
	const Wt::WFormModel::Field AccountFormModel::nameField = "name";

	AccountFormModel::AccountFormModel(AccountView *view, Dbo::ptr<Account> accountPtr)
		: RecordFormModel(view, move(accountPtr)), _view(view)
	{
		addField(typeField);
		addField(nameField);
	}

	void AccountFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(typeField, (int)_recordPtr->type);
		setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));

		if(_recordPtr->type != Account::Asset && _recordPtr->type != Account::Liability)
			setVisible(typeField, false);
	}

	void AccountFormModel::persistedHandler()
	{
		setReadOnly(typeField, true);

		auto entryList = _view->bindNew<AccountChildrenEntryList>("entry-list", recordPtr());
		entryList->enableFilters();
	}

	AuthLogin::PermissionResult AccountFormModel::checkModifyPermission() const
	{
		auto res = RecordFormModel::checkModifyPermission();
		if(res != AuthLogin::Permitted)
			return res;

		long long cashAccountId = SERVER->configs().getLongInt("CashAccountId", -1);
		if(cashAccountId != -1 && _recordPtr.id() == cashAccountId)
			return AuthLogin::Denied;

		return res;
	}

	unique_ptr<Wt::WWidget> AccountFormModel::createFormWidget(Field field)
	{
		if(field == nameField)
		{
			auto name = make_unique<Wt::WLineEdit>();
			name->setMaxLength(70);
			auto nameValidator = make_shared<AccountNameValidator>(true);
			if(isRecordPersisted())
			{
				TRANSACTION(APP);
				nameValidator->setAllowedName(_recordPtr->name);
			}
			setValidator(nameField, nameValidator);
			name->changed().connect(this, std::bind(&AbstractRecordFormModel::validateUpdateField, this, nameField));
			return name;
		}
		if(field == typeField)
		{
			auto typeCombo = make_unique<Wt::WComboBox>();
			typeCombo->insertItem(Account::Asset, Wt::any_traits<Account::Type>::asString(Account::Asset, ""));
			typeCombo->insertItem(Account::Liability, Wt::any_traits<Account::Type>::asString(Account::Liability, ""));
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
			_recordPtr = app->dboSession().addNew<Account>();
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->name = valueText(nameField).toUTF8();
		if(isVisible(typeField))
			_recordPtr.modify()->type = Account::Type(Wt::any_cast<int>(value(typeField)));

		t.commit();

		app->dboSession().flush();
		auto nameValidator = static_pointer_cast<AccountNameValidator>(validator(nameField));
		nameValidator->setAllowedName(_recordPtr->name);
		return true;
	}

	AccountView::AccountView(Dbo::ptr<Account> accountPtr)
		: RecordFormView(tr("ERP.Admin.AccountView"))
	{
		_model = newFormModel<AccountFormModel>("account", this, move(accountPtr));
	}

	Wt::WString AccountView::viewName() const
	{
		if(accountPtr())
		{
			TRANSACTION(APP);
			return accountPtr()->name;
		}
		return "AccountView";
	}

	const Wt::WFormModel::Field BaseAccountEntryFormModel::descriptionField = "description";
	const Wt::WFormModel::Field BaseAccountEntryFormModel::debitAccountField = "debitAccount";
	const Wt::WFormModel::Field BaseAccountEntryFormModel::creditAccountField = "creditAccount";
	const Wt::WFormModel::Field BaseAccountEntryFormModel::amountField = "amount";
	const Wt::WFormModel::Field BaseAccountEntryFormModel::entityField = "entity";

	void BaseAccountEntryFormModel::persistedHandler()
	{
		setReadOnly(amountField, true);
		setReadOnly(debitAccountField, true);
		setReadOnly(creditAccountField, true);
	}

	BaseAccountEntryFormModel::BaseAccountEntryFormModel(AccountEntryView *view, Dbo::ptr<AccountEntry> accountEntryPtr)
		: RecordFormModel(view, move(accountEntryPtr)), _view(view)
	{
		addField(descriptionField);
		addField(debitAccountField);
		addField(creditAccountField);
		addField(amountField);
		addField(entityField);
	}

	AccountEntryFormModel::AccountEntryFormModel(AccountEntryView *view, Dbo::ptr<AccountEntry> accountEntryPtr)
		: BaseAccountEntryFormModel(view, move(accountEntryPtr))
	{
		setReadOnly(entityField, true);
		setVisible(entityField, false);
	}

	TransactionFormModel::TransactionFormModel(AccountEntryView *view, Dbo::ptr<AccountEntry> accountEntryPtr)
		: BaseAccountEntryFormModel(view, move(accountEntryPtr))
	{
		setReadOnly(entityField, false);
		setVisible(entityField, true);
		setReadOnly(debitAccountField, true);
		setReadOnly(creditAccountField, true);
	}

	void BaseAccountEntryFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(descriptionField, _recordPtr->description);
		setValue(debitAccountField, _recordPtr->debitAccountPtr());
		setValue(creditAccountField, _recordPtr->creditAccountPtr());
		setValue(amountField, _recordPtr->amount());
	}

	void AccountEntryFormModel::updateFromDb()
	{
		BaseAccountEntryFormModel::updateFromDb();
		handleAccountChanged(false);
	}

	bool TransactionFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		Dbo::ptr<Entity> entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(value(entityField));
		app->accountsDatabase().createEntityAccountsIfNotFound(entityPtr);
		setAccountsFromEntity();

		return BaseAccountEntryFormModel::saveChanges();
	}

	unique_ptr<Wt::WWidget> BaseAccountEntryFormModel::createFormWidget(Field field)
	{
		if(field == descriptionField)
		{
			auto description = make_unique<Wt::WTextArea>();
			description->setRows(2);
			setValidator(descriptionField, make_shared<Wt::WLengthValidator>(0, 255));
			return description;
		}
		if(field == debitAccountField)
		{
			return make_unique<FindAccountEdit>();
		}
		if(field == creditAccountField)
		{
			return make_unique<FindAccountEdit>();
		}
		if(field == amountField)
		{
			auto amount = make_unique<Wt::WLineEdit>();
			auto amountValidator = make_shared<Wt::WDoubleValidator>();
			amountValidator->setBottom(0);
			amountValidator->setMandatory(true);
			setValidator(amountField, amountValidator);
			return amount;
		}
		if(field == entityField)
		{
			return make_unique<FindEntityEdit>();
		}
		return RecordFormModel::createFormWidget(field);
	}

	unique_ptr<Wt::WWidget> AccountEntryFormModel::createFormWidget(Field field)
	{
		auto w = BaseAccountEntryFormModel::createFormWidget(field);
		if(w)
		{
			if(field == debitAccountField)
			{
				auto debitFindEdit = dynamic_cast<FindAccountEdit*>(w.get());
				debitFindEdit->valueChanged().connect(this, std::bind(&BaseAccountEntryFormModel::handleAccountChanged, this, true));
				setValidator(debitAccountField, make_shared<FindAccountValidator>(debitFindEdit, true));
			}
			if(field == creditAccountField)
			{
				auto creditFindEdit = dynamic_cast<FindAccountEdit*>(w.get());
				creditFindEdit->valueChanged().connect(this, std::bind(&BaseAccountEntryFormModel::handleAccountChanged, this, true));
				setValidator(creditAccountField, make_shared<FindAccountValidator>(creditFindEdit, true));
			}
		}
		return w;
	}

	unique_ptr<Wt::WWidget> TransactionFormModel::createFormWidget(Field field)
	{
		auto w = BaseAccountEntryFormModel::createFormWidget(field);
		if(w)
		{
			if(field == entityField)
			{
				auto *entityFindEdit = dynamic_cast<FindEntityEdit*>(w.get());
				entityFindEdit->valueChanged().connect(this, &TransactionFormModel::handleEntityChanged);
				setValidator(entityField, make_shared<FindEntityValidator>(entityFindEdit, true));
			}
		}
		return w;
	}

	void BaseAccountEntryFormModel::handleAccountChanged(bool update)
	{
		long long cashAccountId = SERVER->configs().getLongInt("CashAccountId", -1);
		if(cashAccountId == -1)
			return;

		if(update)
		{
			_view->updateModelField(this, debitAccountField);
			_view->updateModelField(this, creditAccountField);
		}

		const Wt::any &debitData = value(debitAccountField);
		const Wt::any &creditData = value(creditAccountField);
		if(debitData.empty() || creditData.empty())
		{
			setValue(entityField, Dbo::ptr<Entity>());
			if(update)
				_view->updateViewField(this, entityField);
			return;
		}

		Dbo::ptr<Account> debitPtr = Wt::any_cast<Dbo::ptr<Account>>(debitData);
		Dbo::ptr<Account> creditPtr = Wt::any_cast<Dbo::ptr<Account>>(creditData);
		if(!debitPtr || !creditPtr)
		{
			setValue(entityField, Dbo::ptr<Entity>());
			if(update)
				_view->updateViewField(this, entityField);
			return;
		}

		TRANSACTION(APP);
		Dbo::ptr<Entity> entityPtr;
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

		long long cashAccountId = SERVER->configs().getLongInt("CashAccountId", -1);
		if(cashAccountId == -1)
			return;

		_view->updateModelField(this, entityField);
		const Wt::any &entityData = value(entityField);

		if(!entityData.empty())
		{
			if(Dbo::ptr<Entity> entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(entityData))
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

		setValue(debitAccountField, Dbo::ptr<Account>());
		setValue(creditAccountField, Dbo::ptr<Account>());
		_view->updateViewField(this, debitAccountField);
		_view->updateViewField(this, creditAccountField);
	}

	void TransactionFormModel::handleEntityChanged()
	{
		try
		{
			setAccountsFromEntity();
		}
		catch(const Dbo::Exception &e)
		{
			Wt::log("error") << "TransactionFormModel::handleEntityChanged(): Dbo error(" << e.code() << "): " << e.what();
			APP->showDbBackendError(e.code());
		}
	}

	bool BaseAccountEntryFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		Money amount = Money(valueText(amountField).toUTF8(), DEFAULT_CURRENCY);
		Dbo::ptr<Account> debitAccountPtr = Wt::any_cast<Dbo::ptr<Account>>(value(debitAccountField));
		Dbo::ptr<Account> creditAccountPtr = Wt::any_cast<Dbo::ptr<Account>>(value(creditAccountField));

		if(!_recordPtr)
			_recordPtr = app->accountsDatabase().createAccountEntry(amount, debitAccountPtr, creditAccountPtr);

		_recordPtr.modify()->description = valueText(descriptionField).toUTF8();
		_recordPtr.flush();

		t.commit();
		return true;
	}

	AccountEntryView::AccountEntryView(Dbo::ptr<AccountEntry> accountEntryPtr)
		: RecordFormView(tr("ERP.Admin.AccountEntryView"))
	{
		_model = newFormModel<AccountEntryFormModel>("account", this, move(accountEntryPtr));
	}

	Wt::WString AccountEntryView::viewName() const
	{
		if(!accountEntryPtr())
			return "AccountEntryView";

		const Wt::any &entityData = _model->value(BaseAccountEntryFormModel::entityField);
		if(!entityData.empty())
		{
			if(Dbo::ptr<Entity> entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(entityData))
			{
				TRANSACTION(APP);
				return tr("TransactionViewName").arg(accountEntryPtr().id()).arg(entityPtr->name);
			}
		}

		return tr("AccountEntryViewName").arg(accountEntryPtr().id());
	}

	TransactionView::TransactionView()
		: AccountEntryView(nullptr)
	{
		setTemplateText(tr("ERP.Admin.TransactionView"));
		_model = newFormModel<TransactionFormModel>("account", this);
	}

	void TransactionView::initView()
	{
		setCondition("select-direction", true);
		setCondition("direction-selected", false);

		_selectReceipt = bindNew<Wt::WPushButton>("selectReceipt", tr("Receipt"));
		_selectReceipt->clicked().connect(this, std::bind(&TransactionView::selectDirection, this, true));

		_selectPayment = bindNew<Wt::WPushButton>("selectPayment", tr("Payment"));
		_selectPayment->clicked().connect(this, std::bind(&TransactionView::selectDirection, this, false));
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

