#include "Utilities/FindRecordEdit.h"
#include "Utilities/FilteredList.h"
#include "Widgets/EntityList.h"
#include "Widgets/EntityView.h"
#include "Widgets/AccountMVC.h"

#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WTableView>
#include <Wt/WStringListModel>

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while(it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

namespace GS
{

	void FindRecordEditTemplate::setReadOnly(bool readOnly)
	{
		setCondition("readonly", readOnly);
		setCondition("not-readonly", !readOnly);
	}

	void FindRecordEditTemplate::setDisabled(bool disabled)
	{
		setReadOnly(disabled);
		Wt::WTemplate::setDisabled(disabled);
	}

	FindRecordEditTemplate::FindRecordEditTemplate(Wt::WLineEdit *edit, Wt::WContainerWidget *parent /*= nullptr*/) : Wt::WTemplate(tr("GS.FindRecordEdit"), parent), _edit(edit)
	{
		bindWidget("edit", _edit);
		setReadOnly(false);
	}

	template<class Value>
	void FindRecordEdit<Value>::load()
	{
		if(isEnabled())
			_suggestionPopup->forEdit(_lineEdit, Wt::WSuggestionPopup::Editing | Wt::WSuggestionPopup::DropDownIcon);

		Wt::WCompositeWidget::load();
	}

	template<class Value>
	FindRecordEdit<Value>::FindRecordEdit(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WCompositeWidget(parent)
	{
		setFormWidgetImpl(_lineEdit = new Wt::WLineEdit());
		setImplementation(_containerTemplate = new FindRecordEditTemplate(_lineEdit));
		valueChanged().connect(boost::bind(&Wt::WLineEdit::validate, _lineEdit));
	}

	template<class Value>
	void FindRecordEdit<Value>::propagateSetEnabled(bool enabled)
	{
		if(enabled)
			_suggestionPopup->forEdit(_lineEdit, Wt::WSuggestionPopup::Editing | Wt::WSuggestionPopup::DropDownIcon);
		else
			_suggestionPopup->removeEdit(_lineEdit);

		Wt::WCompositeWidget::propagateSetEnabled(enabled);
	}

	template<class Value>
	void FindRecordEdit<Value>::handleActivated(int index, Wt::WFormWidget *lineEdit)
	{
		if(lineEdit != _lineEdit)
			return;

		WApplication *app = WApplication::instance();
		auto itemIndex = _suggestionPopup->model()->index(index, _idColumn);

		if(itemIndex.isValid())
		{
			try
			{
				Wt::Dbo::Transaction t(app->session());
				setValuePtr(app->session().load<Value>(boost::any_cast<long long>(itemIndex.data(Wt::UserRole))));
			}
			catch(Wt::Dbo::Exception &e)
			{
				Wt::log("error") << "FindRecordEdit::handleActivated(): Dbo error(" << e.code() << "): " << e.what();
				app->showDbBackendError(e.code());
			}
		}
		else
			setValuePtr(Wt::Dbo::ptr<Value>());
	}

	template<class Value>
	void FindRecordEdit<Value>::setValuePtr(Wt::Dbo::ptr<Value> ptr)
	{
		if(_valuePtr == ptr)
			return;

		_valuePtr = ptr;
		_valueChanged.emit();
	}

	FindEntityEdit::FindEntityEdit(Entity::Type entityType, Wt::WContainerWidget *parent)
		: FindRecordEdit<Entity>(parent), _entityType(entityType)
	{
		//_lineEdit->setPlaceholderText(tr("FindEntityEditPlaceholder"));

		auto newEntity = new Wt::WPushButton();
		newEntity->clicked().connect(this, &FindEntityEdit::showNewEntityDialog);
		switch(_entityType)
		{
		case Entity::PersonType: newEntity->setText(tr("AddNewPerson")); break;
		case Entity::BusinessType: newEntity->setText(tr("AddNewBusiness")); break;
		default: newEntity->setText(tr("AddNewEntity")); break;
		}
		_containerTemplate->bindWidget("new", newEntity);

		auto showList = new Wt::WPushButton(tr("SelectFromList"));
		showList->clicked().connect(this, &FindEntityEdit::showEntityListDialog);
		_containerTemplate->bindWidget("list", showList);

		WApplication *app = WApplication::instance();
		if(_entityType == Entity::PersonType)
		{
			app->initFindPersonSuggestion();
			_suggestionPopup = app->findPersonSuggestion();
		}
		else if(_entityType == Entity::BusinessType)
		{
			app->initFindBusinessSuggestion();
			_suggestionPopup = app->findBusinessSuggestion();
		}
		else
		{
			app->initFindEntitySuggestion();
			_suggestionPopup = app->findEntitySuggestion();
		}

		_suggestionPopup->activated().connect(this, &FindEntityEdit::handleActivated);
	}

	void FindEntityEdit::showNewEntityDialog()
	{
		_newDialog = new Wt::WDialog(this);
		_newDialog->setClosable(true);
		_newDialog->resize(900, Wt::WLength(95, Wt::WLength::Percentage));
		_newDialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);
		_newDialog->setDeleteWhenHidden(true);

		if(_entityType == Entity::PersonType)
			_newDialog->setWindowTitle(tr("AddNewPerson"));
		else if(_entityType == Entity::BusinessType)
			_newDialog->setWindowTitle(tr("AddNewBusiness"));
		else
			_newDialog->setWindowTitle(tr("AddNewEntity"));

		EntityView *newEntityView = new EntityView(_entityType, _newDialog->contents());
		newEntityView->submitted().connect(boost::bind(&FindEntityEdit::handleEntityViewSubmitted, this, newEntityView));

		_newDialog->show();
	}

	void FindEntityEdit::showEntityListDialog()
	{
		if(!_listDialog)
		{
			_listDialog = new Wt::WDialog(this);
			_listDialog->setClosable(true);
			_listDialog->resize(900, Wt::WLength(95, Wt::WLength::Percentage));
			_listDialog->setTransient(true);
			_listDialog->rejectWhenEscapePressed(true);
			_listDialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);

			AbstractFilteredList *listWidget;
			if(_entityType == Entity::PersonType)
			{
				listWidget = new PersonList(_listDialog->contents());
				_listDialog->setWindowTitle(tr("SelectXFromList").arg(tr("person")));
			}
			else if(_entityType == Entity::BusinessType)
			{
				listWidget = new BusinessList(_listDialog->contents());
				_listDialog->setWindowTitle(tr("SelectXFromList").arg(tr("business")));
			}
			else
			{
				listWidget = new AllEntityList(_listDialog->contents());
				_listDialog->setWindowTitle(tr("SelectXFromList").arg(tr("entity")));
			}

			listWidget->tableView()->setSelectionMode(Wt::SingleSelection);
			listWidget->tableView()->setSelectionBehavior(Wt::SelectRows);
			listWidget->tableView()->selectionChanged().connect(boost::bind(&FindEntityEdit::handleListSelectionChanged, this, listWidget));
		}
		_listDialog->show();
	}

	void FindEntityEdit::handleListSelectionChanged(AbstractFilteredList *listWidget)
	{
		auto indexSet = listWidget->tableView()->selectedIndexes();
		if(indexSet.empty())
			return;

		const auto &index = *indexSet.begin();
		if(!index.isValid())
			return;

		long long entityId = boost::any_cast<long long>(index.data());
		Wt::Dbo::Transaction t(APP->session());
		Wt::Dbo::ptr<Entity> entityPtr = APP->session().load<Entity>(entityId);
		setValuePtr(entityPtr);
		_listDialog->accept();
		t.commit();
	}

	void FindEntityEdit::handleEntityViewSubmitted(EntityView *view)
	{
		setValuePtr(view->entityPtr());
		_newDialog->accept();
	}

	void FindEntityEdit::setValuePtr(Wt::Dbo::ptr<Entity> ptr)
	{
		if(ptr)
			_lineEdit->setText(tr("FindEntityEditValueTemplate").arg(ptr->name).arg(ptr.id()));
		else
			_lineEdit->setText("");

		FindRecordEdit<Entity>::setValuePtr(ptr);
	}

	Wt::WValidator::Result FindEntityValidator::validate(const Wt::WString &) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->lineEdit()->valueText());
		if(baseResult.state() != Valid)
			return baseResult;

		if(_findEdit->lineEdit()->valueText().empty())
			return baseResult;

		if(!_findEdit->valuePtr() ||
			_findEdit->lineEdit()->valueText() != Wt::WString::tr("FindEntityEditValueTemplate").arg(_findEdit->valuePtr()->name).arg(_findEdit->valuePtr().id()))
		{
			if(_findEdit->_entityType == Entity::PersonType)
			{
				if(isMandatory())
					return Result(Invalid, Wt::WString::tr("InvalidPersonSelectionMandatory"));
				else
					return Result(Invalid, Wt::WString::tr("InvalidPersonSelection"));
			}

			if(_findEdit->_entityType == Entity::BusinessType)
			{
				if(isMandatory())
					return Result(Invalid, Wt::WString::tr("InvalidBusinessSelectionMandatory"));
				else
					return Result(Invalid, Wt::WString::tr("InvalidBusinessSelection"));
			}
		}

		return baseResult;
	}

	FindAccountEdit::FindAccountEdit(Wt::WContainerWidget *parent /*= nullptr*/)
		: FindRecordEdit<Account>(parent)
	{
		//_lineEdit->setPlaceholderText(tr("FindAccountEditPlaceholder"));

		auto newAccount = new Wt::WPushButton(tr("AddNewAccount"));
		newAccount->clicked().connect(this, &FindAccountEdit::showNewAccountDialog);
		_containerTemplate->bindWidget("new", newAccount);
		auto showList = new Wt::WPushButton(tr("SelectFromList"));
		showList->clicked().connect(this, &FindAccountEdit::showAccountListDialog);
		_containerTemplate->bindWidget("list", showList);

		WApplication *app = WApplication::instance();
		app->initFindAccountSuggestion();
		_suggestionPopup = app->findAccountSuggestion();
		_suggestionPopup->activated().connect(this, &FindAccountEdit::handleActivated);
	}

	void FindAccountEdit::showNewAccountDialog()
	{
		_newDialog = new Wt::WDialog(tr("AddNewAccount"), this);
		_newDialog->setClosable(true);
		_newDialog->resize(900, Wt::WLength(95, Wt::WLength::Percentage));
		_newDialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);
		_newDialog->setDeleteWhenHidden(true);

		AccountView *newAccountView = new AccountView(Wt::Dbo::ptr<Account>(), _newDialog->contents());
		newAccountView->submitted().connect(boost::bind(&FindAccountEdit::handleAccountViewSubmitted, this, newAccountView));

		_newDialog->show();
	}

	void FindAccountEdit::showAccountListDialog()
	{
		if(!_listDialog)
		{
			_listDialog = new Wt::WDialog(tr("SelectXFromList").arg(tr("account")), this);
			_listDialog->setClosable(true);
			_listDialog->resize(900, Wt::WLength(95, Wt::WLength::Percentage));
			_listDialog->setTransient(true);
			_listDialog->rejectWhenEscapePressed(true);
			_listDialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);

			AbstractFilteredList *listWidget = new AccountList(_listDialog->contents());
			listWidget->tableView()->setSelectionMode(Wt::SingleSelection);
			listWidget->tableView()->setSelectionBehavior(Wt::SelectRows);
			listWidget->tableView()->selectionChanged().connect(boost::bind(&FindAccountEdit::handleListSelectionChanged, this, listWidget));
		}
		_listDialog->show();
	}

	void FindAccountEdit::setValuePtr(Wt::Dbo::ptr<Account> ptr)
	{
		if(ptr)
			_lineEdit->setText(tr("FindAccountEditValueTemplate")
				.arg(ptr->name).arg(ptr.id()).arg(Wt::boost_any_traits<Account::Type>::asString(ptr->type, "")));
		else
			_lineEdit->setText("");

		FindRecordEdit<Account>::setValuePtr(ptr);
	}

	void FindAccountEdit::handleAccountViewSubmitted(AccountView *view)
	{
		setValuePtr(view->accountPtr());
		_newDialog->accept();
	}

	void FindAccountEdit::handleListSelectionChanged(AbstractFilteredList *listWidget)
	{
		auto indexSet = listWidget->tableView()->selectedIndexes();
		if(indexSet.empty())
			return;

		const auto &index = *indexSet.begin();
		if(!index.isValid())
			return;

		long long accountId = boost::any_cast<long long>(index.data());
		Wt::Dbo::Transaction t(APP->session());
		Wt::Dbo::ptr<Account> accountPtr = APP->session().load<Account>(accountId);
		setValuePtr(accountPtr);
		_listDialog->accept();
		t.commit();
	}

	Wt::WValidator::Result FindAccountValidator::validate(const Wt::WString &) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->lineEdit()->valueText());
		if(baseResult.state() != Valid)
			return baseResult;

		if(_findEdit->lineEdit()->valueText().empty())
			return baseResult;

		if(!_findEdit->valuePtr() ||
			_findEdit->lineEdit()->valueText() != Wt::WString::tr("FindAccountEditValueTemplate")
			.arg(_findEdit->valuePtr()->name).arg(_findEdit->valuePtr().id()).arg(Wt::boost_any_traits<Account::Type>::asString(_findEdit->valuePtr()->type, "")))
		{
			if(isMandatory())
				return Result(Invalid, Wt::WString::tr("InvalidAccountSelectionMandatory"));
			else
				return Result(Invalid, Wt::WString::tr("InvalidAccountSelection"));
		}

		return baseResult;
	}

	FindEntitySuggestionPopup::FindEntitySuggestionPopup(Entity::Type typeFilter, Wt::WObject *parent /*= nullptr*/)
		: Wt::WSuggestionPopup({
					   "<b><u>",	//highlightBeginTag
					   "</u></b>",	//highlightEndTag
					   '\0',		//listSeparator(no)
					   " ",		//whitespace
					   " "		//wordSeparators
			}, parent), _typeFilter(typeFilter)
	{
		setMaximumSize(Wt::WLength(), 300);
		setFilterLength(3);
		filterModel().connect(this, &FindEntitySuggestionPopup::handleFilterModel);
	}

	void FindEntitySuggestionPopup::handleFilterModel(const Wt::WString &str)
	{
		Wt::WStringListModel *stringList = dynamic_cast<Wt::WStringListModel*>(model());
		stringList->removeRows(0, stringList->rowCount());

		WApplication *app = APP;
		typedef boost::tuple<std::string, long long> FEMTuple;
		auto query = app->session().query<FEMTuple>("SELECT name, id FROM " + std::string(Entity::tableName())).limit(50);
		auto countQuery = app->session().query<int>("SELECT COUNT(1) FROM " + std::string(Entity::tableName()));

		if(!str.empty())
		{
			std::string condition;
			std::string stlStr = str.toUTF8();
			boost::tokenizer<> tokens(stlStr);
			for(const auto &token : tokens)
			{
				if(!condition.empty())
					condition += " OR ";
				condition += "name LIKE ?";
				query.bind("%" + token + "%");
				countQuery.bind("%" + token + "%");

				if(is_number(token))
				{
					condition += " OR id LIKE ?";
					query.bind("%" + token + "%");
					countQuery.bind("%" + token + "%");
				}
			}

			query.where(condition);
			countQuery.where(condition);
		}
		if(_typeFilter != Entity::InvalidType)
		{
			query.where("type = ?").bind(_typeFilter);
			countQuery.where("type = ?").bind(_typeFilter);
		}

		Wt::Dbo::Transaction t(app->session());
		int count = countQuery;
		Wt::Dbo::collection<FEMTuple> result = query;

		for(const FEMTuple &row : result)
		{
			stringList->addString(tr("FindEntityEditValueTemplate").arg(boost::get<0>(row)).arg(boost::get<1>(row)));
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), boost::get<1>(row), Wt::UserRole);
		}

		if(stringList->rowCount() < count && stringList->rowCount() != 0)
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), "Wt-more-data", Wt::StyleClassRole);
	}

	FindAccountSuggestionPopup::FindAccountSuggestionPopup(Wt::WObject *parent /*= nullptr*/)
		: Wt::WSuggestionPopup({
					   "<b><u>",	//highlightBeginTag
					   "</u></b>",	//highlightEndTag
					   '\0',		//listSeparator(no)
					   " ",		//whitespace
					   " "		//wordSeparators
			}, parent)
	{
		setMaximumSize(Wt::WLength(), 300);
		setFilterLength(3);
		filterModel().connect(this, &FindAccountSuggestionPopup::handleFilterModel);
	}

	void FindAccountSuggestionPopup::handleFilterModel(const Wt::WString &str)
	{
		Wt::WStringListModel *stringList = dynamic_cast<Wt::WStringListModel*>(model());
		stringList->removeRows(0, stringList->rowCount());

		WApplication *app = APP;
		typedef boost::tuple<std::string, long long, Account::Type, std::string> FAMTuple;
		std::string joinPart = "LEFT JOIN " + std::string(Entity::tableName()) + " e ON (e.bal_account_id = acc.id OR e.pnl_account_id = acc.id)";
		auto query = app->session().query<FAMTuple>("SELECT acc.name, acc.id, acc.type, e.name e_name FROM " + std::string(Account::tableName()) + " acc " + joinPart).limit(50);
		auto countQuery = app->session().query<int>("SELECT COUNT(acc.id) FROM " + std::string(Account::tableName()) + " acc " + joinPart);

		if(!str.empty())
		{
			std::string condition;
			std::string stlStr = str.toUTF8();
			boost::tokenizer<> tokens(stlStr);
			for(const auto &token : tokens)
			{
				if(!condition.empty())
					condition += " OR ";
				condition += "acc.name LIKE ? OR e.name LIKE ?";
				query.bind("%" + token + "%").bind("%" + token + "%");
				countQuery.bind("%" + token + "%").bind("%" + token + "%");

				if(is_number(token))
				{
					condition += " OR acc.id LIKE ? OR e.id LIKE ?";
					query.bind("%" + token + "%").bind("%" + token + "%");
					countQuery.bind("%" + token + "%").bind("%" + token + "%");
				}
			}

			query.where(condition);
			countQuery.where(condition);
		}

		Wt::Dbo::Transaction t(app->session());
		int count = countQuery;
		Wt::Dbo::collection<FAMTuple> result = query;

		for(const FAMTuple &row : result)
		{
			if(!boost::get<EntityName>(row).empty())
				stringList->addString(tr("FindAccountEditValueWithEntityTemplate")
					.arg(boost::get<Name>(row)).arg(boost::get<Id>(row)).arg(boost::get<EntityName>(row)));
			else
				stringList->addString(tr("FindAccountEditValueTemplate")
					.arg(boost::get<Name>(row)).arg(boost::get<Id>(row)).arg(Wt::boost_any_traits<Account::Type>::asString(boost::get<Type>(row), "")));

			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), boost::get<Id>(row), Wt::UserRole);
		}

		if(stringList->rowCount() < count && stringList->rowCount() != 0)
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), "Wt-more-data", Wt::StyleClassRole);
	}

	//EXPLICIT SPECIALIZATIONS
	template class FindRecordEdit<Entity>;
	template class FindRecordEdit<Account>;

}
