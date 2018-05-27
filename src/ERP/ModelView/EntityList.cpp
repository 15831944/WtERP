#include "ModelView/EntityList.h"
#include "ModelView/EntityView.h"

#include <Wt/WCheckBox.h>
#include <Wt/WTableView.h>
#include <Wt/WIntValidator.h>

namespace ERP
{

#define NameColumnWidth 250
#define TypeColumnWidth 80
#define RolesColumnWidth 250

	void AllEntityList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Name"), "e.name"));
		filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(make_shared<WComboBoxFilterModel>(tr("Type"), "e.type", std::bind(&FiltersTemplate::initEntityTypeEdit, _1)));
	}

	void AllEntityList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, e.type, ea.id, ca.id FROM " + Entity::tStr() + " e "
			"LEFT JOIN " + EmployeeAssignment::tStr() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + ClientAssignment::tStr() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewEntityType, model->addColumn("e.type"), tr("Type"), TypeColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = make_shared<EntityListProxyModel<AllEntityList>>(_model);
	}

	Dbo::Query<AllEntityList::ResultType> AllEntityList::generateQuery() const
	{
		Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void PersonList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void PersonList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, ea.id, ca.id FROM " + Entity::tStr() + " e "
			"LEFT JOIN " + EmployeeAssignment::tStr() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + ClientAssignment::tStr() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).where("e.type = " + std::to_string(Entity::PersonType)).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = make_shared<EntityListProxyModel<PersonList>>(_model);
	}

	Dbo::Query<PersonList::ResultType> PersonList::generateQuery() const
	{
		Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void BusinessList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void BusinessList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, ea.id, ca.id FROM " + Entity::tStr() + " e "
			"LEFT JOIN " + EmployeeAssignment::tStr() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + ClientAssignment::tStr() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).where("e.type = " + std::to_string(Entity::BusinessType)).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = make_shared<EntityListProxyModel<BusinessList>>(_model);
	}

	Dbo::Query<BusinessList::ResultType> BusinessList::generateQuery() const
	{
		Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void EmployeeList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void EmployeeList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, ea.id, ca.id FROM " + Entity::tStr() + " e "
			"INNER JOIN " + EmployeeAssignment::tStr() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + ClientAssignment::tStr() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = make_shared<EntityListProxyModel<EmployeeList>>(_model);
	}

	Dbo::Query<EmployeeList::ResultType> EmployeeList::generateQuery() const
	{
		Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void ClientList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(make_shared<WComboBoxFilterModel>(tr("Type"), "e.type", std::bind(&FiltersTemplate::initEntityTypeEdit, _1)));
	}

	void ClientList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, e.type, ea.id, ca.id FROM " + Entity::tStr() + " e "
			"LEFT JOIN " + EmployeeAssignment::tStr() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"INNER JOIN " + ClientAssignment::tStr() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewEntityType, model->addColumn("e.type"), tr("Type"), TypeColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = make_shared<EntityListProxyModel<ClientList>>(_model);
	}

	Dbo::Query<ClientList::ResultType> ClientList::generateQuery() const
	{
		Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	//CRAAAAP
	void FiltersTemplate::initRoleEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(0, tr("Employee"));
		edit->model()->setData(edit->model()->index(0, 0), Entity::EmployeeType, Wt::ItemDataRole::User);

		edit->insertItem(1, tr("Client"));
		edit->model()->setData(edit->model()->index(1, 0), Entity::ClientType, Wt::ItemDataRole::User);
	}

	void FiltersTemplate::initEntityTypeEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(Entity::PersonType, tr("Person"));
		edit->insertItem(Entity::BusinessType, tr("Business"));
	}

}
