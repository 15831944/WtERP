#include "Widgets/EntityList.h"
#include "Widgets/EntityView.h"
#include "Utilities/FilteredList.h"

#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WTableView.h>
#include <Wt/WPushButton.h>
#include <Wt/WIntValidator.h>
#include <Wt/WSuggestionPopup.h>

namespace GS
{

#define NameColumnWidth 250
#define TypeColumnWidth 80
#define RolesColumnWidth 250

	void AllEntityList::initFilters()
	{
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Name"), "e.name"));
		filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(std::make_shared<WComboBoxFilterModel>(tr("Type"), "e.type", std::bind(&FiltersTemplate::initEntityTypeEdit, std::placeholders::_1)));
	}

	void AllEntityList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, e.type, ea.id, p.id, ca.id FROM " + std::string(Entity::tableName()) + " e "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewEntityType, model->addColumn("e.type"), tr("Type"), TypeColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = std::make_shared<EntityListProxyModel<AllEntityList>>(_model);
	}

	Wt::Dbo::Query<AllEntityList::ResultType> AllEntityList::generateQuery() const
	{
		Wt::Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void PersonList::initFilters()
	{
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void PersonList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, ea.id, p.id, ca.id FROM " + std::string(Entity::tableName()) + " e "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).where("e.type = " + boost::lexical_cast<std::string>(Entity::PersonType)).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = std::make_shared<EntityListProxyModel<PersonList>>(_model);
	}

	Wt::Dbo::Query<PersonList::ResultType> PersonList::generateQuery() const
	{
		Wt::Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void BusinessList::initFilters()
	{
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void BusinessList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, ea.id, p.id, ca.id FROM " + std::string(Entity::tableName()) + " e "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).where("e.type = " + boost::lexical_cast<std::string>(Entity::BusinessType)).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = std::make_shared<EntityListProxyModel<BusinessList>>(_model);
	}

	Wt::Dbo::Query<BusinessList::ResultType> BusinessList::generateQuery() const
	{
		Wt::Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void EmployeeList::initFilters()
	{
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void EmployeeList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, ea.id, p.id, ca.id FROM " + std::string(Entity::tableName()) + " e "
			"INNER JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = std::make_shared<EntityListProxyModel<EmployeeList>>(_model);
	}

	Wt::Dbo::Query<EmployeeList::ResultType> EmployeeList::generateQuery() const
	{
		Wt::Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void PersonnelList::initFilters()
	{
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void PersonnelList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, ea.id, p.id, ca.id FROM " + std::string(Entity::tableName()) + " e "
			"INNER JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"INNER JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = std::make_shared<EntityListProxyModel<PersonnelList>>(_model);
	}

	Wt::Dbo::Query<PersonnelList::ResultType> PersonnelList::generateQuery() const
	{
		Wt::Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	void ClientList::initFilters()
	{
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(std::make_shared<WComboBoxFilterModel>(tr("Type"), "e.type", std::bind(&FiltersTemplate::initEntityTypeEdit, std::placeholders::_1)));
	}

	void ClientList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id, e.name, e.type, ea.id, p.id, ca.id FROM " + std::string(Entity::tableName()) + " e "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"INNER JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("e.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewEntityType, model->addColumn("e.type"), tr("Type"), TypeColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = std::make_shared<EntityListProxyModel<ClientList>>(_model);
	}

	Wt::Dbo::Query<ClientList::ResultType> ClientList::generateQuery() const
	{
		Wt::Dbo::Query<ResultType> query(_baseQuery);
		Wt::WDate currentDate = Wt::WDate::currentServerDate();
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		return query;
	}

	//CRAAAAP
	void FiltersTemplate::initRoleEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(0, tr("Employee"));
		edit->model()->setData(edit->model()->index(0, 0), Entity::EmployeeType, Wt::ItemDataRole::User);

		edit->insertItem(1, tr("Personnel"));
		edit->model()->setData(edit->model()->index(1, 0), Entity::PersonnelType, Wt::ItemDataRole::User);

		edit->insertItem(2, tr("Client"));
		edit->model()->setData(edit->model()->index(2, 0), Entity::ClientType, Wt::ItemDataRole::User);
	}

	void FiltersTemplate::initEntityTypeEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(Entity::PersonType, tr("Person"));
		edit->insertItem(Entity::BusinessType, tr("Business"));
	}

}
