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

	void AllEntityList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new WComboBoxFilterModel(tr("Type"), "e.type", std::bind(&FiltersTemplate::initEntityTypeEdit, std::placeholders::_1)));
	}

	void AllEntityList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id e_id, e.name, e.type e_type, ea.id ea_id, p.id p_id, ca.id ca_id FROM " + std::string(Entity::tableName()) + " e "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("e.id e_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewEntityType, model->addColumn("e.type e_type"), tr("Type"), TypeColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id ea_id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<AllEntityList>(_model, _model);
	}

	void AllEntityList::reload()
	{
		if(loaded())
		{
			Wt::Dbo::Query<ResultType> query(_baseQuery);
			Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
			query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
			static_cast<QueryModelType*>(_model)->setQuery(query, true);
		}
		QueryModelFilteredList::reload();
	}

	void PersonList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void PersonList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id e_id, e.name, ea.id ea_id, p.id p_id, ca.id ca_id FROM " + std::string(Entity::tableName()) + " e "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).where("e.type = " + boost::lexical_cast<std::string>(Entity::PersonType)).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("e.id e_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id ea_id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<PersonList>(_model, _model);
	}

	void PersonList::reload()
	{
		if(loaded())
		{
			Wt::Dbo::Query<ResultType> query(_baseQuery);
			Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
			query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
			static_cast<QueryModelType*>(_model)->setQuery(query, true);
		}
		QueryModelFilteredList::reload();
	}

	void BusinessList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void BusinessList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id e_id, e.name, ea.id ea_id, p.id p_id, ca.id ca_id FROM " + std::string(Entity::tableName()) + " e "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).where("e.type = " + boost::lexical_cast<std::string>(Entity::BusinessType)).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("e.id e_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id ea_id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<BusinessList>(_model, _model);
	}

	void BusinessList::reload()
	{
		if(loaded())
		{
			Wt::Dbo::Query<ResultType> query(_baseQuery);
			Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
			query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
			static_cast<QueryModelType*>(_model)->setQuery(query, true);
		}
		QueryModelFilteredList::reload();
	}

	void EmployeeList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void EmployeeList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id e_id, e.name, ea.id ea_id, p.id p_id, ca.id ca_id FROM " + std::string(Entity::tableName()) + " e "
			"INNER JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("e.id e_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id ea_id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<EmployeeList>(_model, _model);
	}

	void EmployeeList::reload()
	{
		if(loaded())
		{
			Wt::Dbo::Query<ResultType> query(_baseQuery);
			Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
			query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
			static_cast<QueryModelType*>(_model)->setQuery(query, true);
		}
		QueryModelFilteredList::reload();
	}

	void PersonnelList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
	}

	void PersonnelList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id e_id, e.name, ea.id ea_id, p.id p_id, ca.id ca_id FROM " + std::string(Entity::tableName()) + " e "
			"INNER JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"INNER JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"LEFT JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("e.id e_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id ea_id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<PersonnelList>(_model, _model);
	}

	void PersonnelList::reload()
	{
		if(loaded())
		{
			Wt::Dbo::Query<ResultType> query(_baseQuery);
			Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
			query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
			static_cast<QueryModelType*>(_model)->setQuery(query, true);
		}
		QueryModelFilteredList::reload();
	}

	void ClientList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "e.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Name"), "e.name")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new WComboBoxFilterModel(tr("Type"), "e.type", std::bind(&FiltersTemplate::initEntityTypeEdit, std::placeholders::_1)));
	}

	void ClientList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT e.id e_id, e.name, e.type e_type, ea.id ea_id, p.id p_id, ca.id ca_id FROM " + std::string(Entity::tableName()) + " e "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON (ea.entity_id = e.id AND ea.startDate <= ? AND (ea.endDate IS NULL OR ea.endDate > ?)) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = ea.employeeposition_id AND p.type = " + boost::lexical_cast<std::string>(EmployeePosition::PersonnelType) + ") "
			"INNER JOIN " + ClientAssignment::tableName() + " ca ON (ca.entity_id = e.id AND ca.startDate <= ? AND (ca.endDate IS NULL OR ca.endDate > ?))"
			).groupBy("e.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "e.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
		query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("e.id e_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("e.name"), tr("Name"), NameColumnWidth);
		addColumn(ViewEntityType, model->addColumn("e.type e_type"), tr("Type"), TypeColumnWidth);
		addColumn(ViewRole, model->addColumn("ea.id ea_id"), tr("Roles"), RolesColumnWidth);

		_proxyModel = new EntityListProxyModel<ClientList>(_model, _model);
	}

	void ClientList::reload()
	{
		if(loaded())
		{
			Wt::Dbo::Query<ResultType> query(_baseQuery);
			Wt::WDate currentDate(boost::gregorian::day_clock::local_day());
			query.bind(currentDate).bind(currentDate).bind(currentDate).bind(currentDate);
			static_cast<QueryModelType*>(_model)->setQuery(query, true);
		}
		QueryModelFilteredList::reload();
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
