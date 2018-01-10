#include <cctype>

#include "Widgets/FindRecordEdit.h"
#include "Widgets/EntityList.h"
#include "Widgets/EntityView.h"
#include "Widgets/AccountMVC.h"
#include "Widgets/LocationMVC.h"
#include "Utilities/FilteredList.h"

#include <Wt/WPushButton.h>
#include <Wt/WDialog.h>
#include <Wt/WTableView.h>
#include <Wt/WStringListModel.h>

#include <boost/tokenizer.hpp>

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

	FindRecordEditTemplate::FindRecordEditTemplate(std::unique_ptr<Wt::WLineEdit> edit)
		: Wt::WTemplate(tr("GS.FindRecordEdit")), _edit(edit.get())
	{
		bindWidget("edit", std::move(edit));
		setReadOnly(false);
	}

	AbstractFindRecordEdit::AbstractFindRecordEdit()
	{
		auto edit = std::make_unique<Wt::WLineEdit>();
		_lineEdit = edit.get();
		setFormWidgetImpl(_lineEdit);

		_containerTemplate = setImplementation(std::make_unique<FindRecordEditTemplate>(std::move(edit)));

		_lineEdit->changed().connect(this, &AbstractFindRecordEdit::handleLineEditChanged);
		valueChanged().connect(_lineEdit, std::bind(&Wt::WLineEdit::validate, _lineEdit));
	}

	void AbstractFindRecordEdit::propagateSetEnabled(bool enabled)
	{
		if(enabled)
			_suggestionPopup->forEdit(_lineEdit, Wt::PopupTrigger::Editing | Wt::PopupTrigger::DropDownIcon);
		else
		{
			_suggestionPopup->removeEdit(_lineEdit);
			_lineEdit->setPlaceholderText("");
		}

		Wt::WCompositeWidget::propagateSetEnabled(enabled);
	}

	void AbstractFindRecordEdit::load()
	{
		bool ld = !loaded();
		Wt::WCompositeWidget::load();

		if(ld)
		{
			if(isEnabled())
				_suggestionPopup->forEdit(_lineEdit, Wt::PopupTrigger::Editing | Wt::PopupTrigger::DropDownIcon);
		}
	}

	void AbstractFindRecordEdit::handleLineEditChanged()
	{
		if(_lineEdit->text().empty())
			setValuePtr(Wt::any());
		else
			setTextFromValuePtr();
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
				setValuePtr(app->dboSession().load<Value>(Wt::any_cast<long long>(itemIndex.data(Wt::ItemDataRole::User))));
			}
			catch(const Wt::Dbo::Exception &e)
			{
				Wt::log("error") << "FindRecordEdit::handleActivated(): Dbo error(" << e.code() << "): " << e.what();
				setValuePtr(Wt::Dbo::ptr<Value>());
				app->showDbBackendError(e.code());
			}
		}
		else
			setValuePtr(Wt::Dbo::ptr<Value>());
	}

	template<class Value>
	void FindRecordEdit<Value>::setValuePtr(Wt::Dbo::ptr<Value> ptr)
	{
		if(_valuePtr != ptr)
		{
			_valuePtr = ptr;
			_valueChanged.emit();
		}
		setTextFromValuePtr();
	}

	FindEntityEdit::FindEntityEdit(Entity::Type entityType, Entity::SpecificType specificType /*= Entity::UnspecificType*/)
		: _entityType(entityType), _specificType(specificType)
	{
		_lineEdit->setPlaceholderText(tr("FindEntityEditPlaceholder"));

		auto newEntity = std::make_unique<Wt::WPushButton>();
		newEntity->clicked().connect(this, &FindEntityEdit::showNewEntityDialog);
		switch(_entityType)
		{
		case Entity::PersonType: newEntity->setText(tr("AddNewX").arg(tr("person"))); break;
		case Entity::BusinessType: newEntity->setText(tr("AddNewX").arg(tr("business"))); break;
		default: newEntity->setText(tr("AddNewX").arg(tr("entity"))); break;
		}
		_containerTemplate->bindWidget("new", std::move(newEntity));

		auto showList = std::make_unique<Wt::WPushButton>(tr("SelectFromList"));
		showList->clicked().connect(this, &FindEntityEdit::showEntityListDialog);
		_containerTemplate->bindWidget("list", std::move(showList));

		WApplication *app = WApplication::instance();
		app->initFindEntitySuggestion();
		_suggestionPopup = app->findEntitySuggestion();
		_suggestionPopup->activated().connect(this, &FindEntityEdit::handleActivated);
	}

	void FindEntityEdit::showNewEntityDialog()
	{
		if(_newDialog)
			_newDialog->removeFromParent();

		_newDialog = addChild(std::make_unique<Wt::WDialog>());
		_newDialog->setClosable(true);
		_newDialog->resize(Wt::WLength(85, Wt::LengthUnit::Percentage), Wt::WLength(95, Wt::LengthUnit::Percentage));
		_newDialog->setTransient(true);
		_newDialog->contents()->setOverflow(Wt::Overflow::Auto);

		if(_entityType == Entity::PersonType)
			_newDialog->setWindowTitle(tr("AddNewX").arg(tr("person")));
		else if(_entityType == Entity::BusinessType)
			_newDialog->setWindowTitle(tr("AddNewX").arg(tr("business")));
		else
			_newDialog->setWindowTitle(tr("AddNewX").arg(tr("entity")));

		EntityView *newEntityView = _newDialog->contents()->addNew<EntityView>(_entityType);
		newEntityView->setSpecificType(_specificType);
		newEntityView->submitted().connect(this, std::bind(&FindEntityEdit::handleEntityViewSubmitted, this, newEntityView));

		_newDialog->show();
	}

	void FindEntityEdit::showEntityListDialog()
	{
		if(_listDialog)
			_listDialog->removeFromParent();

		std::unique_ptr<Wt::WDialog> ld;
		if(_entityType == Entity::PersonType)
		{
			auto selectionDialog = std::make_unique<ListSelectionDialog<PersonList>>(tr("SelectXFromList").arg(tr("person")));
			selectionDialog->selected().connect(this, &FindEntityEdit::handleListSelectionChanged);
			ld = std::move(selectionDialog);
		}
		else if(_entityType == Entity::BusinessType)
		{
			auto selectionDialog = std::make_unique<ListSelectionDialog<BusinessList>>(tr("SelectXFromList").arg(tr("business")));
			selectionDialog->selected().connect(this, &FindEntityEdit::handleListSelectionChanged);
			ld = std::move(selectionDialog);
		}
		else
		{
			auto selectionDialog = std::make_unique<ListSelectionDialog<AllEntityList>>(tr("SelectXFromList").arg(tr("entity")));
			selectionDialog->selected().connect(this, &FindEntityEdit::handleListSelectionChanged);
			ld = std::move(selectionDialog);
		}
		_listDialog = ld.get();
		addChild(std::move(ld));
		_listDialog->show();
	}

	void FindEntityEdit::handleListSelectionChanged(long long id)
	{
		WApplication *app = APP;
		try
		{
			TRANSACTION(app);
			Wt::Dbo::ptr<Entity> entityPtr = app->dboSession().load<Entity>(id);
			setValuePtr(entityPtr);
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "FindEntityEdit::handleListSelectionChanged(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	void FindEntityEdit::handleEntityViewSubmitted(EntityView *view)
	{
		setValuePtr(view->entityPtr());
		_newDialog->accept();
	}

	void FindEntityEdit::setTextFromValuePtr()
	{
		TRANSACTION(APP);
		if(valuePtr())
			_lineEdit->setText(tr("FindEntityEditValueTemplate").arg(valuePtr()->name).arg(valuePtr().id()));
		else
			_lineEdit->setText("");
	}

	Wt::WValidator::Result FindEntityValidator::validate(const Wt::WString &) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->lineEdit()->valueText());
		if(baseResult.state() != Wt::ValidationState::Valid)
			return baseResult;

		if(_findEdit->lineEdit()->valueText().empty())
			return baseResult;
		
		WApplication *app = APP;
		TRANSACTION(app);

		//Check selected entity, its type and line edit text
		if(!_findEdit->valuePtr() 
			|| (_findEdit->_entityType != Entity::InvalidType && _findEdit->valuePtr()->type != _findEdit->_entityType) 
			|| _findEdit->lineEdit()->valueText() != tr("FindEntityEditValueTemplate").arg(_findEdit->valuePtr()->name).arg(_findEdit->valuePtr().id()))
		{
			if(_findEdit->_entityType == Entity::PersonType)
			{
				if(isMandatory())
					return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionMandatory").arg(tr("person")));
				else
					return Result(Wt::ValidationState::Invalid, tr("InvalidXSelection").arg(tr("person")));
			}
			else if(_findEdit->_entityType == Entity::BusinessType)
			{
				if(isMandatory())
					return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionMandatory").arg(tr("business")));
				else
					return Result(Wt::ValidationState::Invalid, tr("InvalidXSelection").arg(tr("business")));
			}
			else
			{
				if(isMandatory())
					return Result(Wt::ValidationState::Invalid, tr("InvalidVSelectionMandatory").arg(tr("entity")));
				else
					return Result(Wt::ValidationState::Invalid, tr("InvalidVSelection").arg(tr("entity")));
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
						return Result(Wt::ValidationState::Invalid, tr("InvalidVSelectionMandatory").arg(tr("employee")));
					else
						return Result(Wt::ValidationState::Invalid, tr("InvalidVSelection").arg(tr("employee")));
				}
				else if(_findEdit->_entityType == Entity::PersonnelType)
				{
					if(isMandatory())
						return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionMandatory").arg(tr("personnel")));
					else
						return Result(Wt::ValidationState::Invalid, tr("InvalidXSelection").arg(tr("personnel")));
				}
				else if(_findEdit->_entityType == Entity::ClientType)
				{
					if(isMandatory())
						return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionMandatory").arg(tr("client")));
					else
						return Result(Wt::ValidationState::Invalid, tr("InvalidXSelection").arg(tr("client")));
				}
				else
					return Result(Wt::ValidationState::Invalid, tr("InvalidSelectionSpecificType"));
			}

			//Check permission
			if(app->authLogin().checkRecordViewPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionPermissionDenied").arg(tr("entity")));

			if(_modifyPermissionRequired && app->authLogin().checkRecordModifyPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionPermissionDenied").arg(tr("entity")));
		}

		return baseResult;
	}

	FindAccountEdit::FindAccountEdit()
	{
		_lineEdit->setPlaceholderText(tr("FindAccountEditPlaceholder"));

		auto newAccount = _containerTemplate->bindNew<Wt::WPushButton>("new", tr("AddNewX").arg(tr("account")));
		newAccount->clicked().connect(this, &FindAccountEdit::showNewAccountDialog);

		auto showList = _containerTemplate->bindNew<Wt::WPushButton>("list", tr("SelectFromList"));
		showList->clicked().connect(this, &FindAccountEdit::showAccountListDialog);

		WApplication *app = WApplication::instance();
		app->initFindAccountSuggestion();
		_suggestionPopup = app->findAccountSuggestion();
		_suggestionPopup->activated().connect(this, &FindAccountEdit::handleActivated);
	}

	void FindAccountEdit::showNewAccountDialog()
	{
		if(_newDialog)
			_newDialog->removeFromParent();

		_newDialog = addChild(std::make_unique<Wt::WDialog>(tr("AddNewX").arg(tr("account"))));
		_newDialog->setClosable(true);
		_newDialog->resize(Wt::WLength(85, Wt::LengthUnit::Percentage), Wt::WLength(95, Wt::LengthUnit::Percentage));
		_newDialog->setTransient(true);
		_newDialog->contents()->setOverflow(Wt::Overflow::Auto);

		AccountView *view = _newDialog->contents()->addNew<AccountView>();
		view->submitted().connect(this, std::bind(&FindAccountEdit::handleAccountViewSubmitted, this, view));

		_newDialog->show();
	}

	void FindAccountEdit::handleAccountViewSubmitted(AccountView *view)
	{
		setValuePtr(view->accountPtr());
		_newDialog->accept();
	}

	void FindAccountEdit::showAccountListDialog()
	{
		if(_listDialog)
			_listDialog->removeFromParent();

		auto selectionDialog = std::make_unique<ListSelectionDialog<AccountList>>(tr("SelectXFromList").arg(tr("account")));
		selectionDialog->selected().connect(this, &FindAccountEdit::handleListSelectionChanged);
		
		_listDialog = selectionDialog.get();
		addChild(std::move(selectionDialog));
		_listDialog->show();
	}

	void FindAccountEdit::handleListSelectionChanged(long long id)
	{
		WApplication *app = APP;
		try
		{
			TRANSACTION(app);
			Wt::Dbo::ptr<Account> accountPtr = APP->dboSession().load<Account>(id);
			setValuePtr(accountPtr);
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "FindAccountEdit::handleListSelectionChanged(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	void FindAccountEdit::setTextFromValuePtr()
	{
		TRANSACTION(APP);
		if(valuePtr())
			_lineEdit->setText(tr("FindAccountEditValueTemplate")
				.arg(tr("FindAccountEditValuePrefix").arg(valuePtr()->name).arg(valuePtr().id()))
				.arg(Wt::any_traits<Account::Type>::asString(valuePtr()->type, "")));
		else
			_lineEdit->setText("");
	}

	Wt::WValidator::Result FindAccountValidator::validate(const Wt::WString &) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->lineEdit()->valueText());
		if(baseResult.state() != Wt::ValidationState::Valid)
			return baseResult;

		if(_findEdit->lineEdit()->valueText().empty())
			return baseResult;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_findEdit->valuePtr())
		{
			if(isMandatory())
				return Result(Wt::ValidationState::Invalid, tr("InvalidVSelectionMandatory").arg(tr("account")));
			else
				return Result(Wt::ValidationState::Invalid, tr("InvalidVSelection").arg(tr("account")));
		}

		std::string requiredPrefixStr = tr("FindAccountEditValuePrefix").arg(_findEdit->valuePtr()->name).arg(_findEdit->valuePtr().id()).toUTF8();
		if(_findEdit->lineEdit()->valueText().toUTF8().compare(0, requiredPrefixStr.length(), requiredPrefixStr) != 0)
		{
			if(isMandatory())
				return Result(Wt::ValidationState::Invalid, tr("InvalidVSelectionMandatory").arg(tr("account")));
			else
				return Result(Wt::ValidationState::Invalid, tr("InvalidVSelection").arg(tr("account")));
		}

		if(_findEdit->valuePtr())
		{
			if(app->authLogin().checkRecordViewPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionPermissionDenied").arg(tr("account")));

			if(_modifyPermissionRequired && app->authLogin().checkRecordModifyPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionPermissionDenied").arg(tr("account")));
		}

		return baseResult;
	}

	FindLocationEdit::FindLocationEdit()
	{
		_lineEdit->setPlaceholderText(tr("FindLocationEditPlaceholder"));

		auto newLocation = _containerTemplate->bindNew<Wt::WPushButton>("new", tr("AddNewX").arg(tr("location")));
		newLocation->clicked().connect(this, &FindLocationEdit::showNewLocationDialog);
		
		auto showList = _containerTemplate->bindNew<Wt::WPushButton>("list", tr("SelectFromList"));
		showList->clicked().connect(this, &FindLocationEdit::showLocationListDialog);

		WApplication *app = WApplication::instance();
		app->initFindLocationSuggestion();
		_suggestionPopup = app->findLocationSuggestion();
		_suggestionPopup->activated().connect(this, &FindLocationEdit::handleActivated);
	}

	void FindLocationEdit::setTextFromValuePtr()
	{
		TRANSACTION(APP);
		if(valuePtr())
			_lineEdit->setText(valuePtr()->address);
		else
			_lineEdit->setText("");
	}

	void FindLocationEdit::showNewLocationDialog()
	{
		if(_newDialog)
			_newDialog->removeFromParent();

		_newDialog = addChild(std::make_unique<Wt::WDialog>(tr("AddNewX").arg(tr("location"))));
		_newDialog->setClosable(true);
		_newDialog->resize(Wt::WLength(85, Wt::LengthUnit::Percentage), Wt::WLength(95, Wt::LengthUnit::Percentage));
		_newDialog->setTransient(true);
		_newDialog->contents()->setOverflow(Wt::Overflow::Auto);

		LocationView *view = _newDialog->contents()->addNew<LocationView>();
		view->submitted().connect(this, std::bind(&FindLocationEdit::handleLocationViewSubmitted, this, view));

		_newDialog->show();
	}

	void FindLocationEdit::handleLocationViewSubmitted(LocationView *view)
	{
		setValuePtr(view->locationPtr());
		_newDialog->accept();
	}

	void FindLocationEdit::showLocationListDialog()
	{
		if(_listDialog)
			_listDialog->removeFromParent();

		auto selectionDialog = std::make_unique<ListSelectionDialog<LocationList>>(tr("SelectXFromList").arg(tr("location")));
		selectionDialog->selected().connect(this, &FindLocationEdit::handleListSelectionChanged);

		_listDialog = selectionDialog.get();
		addChild(std::move(selectionDialog));
		_listDialog->show();
	}

	void FindLocationEdit::handleListSelectionChanged(long long id)
	{
		WApplication *app = APP;
		try
		{
			TRANSACTION(app);
			Wt::Dbo::ptr<Location> ptr = app->dboSession().load<Location>(id);
			setValuePtr(ptr);
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "FindLocationEdit::handleListSelectionChanged(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	Wt::WValidator::Result FindLocationValidator::validate(const Wt::WString &input) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->lineEdit()->valueText());
		if(baseResult.state() != Wt::ValidationState::Valid)
			return baseResult;

		if(_findEdit->lineEdit()->valueText().empty())
			return baseResult;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_findEdit->valuePtr() ||
			_findEdit->lineEdit()->valueText().toUTF8().compare(0, _findEdit->valuePtr()->address.length(), _findEdit->valuePtr()->address) != 0)
		{
			if(isMandatory())
				return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionMandatory").arg(tr("location")));
			else
				return Result(Wt::ValidationState::Invalid, tr("InvalidXSelection").arg(tr("location")));
		}

		if(_findEdit->valuePtr())
		{
			if(app->authLogin().checkRecordViewPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionPermissionDenied").arg(tr("location")));

			if(_modifyPermissionRequired && app->authLogin().checkRecordModifyPermission(_findEdit->valuePtr().get()) != AuthLogin::Permitted)
				return Result(Wt::ValidationState::Invalid, tr("InvalidXSelectionPermissionDenied").arg(tr("location")));
		}

		return baseResult;
	}

	FindEntitySuggestionPopup::FindEntitySuggestionPopup()
		: Wt::WSuggestionPopup({
					   "<b><u>",	//highlightBeginTag
					   "</u></b>",	//highlightEndTag
					   '\0',		//listSeparator(no)
					   " ",		//whitespace
					   " "		//wordSeparators
			})
	{
		setEditRole(Wt::ItemDataRole::Display);
		setMaximumSize(Wt::WLength(), 300);
		setFilterLength(3);
		filterModel().connect(this, &FindEntitySuggestionPopup::handleFilterModel);
	}

	void FindEntitySuggestionPopup::handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit)
	{
		bool modifyPermissionRequired = false;
		if(auto validator = std::dynamic_pointer_cast<FindEntityValidator>(edit->validator()))
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

		auto stringList = std::static_pointer_cast<Wt::WStringListModel>(model());
		stringList->removeRows(0, stringList->rowCount());

		WApplication *app = APP;
		typedef std::tuple<std::string, long long> FEMTuple;
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
			stringList->addString(tr("FindEntityEditValueTemplate").arg(std::get<0>(row)).arg(std::get<1>(row)));
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), std::get<1>(row), Wt::ItemDataRole::User);
		}

		if(stringList->rowCount() < count && stringList->rowCount() != 0)
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), "Wt-more-data", Wt::ItemDataRole::StyleClass);
	}

	FindAccountSuggestionPopup::FindAccountSuggestionPopup()
		: Wt::WSuggestionPopup({
					   "<b><u>",	//highlightBeginTag
					   "</u></b>",	//highlightEndTag
					   '\0',		//listSeparator(no)
					   " ",		//whitespace
					   " "		//wordSeparators
			})
	{
		setEditRole(Wt::ItemDataRole::Display);
		setMaximumSize(Wt::WLength(), 300);
		setFilterLength(3);
		filterModel().connect(this, &FindAccountSuggestionPopup::handleFilterModel);
	}

	void FindAccountSuggestionPopup::handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit)
	{
		bool modifyPermissionRequired = false;
		if(auto validator = std::dynamic_pointer_cast<FindAccountValidator>(edit->validator()))
			modifyPermissionRequired = validator->modifyPermissionRequired();

		auto stringList = std::static_pointer_cast<Wt::WStringListModel>(model());
		stringList->removeRows(0, stringList->rowCount());

		WApplication *app = APP;
		typedef std::tuple<std::string, long long, Account::Type, std::string> FAMTuple;
		std::string joinPart = "LEFT JOIN " + std::string(Entity::tableName()) + " e ON (e.bal_account_id = acc.id OR e.pnl_account_id = acc.id)";
		auto query = app->dboSession().query<FAMTuple>("SELECT acc.name, acc.id, acc.type, e.name FROM " + std::string(Account::tableName()) + " acc " + joinPart).limit(50);
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
			if(!std::get<EntityName>(row).empty())
				stringList->addString(tr("FindAccountEditValueWithEntityTemplate")
					.arg(tr("FindAccountEditValuePrefix").arg(std::get<Name>(row)).arg(std::get<Id>(row)))
					.arg(std::get<EntityName>(row)));
			else
				stringList->addString(tr("FindAccountEditValueTemplate")
					.arg(tr("FindAccountEditValuePrefix").arg(std::get<Name>(row)).arg(std::get<Id>(row)))
					.arg(Wt::any_traits<Account::Type>::asString(std::get<Type>(row), "")));

			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), std::get<Id>(row), Wt::ItemDataRole::User);
		}

		if(stringList->rowCount() < count && stringList->rowCount() != 0)
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), "Wt-more-data", Wt::ItemDataRole::StyleClass);
	}

	FindLocationSuggestionPopup::FindLocationSuggestionPopup()
		: Wt::WSuggestionPopup({
					   "<b><u>",	//highlightBeginTag
					   "</u></b>",	//highlightEndTag
					   '\0',		//listSeparator(no)
					   " ",		//whitespace
					   " "		//wordSeparators
			})
	{
		setEditRole(Wt::ItemDataRole::Display);
		setMaximumSize(600, 300);
		setFilterLength(3);
		filterModel().connect(this, &FindLocationSuggestionPopup::handleFilterModel);
	}

	void FindLocationSuggestionPopup::handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit)
	{
		bool modifyPermissionRequired = false;
		if(auto validator = std::dynamic_pointer_cast<FindLocationValidator>(edit->validator()))
			modifyPermissionRequired = validator->modifyPermissionRequired();

		auto stringList = std::static_pointer_cast<Wt::WStringListModel>(model());
		stringList->removeRows(0, stringList->rowCount());

		WApplication *app = APP;
		std::string joinPart = 
			"LEFT JOIN " + std::string(Country::tableName()) + " cnt ON cnt.code = l.country_code "
			"LEFT JOIN " + std::string(City::tableName()) + " city ON city.id = l.city_id "
			"LEFT JOIN " + std::string(Entity::tableName()) + " e ON e.id = l.entity_id";
		auto query = app->dboSession().query<ResultTuple>("SELECT l.id, l.address, cnt.name, city.name, e.name FROM " + std::string(Location::tableName()) + " l " + joinPart).limit(50);
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
			Wt::WString country = std::get<CountryName>(row).empty() ? tr("Unknown") : std::get<CountryName>(row);
			Wt::WString city = std::get<CityName>(row).empty() ? tr("Unknown") : std::get<CityName>(row);

			if(!std::get<EntityName>(row).empty())
				stringList->addString(tr("FindLocationEditValueTemplate")
					.arg(std::get<Address>(row)).arg(country).arg(city));
			else
				stringList->addString(tr("FindLocationEditValueWithEntityTemplate")
					.arg(std::get<Address>(row)).arg(country).arg(city).arg(std::get<EntityName>(row)));

			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), std::get<Id>(row), Wt::ItemDataRole::User);
		}

		if(stringList->rowCount() < count && stringList->rowCount() != 0)
			stringList->setData(stringList->index(stringList->rowCount() - 1, 0), "Wt-more-data", Wt::ItemDataRole::StyleClass);
	}

	//EXPLICIT SPECIALIZATIONS
	template class FindRecordEdit<Entity>;
	template class FindRecordEdit<Account>;
	template class FindRecordEdit<Location>;

}
