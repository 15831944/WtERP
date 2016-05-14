#include "Widgets/FindRecordEdit.h"
#include "Widgets/EntityList.h"
#include "Widgets/EntityView.h"
#include "Widgets/AccountMVC.h"
#include "Widgets/LocationMVC.h"
#include "Utilities/FilteredList.h"

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
		bool ld = !loaded();
		AbstractFindRecordEdit::load();

		if(ld)
		{
			if(isEnabled())
				_suggestionPopup->forEdit(_lineEdit, Wt::WSuggestionPopup::Editing | Wt::WSuggestionPopup::DropDownIcon);
		}
	}

	template<class Value>
	FindRecordEdit<Value>::FindRecordEdit()
		: AbstractFindRecordEdit(), _valueChanged(this)
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
		{
			_suggestionPopup->removeEdit(_lineEdit);
			_lineEdit->setPlaceholderText("");
		}

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
				TRANSACTION(app);
				FindRecordEdit::setValuePtr(app->dboSession().load<Value>(boost::any_cast<long long>(itemIndex.data(Wt::UserRole))));
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

	FindEntityEdit::FindEntityEdit(Entity::Type entityType, Entity::SpecificType specificType /*= Entity::UnspecificType*/)
		: FindRecordEdit(), _entityType(entityType), _specificType(specificType)
	{
		_lineEdit->setPlaceholderText(tr("FindEntityEditPlaceholder"));

		auto newEntity = new Wt::WPushButton();
		newEntity->clicked().connect(this, &FindEntityEdit::showNewEntityDialog);
		switch(_entityType)
		{
		case Entity::PersonType: newEntity->setText(tr("AddNewX").arg(tr("person"))); break;
		case Entity::BusinessType: newEntity->setText(tr("AddNewX").arg(tr("business"))); break;
		default: newEntity->setText(tr("AddNewX").arg(tr("entity"))); break;
		}
		_containerTemplate->bindWidget("new", newEntity);

		auto showList = new Wt::WPushButton(tr("SelectFromList"));
		showList->clicked().connect(this, &FindEntityEdit::showEntityListDialog);
		_containerTemplate->bindWidget("list", showList);

		WApplication *app = WApplication::instance();
		app->initFindEntitySuggestion();
		_suggestionPopup = app->findEntitySuggestion();
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
			_newDialog->setWindowTitle(tr("AddNewX").arg(tr("person")));
		else if(_entityType == Entity::BusinessType)
			_newDialog->setWindowTitle(tr("AddNewX").arg(tr("business")));
		else
			_newDialog->setWindowTitle(tr("AddNewX").arg(tr("entity")));

		EntityView *newEntityView = new EntityView(_entityType);
		_newDialog->contents()->addWidget(newEntityView);
		newEntityView->setSpecificType(_specificType);
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
				listWidget = new PersonList();
				_listDialog->setWindowTitle(tr("SelectXFromList").arg(tr("person")));
			}
			else if(_entityType == Entity::BusinessType)
			{
				listWidget = new BusinessList();
				_listDialog->setWindowTitle(tr("SelectXFromList").arg(tr("business")));
			}
			else
			{
				listWidget = new AllEntityList();
				_listDialog->setWindowTitle(tr("SelectXFromList").arg(tr("entity")));
			}
			_listDialog->contents()->addWidget(listWidget);

			listWidget->enableFilters();
			listWidget->tableView()->setSelectionMode(Wt::SingleSelection);
			listWidget->tableView()->setSelectionBehavior(Wt::SelectRows);
			listWidget->tableView()->selectionChanged().connect(boost::bind(&FindEntityEdit::handleListSelectionChanged, this, listWidget));
		}
		else
		{
			if(AbstractFilteredList *listWidget = dynamic_cast<AbstractFilteredList*>(_listDialog->contents()->widget(0)))
				listWidget->reload();
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
		TRANSACTION(APP);
		Wt::Dbo::ptr<Entity> entityPtr = APP->dboSession().load<Entity>(entityId);
		setValuePtr(entityPtr);
		_listDialog->accept();
	}

	void FindEntityEdit::handleEntityViewSubmitted(EntityView *view)
	{
		setValuePtr(view->entityPtr());
		_newDialog->accept();
	}

	void FindEntityEdit::setValuePtr(Wt::Dbo::ptr<Entity> ptr)
	{
		TRANSACTION(APP);
		if(ptr)
			_lineEdit->setText(tr("FindEntityEditValueTemplate").arg(ptr->name).arg(ptr.id()));
		else
			_lineEdit->setText("");

		FindRecordEdit::setValuePtr(ptr);
	}

	Wt::WValidator::Result FindEntityValidator::validate(const Wt::WString &) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->lineEdit()->valueText());
		if(baseResult.state() != Valid)
			return baseResult;

		if(_findEdit->lineEdit()->valueText().empty())
			return baseResult;
		
		WApplication *app = APP;
		TRANSACTION(app);

		//Check selected entity, its type and line edit text
		if(!_findEdit->valuePtr() 
			|| (_findEdit->_entityType != Entity::InvalidType && _findEdit->valuePtr()->type != _findEdit->_entityType) 
			|| _findEdit->lineEdit()->valueText() != Wt::WString::tr("FindEntityEditValueTemplate").arg(_findEdit->valuePtr()->name).arg(_findEdit->valuePtr().id()))
		{
			if(_findEdit->_entityType == Entity::PersonType)
			{
				if(isMandatory())
					return Result(Invalid, Wt::WString::tr("InvalidXSelectionMandatory").arg(Wt::WString::tr("person")));
				else
					return Result(Invalid, Wt::WString::tr("InvalidXSelection").arg(Wt::WString::tr("person")));
			}
			else if(_findEdit->_entityType == Entity::BusinessType)
			{
				if(isMandatory())
					return Result(Invalid, Wt::WString::tr("InvalidXSelectionMandatory").arg(Wt::WString::tr("business")));
				else
					return Result(Invalid, Wt::WString::tr("InvalidXSelection").arg(Wt::WString::tr("business")));
			}
			else
			{
				if(isMandatory())
					return Result(Invalid, Wt::WString::tr("InvalidVSelectionMandatory").arg(Wt::WString::tr("entity")));
				else
					return Result(Invalid, Wt::WString::tr("InvalidVSelection").arg(Wt::WString::tr("entity")));
			}
		}

		if(_findEdit->valuePtr())
		{
			//Check role
			if((_findEdit->valuePtr()->specificTypeMask & _findEdit->_specificType) != _findEdit->_specificType)
			{
				if(_findEdit->_specificType == Entity::EmployeeType)
				{
					if(isMandatory())
						return Result(Invalid, Wt::WString::tr("InvalidVSelectionMandatory").arg(Wt::WString::tr("employee")));
					else
						return Result(Invalid, Wt::WString::tr("InvalidVSelection").arg(Wt::WString::tr("employee")));
				}
				else if(_findEdit->_entityType == Entity::PersonnelType)
				{
					if(isMandatory())
						return Result(Invalid, Wt::WString::tr("InvalidXSelectionMandatory").arg(Wt::WString::tr("personnel")));
					else
						return Result(Invalid, Wt::WString::tr("InvalidXSelection").arg(Wt::WString::tr("personnel")));
				}
				else if(_findEdit->_entityType == Entity::ClientType)
				{
					if(isMandatory())
						return Result(Invalid, Wt::WString::tr("InvalidXSelectionMandatory").arg(Wt::WString::tr("client")));
					else
						return Result(Invalid, Wt::WString::tr("InvalidXSelection").arg(Wt::WString::tr("client")));
				}
				else
					return Result(Invalid, Wt::WString::tr("InvalidSelectionSpecificType"));
			}

			//Check permission
			if(app->authLogin().checkRecordViewPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Invalid, Wt::WString::tr("InvalidXSelectionPermissionDenied").arg(Wt::WString::tr("entity")));

			if(_modifyPermissionRequired && app->authLogin().checkRecordModifyPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Invalid, Wt::WString::tr("InvalidXSelectionPermissionDenied").arg(Wt::WString::tr("entity")));
		}

		return baseResult;
	}

	FindAccountEdit::FindAccountEdit()
		: FindRecordEdit()
	{
		_lineEdit->setPlaceholderText(tr("FindAccountEditPlaceholder"));

		auto newAccount = new Wt::WPushButton(tr("AddNewX").arg(tr("account")));
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
		_newDialog = new Wt::WDialog(tr("AddNewX").arg(tr("account")), this);
		_newDialog->setClosable(true);
		_newDialog->resize(900, Wt::WLength(95, Wt::WLength::Percentage));
		_newDialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);
		_newDialog->setDeleteWhenHidden(true);

		AccountView *view = new AccountView(Wt::Dbo::ptr<Account>());
		_newDialog->contents()->addWidget(view);
		view->submitted().connect(boost::bind(&FindAccountEdit::handleAccountViewSubmitted, this, view));

		_newDialog->show();
	}

	void FindAccountEdit::handleAccountViewSubmitted(AccountView *view)
	{
		setValuePtr(view->accountPtr());
		_newDialog->accept();
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

			AbstractFilteredList *listWidget = new AccountList();
			_listDialog->contents()->addWidget(listWidget);
			listWidget->enableFilters();
			listWidget->tableView()->setSelectionMode(Wt::SingleSelection);
			listWidget->tableView()->setSelectionBehavior(Wt::SelectRows);
			listWidget->tableView()->selectionChanged().connect(boost::bind(&FindAccountEdit::handleListSelectionChanged, this, listWidget));
		}
		else
		{
			if(AbstractFilteredList *listWidget = dynamic_cast<AbstractFilteredList*>(_listDialog->contents()->widget(0)))
				listWidget->reload();
		}
		_listDialog->show();
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
		TRANSACTION(APP);
		Wt::Dbo::ptr<Account> accountPtr = APP->dboSession().load<Account>(accountId);
		setValuePtr(accountPtr);
		_listDialog->accept();
	}

	void FindAccountEdit::setValuePtr(Wt::Dbo::ptr<Account> ptr)
	{
		TRANSACTION(APP);
		if(ptr)
			_lineEdit->setText(tr("FindAccountEditValueTemplate")
				.arg(Wt::WString::tr("FindAccountEditValuePrefix").arg(ptr->name).arg(ptr.id())).arg(Wt::boost_any_traits<Account::Type>::asString(ptr->type, "")));
		else
			_lineEdit->setText("");

		FindRecordEdit::setValuePtr(ptr);
	}

	Wt::WValidator::Result FindAccountValidator::validate(const Wt::WString &) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->lineEdit()->valueText());
		if(baseResult.state() != Valid)
			return baseResult;

		if(_findEdit->lineEdit()->valueText().empty())
			return baseResult;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_findEdit->valuePtr())
		{
			if(isMandatory())
				return Result(Invalid, Wt::WString::tr("InvalidAccountSelectionMandatory"));
			else
				return Result(Invalid, Wt::WString::tr("InvalidAccountSelection"));
		}

		std::string requiredPrefixStr = Wt::WString::tr("FindAccountEditValuePrefix").arg(_findEdit->valuePtr()->name).arg(_findEdit->valuePtr().id()).toUTF8();
		if(_findEdit->lineEdit()->valueText().toUTF8().compare(0, requiredPrefixStr.length(), requiredPrefixStr) != 0)
		{
			if(isMandatory())
				return Result(Invalid, Wt::WString::tr("InvalidAccountSelectionMandatory"));
			else
				return Result(Invalid, Wt::WString::tr("InvalidAccountSelection"));
		}

		if(_findEdit->valuePtr())
		{
			if(app->authLogin().checkRecordViewPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Invalid, Wt::WString::tr("InvalidXSelectionPermissionDenied").arg(Wt::WString::tr("account")));

			if(_modifyPermissionRequired && app->authLogin().checkRecordModifyPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Invalid, Wt::WString::tr("InvalidXSelectionPermissionDenied").arg(Wt::WString::tr("account")));
		}

		return baseResult;
	}

	FindLocationEdit::FindLocationEdit()
		: FindRecordEdit()
	{
		_lineEdit->setPlaceholderText(tr("FindLocationEditPlaceholder"));

		auto newLocation = new Wt::WPushButton(tr("AddNewX").arg(tr("location")));
		newLocation->clicked().connect(this, &FindLocationEdit::showNewLocationDialog);
		_containerTemplate->bindWidget("new", newLocation);
		auto showList = new Wt::WPushButton(tr("SelectFromList"));
		showList->clicked().connect(this, &FindLocationEdit::showLocationListDialog);
		_containerTemplate->bindWidget("list", showList);

		WApplication *app = WApplication::instance();
		app->initFindLocationSuggestion();
		_suggestionPopup = app->findLocationSuggestion();
		_suggestionPopup->activated().connect(this, &FindLocationEdit::handleActivated);
	}

	void FindLocationEdit::setValuePtr(Wt::Dbo::ptr<Location> ptr)
	{
		TRANSACTION(APP);
		if(ptr)
			_lineEdit->setText(ptr->address);
		else
			_lineEdit->setText("");

		FindRecordEdit::setValuePtr(ptr);
	}

	void FindLocationEdit::showNewLocationDialog()
	{
		_newDialog = new Wt::WDialog(tr("AddNewX").arg(tr("location")), this);
		_newDialog->setClosable(true);
		_newDialog->resize(900, Wt::WLength(95, Wt::WLength::Percentage));
		_newDialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);
		_newDialog->setDeleteWhenHidden(true);

		LocationView *view = new LocationView();
		_newDialog->contents()->addWidget(view);
		view->submitted().connect(boost::bind(&FindLocationEdit::handleLocationViewSubmitted, this, view));

		_newDialog->show();
	}

	void FindLocationEdit::handleLocationViewSubmitted(LocationView *view)
	{
		setValuePtr(view->locationPtr());
		_newDialog->accept();
	}

	void FindLocationEdit::showLocationListDialog()
	{
		if(!_listDialog)
		{
			_listDialog = new Wt::WDialog(tr("SelectXFromList").arg(tr("location")), this);
			_listDialog->setClosable(true);
			_listDialog->resize(900, Wt::WLength(95, Wt::WLength::Percentage));
			_listDialog->setTransient(true);
			_listDialog->rejectWhenEscapePressed(true);
			_listDialog->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);

			AbstractFilteredList *listWidget = new LocationList();
			_listDialog->contents()->addWidget(listWidget);
			listWidget->enableFilters();
			listWidget->tableView()->setSelectionMode(Wt::SingleSelection);
			listWidget->tableView()->setSelectionBehavior(Wt::SelectRows);
			listWidget->tableView()->selectionChanged().connect(boost::bind(&FindLocationEdit::handleListSelectionChanged, this, listWidget));
		}
		else
		{
			if(AbstractFilteredList *listWidget = dynamic_cast<AbstractFilteredList*>(_listDialog->contents()->widget(0)))
				listWidget->reload();
		}
		_listDialog->show();
	}

	void FindLocationEdit::handleListSelectionChanged(AbstractFilteredList *listWidget)
	{
		auto indexSet = listWidget->tableView()->selectedIndexes();
		if(indexSet.empty())
			return;

		const auto &index = *indexSet.begin();
		if(!index.isValid())
			return;

		long long id = boost::any_cast<long long>(index.data());
		TRANSACTION(APP);
		Wt::Dbo::ptr<Location> ptr = APP->dboSession().load<Location>(id);
		setValuePtr(ptr);
		_listDialog->accept();
	}

	Wt::WValidator::Result FindLocationValidator::validate(const Wt::WString &input) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->lineEdit()->valueText());
		if(baseResult.state() != Valid)
			return baseResult;

		if(_findEdit->lineEdit()->valueText().empty())
			return baseResult;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_findEdit->valuePtr() ||
			_findEdit->lineEdit()->valueText().toUTF8().compare(0, _findEdit->valuePtr()->address.length(), _findEdit->valuePtr()->address) != 0)
		{
			if(isMandatory())
				return Result(Invalid, Wt::WString::tr("InvalidLocationSelectionMandatory"));
			else
				return Result(Invalid, Wt::WString::tr("InvalidLocationSelection"));
		}

		if(_findEdit->valuePtr())
		{
			if(app->authLogin().checkRecordViewPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Invalid, Wt::WString::tr("InvalidXSelectionPermissionDenied").arg(Wt::WString::tr("location")));

			if(_modifyPermissionRequired && app->authLogin().checkRecordModifyPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Invalid, Wt::WString::tr("InvalidXSelectionPermissionDenied").arg(Wt::WString::tr("location")));
		}

		return baseResult;
	}

	FindEntitySuggestionPopup::FindEntitySuggestionPopup(Wt::WObject *parent /*= nullptr*/)
		: Wt::WSuggestionPopup({
					   "<b><u>",	//highlightBeginTag
					   "</u></b>",	//highlightEndTag
					   '\0',		//listSeparator(no)
					   " ",		//whitespace
					   " "		//wordSeparators
			}, parent)
	{
		setEditRole(Wt::DisplayRole);
		setMaximumSize(Wt::WLength(), 300);
		setFilterLength(3);
		filterModel().connect(this, &FindEntitySuggestionPopup::handleFilterModel);
	}

	void FindEntitySuggestionPopup::handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit)
	{
		bool modifyPermissionRequired = false;
		if(FindEntityValidator *validator = dynamic_cast<FindEntityValidator*>(edit->validator()))
			modifyPermissionRequired = validator->modifyPermissionRequired();

		Entity::Type typeFilter = Entity::InvalidType;
		Entity::SpecificType specificTypeFilter = Entity::UnspecificType;
		if(edit->parent() && edit->parent()->parent())
		{
			if(FindEntityEdit *findEdit = dynamic_cast<FindEntityEdit*>(edit->parent()->parent()))
			{
				typeFilter = findEdit->entityType();
				specificTypeFilter = findEdit->specificType();
			}
		}

		Wt::WStringListModel *stringList = dynamic_cast<Wt::WStringListModel*>(model());
		stringList->removeRows(0, stringList->rowCount());

		WApplication *app = APP;
		typedef boost::tuple<std::string, long long> FEMTuple;
		auto query = app->dboSession().query<FEMTuple>("SELECT name, id FROM " + std::string(Entity::tableName())).limit(50);
		auto countQuery = app->dboSession().query<int>("SELECT COUNT(1) FROM " + std::string(Entity::tableName()));

		app->authLogin().setPermissionConditionsToQuery(query, false, "", modifyPermissionRequired);
		app->authLogin().setPermissionConditionsToQuery(countQuery, false, "", modifyPermissionRequired);

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
		if(typeFilter != Entity::InvalidType)
		{
			query.where("type = ?").bind(typeFilter);
			countQuery.where("type = ?").bind(typeFilter);
		}
		if(specificTypeFilter != Entity::UnspecificType)
		{
			query.where("(specificTypeMask & ?) = ?").bind(specificTypeFilter).bind(specificTypeFilter);
			countQuery.where("(specificTypeMask & ?) = ?").bind(specificTypeFilter).bind(specificTypeFilter);
		}

		TRANSACTION(app);
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
		setEditRole(Wt::DisplayRole);
		setMaximumSize(Wt::WLength(), 300);
		setFilterLength(3);
		filterModel().connect(this, &FindAccountSuggestionPopup::handleFilterModel);
	}

	void FindAccountSuggestionPopup::handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit)
	{
		bool modifyPermissionRequired = false;
		if(FindAccountValidator *validator = dynamic_cast<FindAccountValidator*>(edit->validator()))
			modifyPermissionRequired = validator->modifyPermissionRequired();

		Wt::WStringListModel *stringList = dynamic_cast<Wt::WStringListModel*>(model());
		stringList->removeRows(0, stringList->rowCount());

		WApplication *app = APP;
		typedef boost::tuple<std::string, long long, Account::Type, std::string> FAMTuple;
		std::string joinPart = "LEFT JOIN " + std::string(Entity::tableName()) + " e ON (e.bal_account_id = acc.id OR e.pnl_account_id = acc.id)";
		auto query = app->dboSession().query<FAMTuple>("SELECT acc.name, acc.id, acc.type, e.name e_name FROM " + std::string(Account::tableName()) + " acc " + joinPart).limit(50);
		auto countQuery = app->dboSession().query<int>("SELECT COUNT(acc.id) FROM " + std::string(Account::tableName()) + " acc " + joinPart);

		app->authLogin().setPermissionConditionsToQuery(query, false, "acc.", modifyPermissionRequired);
		app->authLogin().setPermissionConditionsToQuery(countQuery, false, "acc.", modifyPermissionRequired);

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

		TRANSACTION(app);
		int count = countQuery;
		Wt::Dbo::collection<FAMTuple> result = query;

		for(const FAMTuple &row : result)
		{
			if(!boost::get<EntityName>(row).empty())
				stringList->addString(tr("FindAccountEditValueWithEntityTemplate")
					.arg(tr("FindAccountEditValuePrefix").arg(boost::get<Name>(row)).arg(boost::get<Id>(row)))
					.arg(boost::get<EntityName>(row)));
			else
				stringList->addString(tr("FindAccountEditValueTemplate")
					.arg(tr("FindAccountEditValuePrefix").arg(boost::get<Name>(row)).arg(boost::get<Id>(row)))
					.arg(Wt::boost_any_traits<Account::Type>::asString(boost::get<Type>(row), "")));

			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), boost::get<Id>(row), Wt::UserRole);
		}

		if(stringList->rowCount() < count && stringList->rowCount() != 0)
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), "Wt-more-data", Wt::StyleClassRole);
	}

	FindLocationSuggestionPopup::FindLocationSuggestionPopup(Wt::WObject *parent /*= nullptr*/)
		: Wt::WSuggestionPopup({
					   "<b><u>",	//highlightBeginTag
					   "</u></b>",	//highlightEndTag
					   '\0',		//listSeparator(no)
					   " ",		//whitespace
					   " "		//wordSeparators
			}, parent)
	{
		setEditRole(Wt::DisplayRole);
		setMaximumSize(600, 300);
		setFilterLength(3);
		filterModel().connect(this, &FindLocationSuggestionPopup::handleFilterModel);
	}

	void FindLocationSuggestionPopup::handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit)
	{
		bool modifyPermissionRequired = false;
		if(FindLocationValidator *validator = dynamic_cast<FindLocationValidator*>(edit->validator()))
			modifyPermissionRequired = validator->modifyPermissionRequired();

		Wt::WStringListModel *stringList = dynamic_cast<Wt::WStringListModel*>(model());
		stringList->removeRows(0, stringList->rowCount());

		WApplication *app = APP;
		std::string joinPart = 
			"LEFT JOIN " + std::string(Country::tableName()) + " cnt ON cnt.code = l.country_code "
			"LEFT JOIN " + std::string(City::tableName()) + " city ON city.id = l.city_id "
			"LEFT JOIN " + std::string(Entity::tableName()) + " e ON e.id = l.entity_id";
		auto query = app->dboSession().query<ResultTuple>("SELECT l.id, l.address, cnt.name cnt_name, city.name city_name, e.name e_name FROM " + std::string(Location::tableName()) + " l " + joinPart).limit(50);
		auto countQuery = app->dboSession().query<int>("SELECT COUNT(l.id) FROM " + std::string(Location::tableName()) + " l " + joinPart);

		app->authLogin().setPermissionConditionsToQuery(query, false, "l.", modifyPermissionRequired);
		app->authLogin().setPermissionConditionsToQuery(countQuery, false, "l.", modifyPermissionRequired);

		if(!str.empty())
		{
			std::string condition;
			std::string stlStr = str.toUTF8();
			boost::tokenizer<> tokens(stlStr);
			for(const auto &token : tokens)
			{
				if(!condition.empty())
					condition += " OR ";
				condition += "l.address LIKE ? OR e.name LIKE ? OR cnt.name LIKE ? OR city.name LIKE ?";
				query.bind("%" + token + "%").bind("%" + token + "%").bind("%" + token + "%").bind("%" + token + "%");
				countQuery.bind("%" + token + "%").bind("%" + token + "%").bind("%" + token + "%").bind("%" + token + "%");

				if(is_number(token))
				{
					condition += " OR e.id LIKE ?";
					query.bind("%" + token + "%");
					countQuery.bind("%" + token + "%");
				}
			}

			query.where(condition);
			countQuery.where(condition);
		}

		TRANSACTION(app);
		int count = countQuery;
		Wt::Dbo::collection<ResultTuple> result = query;

		for(const ResultTuple &row : result)
		{
			Wt::WString country = boost::get<CountryName>(row).empty() ? Wt::WString::tr("Unknown") : boost::get<CountryName>(row);
			Wt::WString city = boost::get<CityName>(row).empty() ? Wt::WString::tr("Unknown") : boost::get<CityName>(row);

			if(!boost::get<EntityName>(row).empty())
				stringList->addString(tr("FindLocationEditValueTemplate")
					.arg(boost::get<Address>(row)).arg(country).arg(city));
			else
				stringList->addString(tr("FindLocationEditValueWithEntityTemplate")
					.arg(boost::get<Address>(row)).arg(country).arg(city).arg(boost::get<EntityName>(row)));

			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), boost::get<Id>(row), Wt::UserRole);
		}

		if(stringList->rowCount() < count && stringList->rowCount() != 0)
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), "Wt-more-data", Wt::StyleClassRole);
	}

	//EXPLICIT SPECIALIZATIONS
	template class FindRecordEdit<Entity>;
	template class FindRecordEdit<Account>;
	template class FindRecordEdit<Location>;

}
