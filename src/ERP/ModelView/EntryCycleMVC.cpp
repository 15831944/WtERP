#include "Application/WApplication.h"
#include "ModelView/EntryCycleMVC.h"
#include "ModelView/EntityView.h"
#include "ModelView/EntityList.h"
#include "ModelView/AccountMVC.h"
#include "ModelView/HRMVC.h"

#include <Wt/WDateEdit.h>
#include <Wt/WIntValidator.h>
#include <Wt/WDoubleValidator.h>
#include <Wt/WTableView.h>

namespace ERP
{

	//PAYMENT CYCLE MODEL
	const Wt::WFormModel::Field EntryCycleFormModel::entityField = "entity";
	const Wt::WFormModel::Field EntryCycleFormModel::startDateField = "startDate";
	const Wt::WFormModel::Field EntryCycleFormModel::endDateField = "endDate";
	const Wt::WFormModel::Field EntryCycleFormModel::intervalField = "interval";
	const Wt::WFormModel::Field EntryCycleFormModel::nIntervalsField = "nIntervals";
	const Wt::WFormModel::Field EntryCycleFormModel::amountField = "amount";
	const Wt::WFormModel::Field EntryCycleFormModel::firstEntryAfterCycleField = "firstEntryAfterCycle";

	void EntryCycleFormModel::addFields(Wt::WFormModel *model)
	{
		model->addField(entityField);
		model->addField(startDateField);
		model->addField(endDateField);
		model->addField(intervalField);
		model->addField(nIntervalsField);
		model->addField(amountField);
		model->addField(firstEntryAfterCycleField);
	}

	void EntryCycleFormModel::updateModelFromCycle(Wt::WFormModel *model, const EntryCycle &cycle)
	{
		model->setValue(entityField, cycle.entityPtr);
		model->setValue(startDateField, cycle.startDate);
		model->setValue(endDateField, cycle.endDate);
		model->setValue(intervalField, (int)cycle.interval);
		model->setValue(nIntervalsField, cycle.nIntervals);
		model->setValue(amountField, cycle.amount());
		model->setValue(firstEntryAfterCycleField, cycle.firstEntryAfterCycle ? 1 : 0);
	}

	template<class CycleDbo>
	void EntryCycleFormModel::updateCycleFromModel(Wt::WFormModel *model, Dbo::ptr<CycleDbo> ptr, bool newCycle)
	{
		ptr.modify()->entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(model->value(EntryCycleFormModel::entityField));
		ptr.modify()->startDate = Wt::any_cast<Wt::WDate>(model->value(EntryCycleFormModel::startDateField));
		ptr.modify()->endDate = Wt::any_cast<Wt::WDate>(model->value(EntryCycleFormModel::endDateField));
		ptr.modify()->interval = CycleInterval(Wt::any_cast<int>(model->value(EntryCycleFormModel::intervalField)));
		ptr.modify()->nIntervals = Wt::WLocale::currentLocale().toInt(model->valueText(EntryCycleFormModel::nIntervalsField));
		if(newCycle)
			ptr.modify()->_amountInCents = Money(model->valueText(EntryCycleFormModel::amountField).toUTF8(), DEFAULT_CURRENCY).valueInCents();
		ptr.modify()->firstEntryAfterCycle = Wt::any_cast<int>(model->value(EntryCycleFormModel::firstEntryAfterCycleField)) == 1;
	}

