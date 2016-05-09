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

#define NameColumnWidth 250
#define TypeColumnWidth 80
#define RolesColumnWidth 250

	AllEntityList::AllEntityList()
		: QueryModelFilteredList()
	{ }

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

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>("SELECT id, name, type, specificTypeMask FROM " + std::string(Entity::tableName()));
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false);

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(EntityType, model->addColumn("type"), tr("Type"), TypeColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	PersonList::PersonList()
		: QueryModelFilteredList()
	{ }

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

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::PersonType);
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false);

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	BusinessList::BusinessList()
		: QueryModelFilteredList()
	{ }

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

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::BusinessType);
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false);

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
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

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName()))
			.where("type = ? AND specificTypeMask & ?").bind(Entity::PersonType).bind(Entity::EmployeeType);
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false);

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	PersonnelList::PersonnelList()
		: QueryModelFilteredList()
	{ }

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

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName()))
			.where("type = ? AND (specificTypeMask & ?) = ?")
			.bind(Entity::PersonType).bind(Entity::EmployeeType | Entity::PersonnelType).bind(Entity::EmployeeType | Entity::PersonnelType);
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false);

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(Id, model->addColumn("id"), tr("ID"), IdColumnWidth);
		addColumn(Name, model->addColumn("name"), tr("Name"), NameColumnWidth);
		addColumn(SpecificTypeMask, model->addColumn("specificTypeMask"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	ClientList::ClientList()
		: QueryModelFilteredList()
	{ }

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

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>("SELECT id, name, type, specificTypeMask FROM " + std::string(Entity::tableName())).where("specificTypeMask & ?").bind(Entity::ClientType);
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false);

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

	template<int IdColumn>
	EntityListProxyModel<IdColumn>::EntityListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	template<int IdColumn>
	boost::any EntityListProxyModel<IdColumn>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");

			if(role == Wt::LinkRole)
			{
				boost::any idData = data(index(idx.row(), IdColumn));
				return Wt::WLink(Wt::WLink::InternalPath, Entity::viewInternalPath(Wt::asString(idData).toUTF8()));
			}
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	template<int IdColumn>
	Wt::WFlags<Wt::ItemFlag> EntityListProxyModel<IdColumn>::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	template<int IdColumn>
	boost::any EntityListProxyModel<IdColumn>::headerData(int section, Wt::Orientation orientation /* = Wt::Horizontal */, int role /* = Wt::DisplayRole */) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	template<int IdColumn>
	void EntityListProxyModel<IdColumn>::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	//EXPLICIT SPECIALIZATIONS
	template class EntityListProxyModel<0>;

}
