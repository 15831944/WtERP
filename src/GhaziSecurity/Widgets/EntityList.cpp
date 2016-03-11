#include "Widgets/EntityList.h"
#include "Widgets/EntityView.h"
#include "Utilities/FilteredList.h"

#include <Wt/WCheckBox>
#include <Wt/WComboBox>
#include <Wt/WTableView>
#include <Wt/WPushButton>
#include <Wt/WIntValidator>
#include <Wt/WSuggestionPopup>

namespace GS
{

	boost::any FindEntityModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(idx.column() == Name && role == Wt::DisplayRole)
		{
			const auto &res = resultRow(idx.row());
			return Wt::WString::tr("FindEntityEditValueTemplate").arg(boost::get<Name>(res)).arg(boost::get<Id>(res));
		}
		return Wt::Dbo::QueryModel<_FEMTuple>::data(idx, role);
	}

	FindEntityEdit::FindEntityEdit(Entity::Type entityType, Wt::WContainerWidget *parent)
		: Wt::WLineEdit(parent), _entityType(entityType)
	{
		setPlaceholderText(tr("FindEntityEditPlaceholder"));
		entityChanged().connect(boost::bind(&FindEntityEdit::validate, this));

		_newEntity = new Wt::WPushButton();
		//Wt::WObject::addChild(_newEntity);
		_newEntity->clicked().connect(this, &FindEntityEdit::showNewEntityDialog);
		switch(_entityType)
		{
		case Entity::PersonType: _newEntity->setText(tr("AddNewPerson")); break;
		case Entity::BusinessType: _newEntity->setText(tr("AddNewBusiness")); break;
		default: _newEntity->setText(tr("AddNewEntity")); break;
		}

		_showList = new Wt::WPushButton(tr("SelectFromList"));
		//Wt::WObject::addChild(_showList);
		_showList->clicked().connect(this, &FindEntityEdit::showEntityListDialog);

		WApplication *app = WApplication::instance();
		app->initFindEntityModel();

		Wt::WSuggestionPopup::Options nameOptions = {
			"<b><u>",	//highlightBeginTag
			"</b></u>",	//highlightEndTag
			'\0',		//listSeparator(no)
			" ",		//whitespace
			" -"			//wordSeparators
		};
		_suggestionPopup = new Wt::WSuggestionPopup(nameOptions, this);
		if(_entityType == Entity::PersonType)
			_suggestionPopup->setModel(app->findPersonModel());
		else if(_entityType == Entity::BusinessType)
			_suggestionPopup->setModel(app->findBusinessModel());
		else
			_suggestionPopup->setModel(app->findEntityModel());

		_suggestionPopup->setMaximumSize(Wt::WLength(), 400);
		_suggestionPopup->forEdit(this, Wt::WSuggestionPopup::DropDownIcon | Wt::WSuggestionPopup::Editing);
		_suggestionPopup->activated().connect(this, &FindEntityEdit::handleActivated);
		_suggestionPopup->setDropDownIconUnfiltered(true);
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
		setEntityPtr(entityPtr);
		_listDialog->accept();
		t.commit();
	}

	void FindEntityEdit::handleEntityViewSubmitted(EntityView *view)
	{
		setEntityPtr(view->entityPtr());
		_newDialog->accept();
	}

	void FindEntityEdit::setEntityPtr(Wt::Dbo::ptr<Entity> ptr)
	{
		if(ptr)
			setText(tr("FindEntityEditValueTemplate").arg(ptr->name).arg(ptr.id()));
		else
			setText("");

		if(_entityPtr != ptr)
		{
			_entityPtr = ptr;
			_entityChanged.emit();
		}
	}

	void FindEntityEdit::handleActivated(int index, Wt::WFormWidget *lineEdit)
	{
		WApplication *app = WApplication::instance();
		auto itemIndex = _suggestionPopup->model()->index(index, FindEntityModel::Id);

		try
		{
			if(itemIndex.isValid())
			{
				Wt::Dbo::Transaction t(app->session());
				setEntityPtr(app->session().load<Entity>(boost::any_cast<long long>(itemIndex.data())));
				t.commit();
			}
			else
				setEntityPtr(Wt::Dbo::ptr<Entity>());
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "FindEntityEdit::handleActivated(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	Wt::WValidator::Result FindEntityValidator::validate(const Wt::WString &) const
	{
		Result baseResult = Wt::WValidator::validate(_findEdit->valueText());
		if(baseResult.state() != Valid)
			return baseResult;

		if(_findEdit->valueText().empty())
			return baseResult;

		
		int currentIndex = _findEdit->_suggestionPopup->currentItem();
		if(!_findEdit->entityPtr() ||
			_findEdit->valueText() != Wt::WString::tr("FindEntityEditValueTemplate").arg(_findEdit->entityPtr()->name).arg(_findEdit->entityPtr().id()))
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

#define IdColumnWidth 80
#define NameColumnWidth 250
#define TypeColumnWidth 80
#define RolesColumnWidth 250

	AllEntityList::AllEntityList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
	}

	void AllEntityList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new WComboBoxFilterModel(tr("Type"), "type", std::bind(&FiltersTemplate::initEntityTypeEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new BitmaskFilterModel(tr("Role"), "specificTypeMask", std::bind(&FiltersTemplate::initRoleEdit, std::placeholders::_1)));
	}

	void AllEntityList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, type, specificTypeMask FROM " + std::string(Entity::tableName()));
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(EntityType, model->addColumn("type"), tr("Type"), TypeColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	PersonList::PersonList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
	}

	void PersonList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new BitmaskFilterModel(tr("Role"), "specificTypeMask", std::bind(&FiltersTemplate::initRoleEdit, std::placeholders::_1)));
	}

	void PersonList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::PersonType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	BusinessList::BusinessList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
	}

	void BusinessList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new BitmaskFilterModel(tr("Role"), "specificTypeMask", std::bind(&FiltersTemplate::initRoleEdit, std::placeholders::_1)));
	}

	void BusinessList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::BusinessType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	EmployeeList::EmployeeList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
	}

	void EmployeeList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new BitmaskFilterModel(tr("Role"), "specificTypeMask", std::bind(&FiltersTemplate::initRoleEdit, std::placeholders::_1)));
	}

	void EmployeeList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName()))
			.where("type = ? AND specificTypeMask & ?").bind(Entity::PersonType).bind(Entity::EmployeeType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	PersonnelList::PersonnelList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
	}

	void PersonnelList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new BitmaskFilterModel(tr("Role"), "specificTypeMask", std::bind(&FiltersTemplate::initRoleEdit, std::placeholders::_1)));
	}

	void PersonnelList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName()))
			.where("type = ? AND (specificTypeMask & ?) = ?")
			.bind(Entity::PersonType).bind(Entity::EmployeeType | Entity::PersonnelType).bind(Entity::EmployeeType | Entity::PersonnelType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	ClientList::ClientList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
		auto model = queryModel();
		model->setHeaderData(Id, Wt::Horizontal, tr("ID"));
		_tableView->setColumnWidth(Id, IdColumnWidth);
		model->setHeaderData(Name, Wt::Horizontal, tr("Name"));
		_tableView->setColumnWidth(Name, NameColumnWidth);
		model->setHeaderData(SpecificTypeMask, Wt::Horizontal, tr("Roles"));
		_tableView->setColumnWidth(SpecificTypeMask, RolesColumnWidth);
	}

	void ClientList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new BitmaskFilterModel(tr("Role"), "specificTypeMask", std::bind(&FiltersTemplate::initRoleEdit, std::placeholders::_1)));
	}

	void ClientList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, type, specificTypeMask FROM " + std::string(Entity::tableName())).where("specificTypeMask & ?").bind(Entity::ClientType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(EntityType, model->addColumn("type"), tr("Type"), TypeColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	//CRAAAAP
	void FiltersTemplate::initRoleEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(0, tr("Employee"));
		edit->model()->setData(edit->model()->index(0, 0), Entity::EmployeeType, Wt::UserRole);

		edit->insertItem(1, tr("Personnel"));
		edit->model()->setData(edit->model()->index(1, 0), Entity::PersonnelType, Wt::UserRole);

		edit->insertItem(2, tr("Client"));
		edit->model()->setData(edit->model()->index(2, 0), Entity::ClientType, Wt::UserRole);
	}

	void FiltersTemplate::initEntityTypeEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(Entity::PersonType, tr("Person"));
		edit->insertItem(Entity::BusinessType, tr("Business"));
	}

}