	unique_ptr<Wt::WWidget> EntryCycleFormModel::createFormWidget(Wt::WFormModel *model, EntryCycleView *view, Wt::WFormModel::Field field)
	{
		if(field == entityField)
		{
			auto findEntityEdit = make_unique<FindEntityEdit>();
			auto findEntityValidator = make_shared<FindEntityValidator>(findEntityEdit.get(), true);
			findEntityValidator->setModifyPermissionRequired(true);
			model->setValidator(EntryCycleFormModel::entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == startDateField)
		{
			auto startDateEdit = make_unique<Wt::WDateEdit>();
			startDateEdit->validator()->setMandatory(true);
			model->setValidator(EntryCycleFormModel::startDateField, startDateEdit->validator());
			startDateEdit->changed().connect(std::bind(&EntryCycleFormModel::updateEndDateValidator, model, view, true));
			return startDateEdit;
		}
		if(field == endDateField)
		{
			auto endDateEdit = make_unique<Wt::WDateEdit>();
			endDateEdit->setPlaceholderText(tr("EmptyEndDate"));
			auto endDateValidator = make_shared<CycleEndDateValidator>(model);
			model->setValidator(EntryCycleFormModel::endDateField, endDateValidator);
			return endDateEdit;
		}
		if(field == intervalField)
		{
			auto intervalCombo = make_unique<Wt::WComboBox>();
			intervalCombo->insertItem(DailyInterval, tr("Days"));
			intervalCombo->insertItem(WeeklyInterval, tr("Weeks"));
			intervalCombo->insertItem(MonthlyInterval, tr("Months"));
			intervalCombo->insertItem(YearlyInterval, tr("Years"));
			intervalCombo->setCurrentIndex(MonthlyInterval);
			intervalCombo->changed().connect(view, &EntryCycleView::handleIntervalChanged);
			intervalCombo->changed().connect(std::bind(&EntryCycleFormModel::updateEndDateValidator, model, view, true));
			return intervalCombo;
		}
		if(field == amountField)
		{
			auto amountEdit = make_unique<Wt::WLineEdit>();
			auto amountValidator = make_shared<Wt::WDoubleValidator>();
			amountValidator->setBottom(0);
			amountValidator->setMandatory(true);
			model->setValidator(EntryCycleFormModel::amountField, amountValidator);
			return amountEdit;
		}
		if(field == nIntervalsField)
		{
			auto nIntervalsEdit = make_unique<Wt::WLineEdit>();
			auto nIntervalsValidator = make_shared<Wt::WIntValidator>();
			nIntervalsValidator->setBottom(1);
			nIntervalsValidator->setMandatory(true);
			model->setValidator(EntryCycleFormModel::nIntervalsField, nIntervalsValidator);
			nIntervalsEdit->changed().connect(view, &EntryCycleView::handleIntervalChanged);
			nIntervalsEdit->changed().connect(std::bind(&EntryCycleFormModel::updateEndDateValidator, model, view, true));
			return nIntervalsEdit;
		}
		if(field == firstEntryAfterCycleField)
		{
			auto firstEntryOnEdit = make_unique<Wt::WComboBox>();
			firstEntryOnEdit->insertItem(0, tr("OnStartingDate"));
			firstEntryOnEdit->insertItem(1, Wt::WString());
			firstEntryOnEdit->changed().connect(std::bind(&EntryCycleFormModel::updateEndDateValidator, model, view, true));
			return firstEntryOnEdit;
		}
		return nullptr;
	}
	
	void EntryCycleFormModel::updateEndDateValidator(Wt::WFormModel *model, EntryCycleView *view, bool update)
	{
		if(update)
		{
			view->updateModelField(model, EntryCycleFormModel::startDateField);
			view->updateModelField(model, EntryCycleFormModel::intervalField);
			view->updateModelField(model, EntryCycleFormModel::nIntervalsField);
			view->updateModelField(model, EntryCycleFormModel::firstEntryAfterCycleField);
		}

		const Wt::any &startDateVal = model->value(EntryCycleFormModel::startDateField);
		if(startDateVal.empty())
			return;

		Wt::WDate startDate = Wt::any_cast<Wt::WDate>(startDateVal);
		auto interval = (CycleInterval)Wt::any_cast<int>(model->value(EntryCycleFormModel::intervalField));
		Wt::WString nIntervalsStr = model->valueText(EntryCycleFormModel::nIntervalsField);
		auto FEAC = Wt::any_cast<int>(model->value(EntryCycleFormModel::firstEntryAfterCycleField));

		auto endDateValidator = static_pointer_cast<Wt::WDateValidator>(model->validator(EntryCycleFormModel::endDateField));
		if(!startDate.isValid())
		{
			endDateValidator->setBottom(Wt::WDate());
			return;
		}

		Wt::WDate endDateBottom = startDate.addDays(1);
		if(FEAC == 1)
		{
			try
			{
				int nIntervals = Wt::WLocale::currentLocale().toInt(nIntervalsStr);
				endDateBottom = addCycleInterval(Wt::WDateTime(endDateBottom), interval, nIntervals).date();
			}
			catch(const std::invalid_argument &) {}
			catch(const std::out_of_range &) {}
		}

		endDateValidator->setBottom(endDateBottom);
	}

	//ENTRY CYCLE VIEW
	void EntryCycleView::updateView(Wt::WFormModel *model)
	{
		RecordFormView::updateView(model);
		EntryCycleFormModel::updateEndDateValidator(model, this, false);
		handleIntervalChanged();
	}

	void EntryCycleView::updateModel(Wt::WFormModel *model)
	{
		RecordFormView::updateModel(model);
		EntryCycleFormModel::updateEndDateValidator(model, this, false);
	}

	void EntryCycleView::handleIntervalChanged()
	{
		auto firstEntryOnEdit = resolve<Wt::WComboBox*>(EntryCycleFormModel::firstEntryAfterCycleField);
		if(firstEntryOnEdit)
			firstEntryOnEdit->setItemText(1, tr("AfterTheInterval"));
		else
			return;

		auto nIntervalsEdit = resolve<Wt::WLineEdit*>(EntryCycleFormModel::nIntervalsField);
		auto intervalCombo = resolve<Wt::WComboBox*>(EntryCycleFormModel::intervalField);
		if(!nIntervalsEdit || !intervalCombo)
			return;

		if(!nIntervalsEdit->valueText().empty() && nIntervalsEdit->validate() == Wt::ValidationState::Valid)
		{
			int nIntervals = Wt::WLocale::currentLocale().toInt(nIntervalsEdit->valueText());
			switch(intervalCombo->currentIndex())
			{
			case DailyInterval: firstEntryOnEdit->setItemText(1, trn("AfterNDays", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case WeeklyInterval: firstEntryOnEdit->setItemText(1, trn("AfterNWeeks", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case MonthlyInterval: firstEntryOnEdit->setItemText(1, trn("AfterNMonths", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case YearlyInterval: firstEntryOnEdit->setItemText(1, trn("AfterNYears", nIntervals).arg(nIntervalsEdit->valueText())); break;
			default: firstEntryOnEdit->setItemText(1, tr("AfterTheInterval")); break;
			}
		}
	}

	Wt::WValidator::Result CycleEndDateValidator::validate(const Wt::WString &input) const
	{
		if(_model->value(EntryCycleFormModel::nIntervalsField).empty()
			|| _model->value(EntryCycleFormModel::intervalField).empty()
			|| _model->value(EntryCycleFormModel::startDateField).empty()
			|| _model->value(EntryCycleFormModel::firstEntryAfterCycleField).empty())
		{
			return Result(Wt::ValidationState::Invalid, tr("Error"));
		}
		return Wt::WDateValidator::validate(input);
	}

	//EXPENSE CYCLE MODEL
	ExpenseCycleFormModel::ExpenseCycleFormModel(ExpenseCycleView *view, Dbo::ptr<ExpenseCycle> cyclePtr)
		: RecordFormModel(view, move(cyclePtr)), _view(view)
	{
		EntryCycleFormModel::addFields(this);
	}

	void ExpenseCycleFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		EntryCycleFormModel::updateModelFromCycle(this, *_recordPtr);
	}

	unique_ptr<Wt::WWidget> ExpenseCycleFormModel::createFormWidget(Wt::WFormModel::Field field)
	{
		if(auto w = EntryCycleFormModel::createFormWidget(this, _view, field))
			return w;

		return RecordFormModel::createFormWidget(field);
	}

	bool ExpenseCycleFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);

		bool newCycle = false;
		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().addNew<ExpenseCycle>();
			_recordPtr.modify()->setCreatedByValues();
			newCycle = true;
		}

		EntryCycleFormModel::updateCycleFromModel(this, _recordPtr, newCycle);

		if(newCycle && _recordPtr->firstEntryAfterCycle == false)
			app->accountsDatabase().createPendingCycleEntry(_recordPtr, nullptr, Wt::WDateTime::currentDateTime());

		_recordPtr.flush();
		t.commit();
		return true;
	}

	void ExpenseCycleFormModel::persistedHandler()
	{
		setReadOnly(EntryCycleFormModel::startDateField, true);

		_view->bindNew<AccountEntryList>("entries", recordPtr());
		_view->bindNew<EmployeeAssignmentList>("employeeAssignments", recordPtr());
	}

	//EXPENSE CYCLE VIEW
	ExpenseCycleView::ExpenseCycleView(Dbo::ptr<ExpenseCycle> cyclePtr)
		: EntryCycleView(tr("ERP.Admin.ExpenseCycleView"))
	{
		_model = newFormModel<ExpenseCycleFormModel>("expense", this, move(cyclePtr));
	}

	unique_ptr<Wt::WWidget> ExpenseCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(auto result = EntryCycleView::createFormWidget(field))
			return result;

		return nullptr;
	}

	Wt::WString ExpenseCycleView::viewName() const
	{
		if(cyclePtr())
		{
			TRANSACTION(APP);
			if(cyclePtr()->entityPtr)
				return tr("ExpenseCycleViewName").arg(cyclePtr().id()).arg(cyclePtr()->entityPtr->name);
			else
				return tr("ExpenseCycleViewNameWithoutEntity").arg(cyclePtr().id());
		}
		return "ExpenseCycleView";
	}

	//INCOME CYCLE MODEL
	IncomeCycleFormModel::IncomeCycleFormModel(IncomeCycleView *view, Dbo::ptr<IncomeCycle> cyclePtr)
		: RecordFormModel(view, move(cyclePtr)), _view(view)
	{
		EntryCycleFormModel::addFields(this);
	}

	void IncomeCycleFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		EntryCycleFormModel::updateModelFromCycle(this, *_recordPtr);
	}

	unique_ptr<Wt::WWidget> IncomeCycleFormModel::createFormWidget(Wt::WFormModel::Field field)
	{
		if(auto w = EntryCycleFormModel::createFormWidget(this, _view, field))
			return w;

		return RecordFormModel::createFormWidget(field);
	}

	bool IncomeCycleFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);

