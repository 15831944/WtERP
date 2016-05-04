#include "HRMVC.h"
#include "Widgets/EntryCycleMVC.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/WTextArea>
#include <Wt/WPushButton>
#include <Wt/WDateEdit>
#include <Wt/WTableView>

namespace GS
{

	const Wt::WFormModel::Field EmployeeAssignmentFormModel::descriptionField = "description";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::startDateField = "startDate";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::endDateField = "endDate";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::entityField = "entity";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::locationField = "location";

	EmployeeAssignmentFormModel::EmployeeAssignmentFormModel(EmployeeAssignmentView *view, Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr /*= Wt::Dbo::ptr<EmployeeAssignment>()*/)
		: RecordFormModel(view, employeeAssignmentPtr), _view(view)
	{
		addField(descriptionField);
		addField(startDateField);
		addField(endDateField);
		addField(entityField);
		addField(locationField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(descriptionField, Wt::WString::fromUTF8(_recordPtr->description));
			setValue(startDateField, _recordPtr->startDate);
			setValue(endDateField, _recordPtr->endDate);
			setValue(entityField, _recordPtr->entityPtr);
			setValue(locationField, _recordPtr->locationPtr);
		}
	}

	Wt::WWidget *EmployeeAssignmentFormModel::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == descriptionField)
		{
			Wt::WTextArea *description = new Wt::WTextArea();
			description->setRows(3);
			return description;
		}
		if(field == startDateField)
		{
			auto startDateEdit = new Wt::WDateEdit();
			startDateEdit->setPlaceholderText(Wt::WLocale::currentLocale().dateFormat());
			auto startDateValidator = new Wt::WDateValidator();
			startDateValidator->setMandatory(true);
			setValidator(startDateField, startDateValidator);
			startDateEdit->changed().connect(boost::bind(&EmployeeAssignmentFormModel::updateEndDateValidator, this, true));
			return startDateEdit;
		}
		if(field == endDateField)
		{
			auto endDateEdit = new Wt::WDateEdit();
			endDateEdit->setPlaceholderText(Wt::WString::tr("EmptyEndDate"));
			auto endDateValidator = new Wt::WDateValidator();
			setValidator(endDateField, endDateValidator);
			return endDateEdit;
		}
		if(field == entityField)
		{
			FindEntityEdit *findEntityEdit = new FindEntityEdit(Entity::PersonType, Entity::EmployeeType);
			FindEntityValidator *findEntityValidator = new FindEntityValidator(findEntityEdit, true);
			findEntityValidator->setModifyPermissionRequired(true);
			setValidator(entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == locationField)
		{
			FindLocationEdit *findLocationEdit = new FindLocationEdit();
			FindLocationValidator *findLocationValidator = new FindLocationValidator(findLocationEdit, true);
			setValidator(locationField, findLocationValidator);
			return findLocationEdit;
		}
		return nullptr;
	}

	bool EmployeeAssignmentFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);

		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().add(new EmployeeAssignment());
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->description = valueText(descriptionField).toUTF8();
		_recordPtr.modify()->startDate = boost::any_cast<Wt::WDate>(value(startDateField));
		_recordPtr.modify()->endDate = boost::any_cast<Wt::WDate>(value(endDateField));
		_recordPtr.modify()->entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->locationPtr = boost::any_cast<Wt::Dbo::ptr<Location>>(value(locationField));

		t.commit();
		return true;
	}

	EmployeeAssignmentView::EmployeeAssignmentView(Wt::Dbo::ptr<EmployeeAssignment> assignmentPtr /*= Wt::Dbo::ptr<EmployeeAssignment>()*/)
		: RecordFormView(tr("GS.Admin.EmployeeAssignmentView")), _tempPtr(assignmentPtr)
	{ }

	void EmployeeAssignmentView::init()
	{
		_model = new EmployeeAssignmentFormModel(this, _tempPtr);
		addFormModel("assignment", _model);
	}

	void EmployeeAssignmentView::updateView(Wt::WFormModel *model)
	{
		RecordFormView::updateView(model);
		_model->updateEndDateValidator(false);
	}

	void EmployeeAssignmentView::updateModel(Wt::WFormModel *model)
	{
		RecordFormView::updateModel(model);
		_model->updateEndDateValidator(false);
	}

	Wt::WString EmployeeAssignmentView::viewName() const
	{
		if(employeeAssignmentPtr())
			return tr("EmployeeAssignmentViewName").arg(employeeAssignmentPtr().id());

		return RecordFormView::viewName();
	}

	void EmployeeAssignmentFormModel::updateEndDateValidator(bool update)
	{
		if(update)
			_view->updateModelField(this, EmployeeAssignmentFormModel::startDateField);

		const boost::any &startDateVal = value(EmployeeAssignmentFormModel::startDateField);
		if(startDateVal.empty())
			return;

		Wt::WDate startDate = boost::any_cast<Wt::WDate>(startDateVal);
		Wt::WDateValidator *endDateValidator = dynamic_cast<Wt::WDateValidator*>(validator(EmployeeAssignmentFormModel::endDateField));
		endDateValidator->setBottom(startDate.isValid() ? startDate.addDays(1) : Wt::WDate());
	}

	EmployeeExpenseView::EmployeeExpenseView(bool isEmployeeAssignment)
		: Wt::WTemplate(), SubmittableRecordWidget(this),
		_isEmployeeAssignment(isEmployeeAssignment)
	{
		setTemplateText(_isEmployeeAssignment ? tr("GS.Admin.EmployeeExpenseView.Assignment") : tr("GS.Admin.EmployeeExpenseView.ExpenseCycle"));
		init();
	}

	EmployeeExpenseView::EmployeeExpenseView(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr)
		: Wt::WTemplate(tr("GS.Admin.EmployeeExpenseView.Assignment")), SubmittableRecordWidget(this),
		_isEmployeeAssignment(true), _tempAssignment(employeeAssignmentPtr)
	{
		init();
	}

	EmployeeExpenseView::EmployeeExpenseView(Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr)
		: Wt::WTemplate(tr("GS.Admin.EmployeeExpenseView.ExpenseCycle")), SubmittableRecordWidget(this),
		_isEmployeeAssignment(false), _tempCycle(expenseCyclePtr)
	{
		init();
	}

	void EmployeeExpenseView::load()
	{
		if(!loaded())
		{
			_assignments = new Wt::WContainerWidget();
			_cycles = new Wt::WContainerWidget();

			auto addAssignmentBtn = new ShowEnabledButton(tr("AddEmployeeAssignment"));
			addAssignmentBtn->clicked().connect(this, &EmployeeExpenseView::handleAddAssignment);
			bindWidget("add-assignment", addAssignmentBtn);

			auto addCycleBtn = new ShowEnabledButton(tr("AddRecurringExpense"));
			addCycleBtn->clicked().connect(this, &EmployeeExpenseView::handleAddCycle);
			bindWidget("add-cycle", addCycleBtn);

			if(_isEmployeeAssignment)
			{
				_mainView = addAssignment(_tempAssignment);
				if(_tempAssignment)
				{
					TRANSACTION(APP);
					if(_tempAssignment->expenseCyclePtr)
						addCycle(_tempAssignment->expenseCyclePtr);
				}
			}
			else
			{
				_mainView = addCycle(_tempCycle);
				if(_tempCycle)
				{
					TRANSACTION(APP);
					EmployeeAssignmentCollection collection = _tempCycle->employeeAssignmentCollection;
					for(const auto &ptr : collection)
						addAssignment(ptr);
				}
			}

			_mainView->submitted().connect(std::bind([this]() {
				submitted().emit();
			}));

			bindWidget("assignments", _assignments);
			bindWidget("cycles", _cycles);
			addAssignmentBtn->setDisabled(!canAddAssignment());
			addCycleBtn->setDisabled(!canAddCycle());
		}

		Wt::WTemplate::load();
	}

	void EmployeeExpenseView::init()
	{
		addFunction("tr", &Wt::WTemplate::Functions::tr);
		addFunction("block", &Wt::WTemplate::Functions::block);
		addFunction("id", &Wt::WTemplate::Functions::id);
		addFunction("fwId", &Wt::WTemplate::Functions::fwId);
	}

	Wt::WString EmployeeExpenseView::viewName() const
	{
		if(_mainView)
			return _mainView->viewName();

		return SubmittableRecordWidget::viewName();
	}

	std::string EmployeeExpenseView::viewInternalPath() const
	{
		if(_mainView)
			return _mainView->viewInternalPath();

		return SubmittableRecordWidget::viewInternalPath();
	}

	EmployeeAssignmentView *EmployeeExpenseView::assignmentView(int index) const
	{
		return _assignments ? (index < _assignments->count()) ? dynamic_cast<EmployeeAssignmentView*>(_assignments->widget(index)) : nullptr : nullptr;
	}

	ExpenseCycleView *EmployeeExpenseView::cycleView(int index) const
	{
		return _cycles ? (index < _cycles->count()) ? dynamic_cast<ExpenseCycleView*>(_cycles->widget(index)) : nullptr : nullptr;
	}

	void EmployeeExpenseView::handleAddCycle()
	{
		if(!canAddCycle())
			return;

		addCycle(Wt::Dbo::ptr<ExpenseCycle>());
	}

	void EmployeeExpenseView::handleAddAssignment()
	{
		if(!canAddAssignment())
			return;

		addAssignment(Wt::Dbo::ptr<EmployeeAssignment>());
	}
	
	EmployeeAssignmentView *EmployeeExpenseView::addAssignment(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr)
	{
		EmployeeAssignmentView *newView = new EmployeeAssignmentView(employeeAssignmentPtr);
		if(!_isEmployeeAssignment)
		{
			if(ExpenseCycleView *cMainView = dynamic_cast<ExpenseCycleView*>(_mainView))
			{
				//_mainView->loaded() == true is asserted
				newView->load();
				newView->model()->setReadOnly(EmployeeAssignmentFormModel::entityField, true);

				if(cMainView->model()->isRecordPersisted())
				{
					const boost::any &entityVal = cMainView->model()->value(EntryCycleFormModel::entityField);
					if(!entityVal.empty())
					{
						if(Wt::Dbo::ptr<Entity> entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(entityVal))
						{
							WApplication *app = APP;
							TRANSACTION(app);
							newView->model()->setValue(EmployeeAssignmentFormModel::entityField, entityPtr);
						}
					}
				}

				newView->updateViewField(newView->model(), EmployeeAssignmentFormModel::entityField);
			}
		}

		_assignments->addWidget(newView);
		resolveWidget("add-assignment")->setDisabled(!canAddAssignment());
		return newView;
	}

	ExpenseCycleView *EmployeeExpenseView::addCycle(Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr)
	{
		ExpenseCycleView *newView = new ExpenseCycleView(expenseCyclePtr);
		if(_isEmployeeAssignment)
		{
			if(EmployeeAssignmentView *cMainView = dynamic_cast<EmployeeAssignmentView*>(_mainView))
			{
				//_mainView->loaded() == true is asserted
				newView->load();
				newView->model()->setReadOnly(EntryCycleFormModel::entityField, true);

				if(cMainView->model()->isRecordPersisted())
				{
					const boost::any &entityVal = cMainView->model()->value(EmployeeAssignmentFormModel::entityField);
					if(!entityVal.empty())
					{
						if(Wt::Dbo::ptr<Entity> entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(entityVal))
						{
							WApplication *app = APP;
							TRANSACTION(app);
							newView->model()->setValue(EntryCycleFormModel::entityField, entityPtr);
						}
					}
				}

				newView->updateViewField(newView->model(), EntryCycleFormModel::entityField);
			}
		}

		_cycles->addWidget(newView);
		resolveWidget("add-cycle")->setDisabled(!canAddCycle());
		return newView;
	}

	void EmployeeExpenseView::handleMainViewSubmitted()
	{
		if(_isEmployeeAssignment)
		{
			for(int i = 0; i < _cycles->count(); ++i)
			{
				auto view = dynamic_cast<ExpenseCycleView*>(_cycles->widget(i));
				view->model()->setValue(EntryCycleFormModel::entityField, _mainView->model()->value(EmployeeAssignmentFormModel::entityField));
				view->updateViewField(view->model(), EntryCycleFormModel::entityField);
			}
		}
		else
		{
			for(int i = 0; i < _assignments->count(); ++i)
			{
				auto view = dynamic_cast<EmployeeAssignmentView*>(_assignments->widget(i));
				view->model()->setValue(EmployeeAssignmentFormModel::entityField, _mainView->model()->value(EntryCycleFormModel::entityField));
				view->updateViewField(view->model(), EmployeeAssignmentFormModel::entityField);
			}
		}
	}

	EmployeeAssignmentListProxyModel::EmployeeAssignmentListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void EmployeeAssignmentListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::WFlags<Wt::ItemFlag> EmployeeAssignmentListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	boost::any EmployeeAssignmentListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	boost::any EmployeeAssignmentListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const EmployeeAssignmentList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<EmployeeAssignmentList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<EmployeeAssignmentList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, EmployeeAssignment::viewInternalPath(id));
			}
		}

		boost::any viewIndexData = headerData(idx.column(), Wt::Horizontal, Wt::ViewIndexRole);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = boost::any_cast<int>(viewIndexData);

		const EmployeeAssignmentList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<EmployeeAssignmentList::ResultType>*>(sourceModel())->resultRow(idx.row());

		if(viewIndex == EntryCycleList::ViewStartDate && role == Wt::DisplayRole)
		{
			const Wt::WDate &date = boost::get<EmployeeAssignmentList::ResStartDate>(res);
			if(date.isValid() && date > Wt::WDate(boost::gregorian::day_clock::local_day()))
				return Wt::WString::tr("XNotStarted").arg(boost::get<EmployeeAssignmentList::ResStartDate>(res).toString(Wt::WLocale::currentLocale().dateFormat()));
		}

		if(viewIndex == EntryCycleList::ViewEndDate && role == Wt::DisplayRole)
		{
			const Wt::WDate &date = boost::get<EmployeeAssignmentList::ResEndDate>(res);
			if(date.isValid() && Wt::WDate(boost::gregorian::day_clock::local_day()) >= date)
				return Wt::WString::tr("XEnded").arg(date.toString(Wt::WLocale::currentLocale().dateFormat()));
		}

		if(role == Wt::StyleClassRole)
		{
			const Wt::WDate &startDate = boost::get<EmployeeAssignmentList::ResStartDate>(res);
			if(startDate.isValid() && startDate > Wt::WDate(boost::gregorian::day_clock::local_day()))
				return "text-info";

			const Wt::WDate &endDate = boost::get<EmployeeAssignmentList::ResEndDate>(res);
			if(endDate.isValid() && Wt::WDate(boost::gregorian::day_clock::local_day()) >= endDate)
				return "text-muted";
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	EmployeeAssignmentList::EmployeeAssignmentList()
		: QueryModelFilteredList()
	{ }

	void EmployeeAssignmentList::load()
	{
		bool ld = !loaded();
		QueryModelFilteredList::load();

		if(ld)
		{
			int timestampColumn = viewIndexToColumn(ViewCreatedOn);
			if(timestampColumn != -1)
				_tableView->sortByColumn(timestampColumn, Wt::DescendingOrder);
		}
	}

	void EmployeeAssignmentList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "a.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1))); filtersTemplate()->addFilter(1);
	}

	void EmployeeAssignmentList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT a.id a_id, a.timestamp, e.name e_name, a.startDate, a.endDate, cnt.name cnt_name, city.name city_name, l.address FROM "
			+ std::string(EmployeeAssignment::tableName()) + " a "
			"INNER JOIN " + Entity::tableName() + " e ON (e.id = a.entity_id) "
			"LEFT JOIN " + Location::tableName() + " l ON (l.id = a.location_id) "
			"LEFT JOIN " + Country::tableName() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tableName() + " city ON (city.id = l.city_id)");

		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "a.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("a.id a_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("a.timestamp"), tr("CreatedOn"), DateTimeColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.name e_name"), tr("Entity"), 200);
		addColumn(ViewStartDate, model->addColumn("a.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("a.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewCountry, model->addColumn("cnt.name cnt_name"), tr("Country"), 150);
		addColumn(ViewCity, model->addColumn("city.name city_name"), tr("City"), 150);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), 300);

		_proxyModel = new EmployeeAssignmentListProxyModel(_model, _model);
	}

}