		bool newCycle = false;
		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().addNew<IncomeCycle>();
			_recordPtr.modify()->setCreatedByValues();
			newCycle = true;
		}

		EntryCycleFormModel::updateCycleFromModel(this, _recordPtr, newCycle);

		if(newCycle && _recordPtr->firstEntryAfterCycle == false)
			app->accountsDatabase().createPendingCycleEntry(_recordPtr, nullptr, Wt::WDateTime::currentDateTime());

		t.commit();
		return true;
	}

	void IncomeCycleFormModel::persistedHandler()
	{
		setReadOnly(EntryCycleFormModel::startDateField, true);

		_view->bindNew<AccountEntryList>("entries", recordPtr());
		_view->bindNew<ClientAssignmentList>("clientAssignments", recordPtr());
	}

	//INCOME CYCLE VIEW
	IncomeCycleView::IncomeCycleView(Dbo::ptr<IncomeCycle> cyclePtr)
		: EntryCycleView(tr("ERP.Admin.IncomeCycleView"))
	{
		_model = newFormModel<IncomeCycleFormModel>("income", this, move(cyclePtr));
	}

	unique_ptr<Wt::WWidget> IncomeCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		return EntryCycleView::createFormWidget(field);
	}

	Wt::WString IncomeCycleView::viewName() const
	{
		if(cyclePtr())
		{
			TRANSACTION(APP);
			if(cyclePtr()->entityPtr)
				return tr("IncomeCycleViewName").arg(cyclePtr().id()).arg(cyclePtr()->entityPtr->name);
			else
				return tr("IncomeCycleViewNameWithoutEntity").arg(cyclePtr().id());
		}
		return "IncomeCycleView";
	}

#define EntityColumnWidth 180
#define AmountColumnWidth 200
#define ExtraColumnWidth 180

	void EntryCycleList::load()
	{
		bool ld = !loaded();
		QueryModelFilteredList::load();

		if(ld)
		{
			int timestampColumn = viewIndexToColumn(ViewCreatedOn);
			if(timestampColumn != -1)
				_tableView->sortByColumn(timestampColumn, Wt::SortOrder::Descending);
		}
	}

	void EntryCycleList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "id"));
		filtersTemplate()->addFilterModel(make_shared<RangeFilterModel>(tr("Amount"), "amount")); filtersTemplate()->addFilter(2);
	}

	void IncomeCycleList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT c.id, c.timestamp, e.name, c.startDate, c.endDate, c.amount, COUNT(a.id), c.interval, c.nIntervals, e.id "
			"FROM " + IncomeCycle::tStr() + " c "
			"INNER JOIN " + Entity::tStr() + " e ON (e.id = c.entity_id) "
			"LEFT JOIN " + ClientAssignment::tStr() + " a ON (a.incomecycle_id = c.id)").groupBy("c.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "c.");

		if(_entityPtr.id() != -1)
			_baseQuery.where("c.entity_id = ?").bind(_entityPtr.id());

		model->setQuery(generateFilteredQuery());
		addColumn(ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("c.timestamp"), tr("CreatedOn"), DateTimeColumnWidth);

		if(_entityPtr.id() == -1)
			addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);

		addColumn(ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ViewExtra, model->addColumn("COUNT(a.id)"), tr("ClientAssignments"), ExtraColumnWidth);

		_proxyModel = make_shared<EntryCycleListProxyModel<IncomeCycleList>>(IncomeCycle::viewInternalPath(""), _model);
	}

	void ExpenseCycleList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT c.id, c.timestamp, e.name, c.startDate, c.endDate, c.amount, COUNT(a.id), c.interval, c.nIntervals, e.id "
			"FROM " + ExpenseCycle::tStr() + " c "
			"INNER JOIN " + Entity::tStr() + " e ON (e.id = c.entity_id) "
			"LEFT JOIN " + EmployeeAssignment::tStr() + " a ON (a.expensecycle_id = c.id)").groupBy("c.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "c.");

		if(_entityPtr.id() != -1)
			_baseQuery.where("c.entity_id = ?").bind(_entityPtr.id());

		model->setQuery(generateFilteredQuery());
		addColumn(ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("c.timestamp"), tr("CreatedOn"), DateTimeColumnWidth);

		if(_entityPtr.id() == -1)
			addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);

		addColumn(ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ViewExtra, model->addColumn("COUNT(a.id)"), tr("EmployeeAssignments"), ExtraColumnWidth);

		_proxyModel = make_shared<EntryCycleListProxyModel<ExpenseCycleList>>(ExpenseCycle::viewInternalPath(""), _model);
	}

// 	Wt::any IncomeCycleListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
// 	{
// 		return EntryCycleListProxyModel::data(idx, role);
// 	}
// 
// 	Wt::any ExpenseCycleListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
// 	{
// 		return EntryCycleListProxyModel::data(idx, role);
// 	}

	template<class FilteredList>
	void EntryCycleListProxyModel<FilteredList>::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	template<class FilteredList>
	Wt::any EntryCycleListProxyModel<FilteredList>::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	template<class FilteredList>
	Wt::WFlags<Wt::ItemFlag> EntryCycleListProxyModel<FilteredList>::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	template<class FilteredList>
	Wt::any EntryCycleListProxyModel<FilteredList>::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const auto &res = static_pointer_cast<Dbo::QueryModel<typename FilteredList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<FilteredList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, _pathPrefix + std::to_string(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		auto viewIndex = Wt::any_cast<int>(viewIndexData);

		const auto &res = static_pointer_cast<Dbo::QueryModel<typename FilteredList::ResultType>>(sourceModel())->resultRow(idx.row());

		if(viewIndex == EntryCycleList::ViewAmount && role == Wt::ItemDataRole::Display)
		{
			return rsEveryNIntervals(Money(std::get<FilteredList::ResAmount>(res), DEFAULT_CURRENCY),
				std::get<FilteredList::ResInterval>(res),
				std::get<FilteredList::ResNIntervals>(res));
		}

		if(viewIndex == EntryCycleList::ViewStartDate && role == Wt::ItemDataRole::Display)
		{
			const Wt::WDate &date = std::get<FilteredList::ResStartDate>(res);
			if(date.isValid() && date > Wt::WDate::currentServerDate())
				return tr("XNotStarted").arg(std::get<FilteredList::ResStartDate>(res).toString(Wt::WLocale::currentLocale().dateFormat()));
		}

		if(viewIndex == EntryCycleList::ViewEndDate && role == Wt::ItemDataRole::Display)
		{
			const Wt::WDate &date = std::get<FilteredList::ResEndDate>(res);
			if(date.isValid() && Wt::WDate::currentServerDate() >= date)
				return tr("XEnded").arg(date.toString(Wt::WLocale::currentLocale().dateFormat()));
		}
		if(viewIndex == EntryCycleList::ViewEntity && role == Wt::ItemDataRole::Link)
		{
			return Wt::WLink(Wt::LinkType::InternalPath, Entity::viewInternalPath(std::get<FilteredList::ResEntityId>(res)));
		}

		if(role == Wt::ItemDataRole::StyleClass)
		{
			const Wt::WDate &startDate = std::get<FilteredList::ResStartDate>(res);
			if(startDate.isValid() && startDate > Wt::WDate::currentServerDate())
				return "text-info";

			const Wt::WDate &endDate = std::get<FilteredList::ResEndDate>(res);
			if(endDate.isValid() && Wt::WDate::currentServerDate() >= endDate)
				return "text-muted";
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}


	//EXPLICIT SPECIALIZATIONS
	template class EntryCycleListProxyModel<IncomeCycleList>;
	template class EntryCycleListProxyModel<ExpenseCycleList>;

}
