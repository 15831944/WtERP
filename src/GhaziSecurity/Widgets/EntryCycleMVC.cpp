#include "Application/WApplication.h"
#include "Widgets/EntryCycleMVC.h"
#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
#include "Widgets/AccountMVC.h"
#include "Widgets/HRMVC.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WComboBox>
#include <Wt/WDateEdit>
#include <Wt/WIntValidator>
#include <Wt/WDoubleValidator>
#include <Wt/WTableView>
#include <Wt/WLengthValidator>

namespace GS
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

	template<class Dbo>
	void EntryCycleFormModel::updateCycleFromModel(Wt::WFormModel *model, Wt::Dbo::ptr<Dbo> ptr, bool newCycle)
	{
		ptr.modify()->entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(model->value(EntryCycleFormModel::entityField));
		ptr.modify()->startDate = boost::any_cast<Wt::WDate>(model->value(EntryCycleFormModel::startDateField));
		ptr.modify()->endDate = boost::any_cast<Wt::WDate>(model->value(EntryCycleFormModel::endDateField));
		ptr.modify()->interval = CycleInterval(boost::any_cast<int>(model->value(EntryCycleFormModel::intervalField)));
		ptr.modify()->nIntervals = Wt::WLocale::currentLocale().toInt(model->valueText(EntryCycleFormModel::nIntervalsField));
		if(newCycle)
			ptr.modify()->_amountInCents = Money(model->valueText(EntryCycleFormModel::amountField).toUTF8(), DEFAULT_CURRENCY).valueInCents();
		ptr.modify()->firstEntryAfterCycle = boost::any_cast<int>(model->value(EntryCycleFormModel::firstEntryAfterCycleField)) == 1;
	}

	Wt::WWidget *EntryCycleFormModel::createFormWidget(Wt::WFormModel *model, EntryCycleView *view, Wt::WFormModel::Field field)
	{
		if(field == entityField)
		{
			FindEntityEdit *findEntityEdit = new FindEntityEdit();
			FindEntityValidator *findEntityValidator = new FindEntityValidator(findEntityEdit, true);
			findEntityValidator->setModifyPermissionRequired(true);
			model->setValidator(EntryCycleFormModel::entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == startDateField)
		{
			auto startDateEdit = new Wt::WDateEdit();
			startDateEdit->validator()->setMandatory(true);
			model->setValidator(EntryCycleFormModel::startDateField, startDateEdit->validator());
			startDateEdit->changed().connect(boost::bind(&EntryCycleFormModel::updateEndDateValidator, model, view, true));
			return startDateEdit;
		}
		if(field == endDateField)
		{
			auto endDateEdit = new Wt::WDateEdit();
			endDateEdit->setPlaceholderText(Wt::WString::tr("EmptyEndDate"));
			auto endDateValidator = new CycleEndDateValidator(model);
			model->setValidator(EntryCycleFormModel::endDateField, endDateValidator);
			return endDateEdit;
		}
		if(field == intervalField)
		{
			auto intervalCombo = new Wt::WComboBox();
			intervalCombo->insertItem(DailyInterval, Wt::WString::tr("Days"));
			intervalCombo->insertItem(WeeklyInterval, Wt::WString::tr("Weeks"));
			intervalCombo->insertItem(MonthlyInterval, Wt::WString::tr("Months"));
			intervalCombo->insertItem(YearlyInterval, Wt::WString::tr("Years"));
			intervalCombo->setCurrentIndex(MonthlyInterval);
			intervalCombo->changed().connect(view, &EntryCycleView::handleIntervalChanged);
			intervalCombo->changed().connect(boost::bind(&EntryCycleFormModel::updateEndDateValidator, model, view, true));
			return intervalCombo;
		}
		if(field == amountField)
		{
			auto amountEdit = new Wt::WLineEdit();
			auto amountValidator = new Wt::WDoubleValidator();
			amountValidator->setBottom(0);
			amountValidator->setMandatory(true);
			model->setValidator(EntryCycleFormModel::amountField, amountValidator);
			return amountEdit;
		}
		if(field == nIntervalsField)
		{
			auto nIntervalsEdit = new Wt::WLineEdit();
			auto nIntervalsValidator = new Wt::WIntValidator();
			nIntervalsValidator->setBottom(1);
			nIntervalsValidator->setMandatory(true);
			model->setValidator(EntryCycleFormModel::nIntervalsField, nIntervalsValidator);
			nIntervalsEdit->changed().connect(view, &EntryCycleView::handleIntervalChanged);
			nIntervalsEdit->changed().connect(boost::bind(&EntryCycleFormModel::updateEndDateValidator, model, view, true));
			return nIntervalsEdit;
		}
		if(field == firstEntryAfterCycleField)
		{
			auto firstEntryOnEdit = new Wt::WComboBox();
			firstEntryOnEdit->insertItem(0, Wt::WString::tr("OnStartingDate"));
			firstEntryOnEdit->insertItem(1, Wt::WString());
			firstEntryOnEdit->changed().connect(boost::bind(&EntryCycleFormModel::updateEndDateValidator, model, view, true));
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

		const boost::any &startDateVal = model->value(EntryCycleFormModel::startDateField);
		if(startDateVal.empty())
			return;

		Wt::WDate startDate = boost::any_cast<Wt::WDate>(startDateVal);
		CycleInterval interval = (CycleInterval)boost::any_cast<int>(model->value(EntryCycleFormModel::intervalField));
		Wt::WString nIntervalsStr = Wt::asString(model->value(EntryCycleFormModel::nIntervalsField));
		int FEAC = boost::any_cast<int>(model->value(EntryCycleFormModel::firstEntryAfterCycleField));

		Wt::WDateValidator *endDateValidator = dynamic_cast<Wt::WDateValidator*>(model->validator(EntryCycleFormModel::endDateField));
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
				endDateBottom.setGregorianDate(addCycleInterval(boost::posix_time::ptime(endDateBottom.toGregorianDate()), interval, nIntervals).date());
			}
			catch(const boost::bad_lexical_cast &) {}
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

		if(!nIntervalsEdit->valueText().empty() && nIntervalsEdit->validate() == Wt::WValidator::Valid)
		{
			int nIntervals = Wt::WLocale::currentLocale().toInt(nIntervalsEdit->valueText());
			switch(intervalCombo->currentIndex())
			{
			case DailyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("AfterNDays", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case WeeklyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("AfterNWeeks", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case MonthlyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("AfterNMonths", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case YearlyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("AfterNYears", nIntervals).arg(nIntervalsEdit->valueText())); break;
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
			return Result(Invalid, Wt::WString::tr("Error"));
		}
		return Wt::WDateValidator::validate(input);
	}

	//EXPENSE CYCLE MODEL
	ExpenseCycleFormModel::ExpenseCycleFormModel(ExpenseCycleView *view, Wt::Dbo::ptr<ExpenseCycle> cyclePtr /*= Wt::Dbo::ptr<ExpenseCycle>()*/)
		: RecordFormModel(view, cyclePtr), _view(view)
	{
		EntryCycleFormModel::addFields(this);
		if(_recordPtr)
		{
			TRANSACTION(APP);
			EntryCycleFormModel::updateModelFromCycle(this, *_recordPtr);
		}
	}

	Wt::WWidget *ExpenseCycleFormModel::createFormWidget(Wt::WFormModel::Field field)
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
			_recordPtr = app->dboSession().add(new ExpenseCycle());
			_recordPtr.modify()->setCreatedByValues();
			newCycle = true;
		}

		EntryCycleFormModel::updateCycleFromModel(this, _recordPtr, newCycle);

		if(newCycle && _recordPtr->firstEntryAfterCycle == false)
			app->accountsDatabase().createPendingCycleEntry(_recordPtr, Wt::Dbo::ptr<AccountEntry>(), boost::posix_time::microsec_clock::local_time());

		_recordPtr.flush();
		t.commit();
		return true;
	}

	void ExpenseCycleFormModel::persistedHandler()
	{
		setReadOnly(EntryCycleFormModel::startDateField, true);

		_view->bindWidget("entries", new AccountEntryList(recordPtr()));
		_view->bindWidget("employeeAssignments", new EmployeeAssignmentList(recordPtr()));
	}

	//EXPENSE CYCLE VIEW
	ExpenseCycleView::ExpenseCycleView(Wt::Dbo::ptr<ExpenseCycle> cyclePtr)
		: EntryCycleView(tr("GS.Admin.ExpenseCycleView")), _tempPtr(cyclePtr)
	{ }

	void ExpenseCycleView::initView()
	{
		_model = new ExpenseCycleFormModel(this, _tempPtr);
		addFormModel("expense", _model);
	}

	Wt::WWidget *ExpenseCycleView::createFormWidget(Wt::WFormModel::Field field)
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
		return RecordFormView::viewName();
	}

	//INCOME CYCLE MODEL
	IncomeCycleFormModel::IncomeCycleFormModel(IncomeCycleView *view, Wt::Dbo::ptr<IncomeCycle> cyclePtr /*= Wt::Dbo::ptr<IncomeCycle>()*/)
		: RecordFormModel(view, cyclePtr), _view(view)
	{
		EntryCycleFormModel::addFields(this);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			EntryCycleFormModel::updateModelFromCycle(this, *_recordPtr);
		}

	}

	Wt::WWidget *IncomeCycleFormModel::createFormWidget(Wt::WFormModel::Field field)
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
			_recordPtr = app->dboSession().add(new IncomeCycle());
			_recordPtr.modify()->setCreatedByValues();
			newCycle = true;
		}

		EntryCycleFormModel::updateCycleFromModel(this, _recordPtr, newCycle);

		if(newCycle && _recordPtr->firstEntryAfterCycle == false)
			app->accountsDatabase().createPendingCycleEntry(_recordPtr, Wt::Dbo::ptr<AccountEntry>(), boost::posix_time::microsec_clock::local_time());

		t.commit();
		return true;
	}

	void IncomeCycleFormModel::persistedHandler()
	{
		setReadOnly(EntryCycleFormModel::startDateField, true);

		_view->bindWidget("entries", new AccountEntryList(recordPtr()));
		_view->bindWidget("clientAssignments", new ClientAssignmentList(recordPtr()));
	}

	//INCOME CYCLE VIEW
	IncomeCycleView::IncomeCycleView(Wt::Dbo::ptr<IncomeCycle> cyclePtr)
		: EntryCycleView(tr("GS.Admin.IncomeCycleView")), _tempPtr(cyclePtr)
	{ }

	void IncomeCycleView::initView()
	{
		_model = new IncomeCycleFormModel(this, _tempPtr);
		addFormModel("income", _model);
	}

	Wt::WWidget *IncomeCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(auto result = EntryCycleView::createFormWidget(field))
			return result;

		return nullptr;
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
		return RecordFormView::viewName();
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
				_tableView->sortByColumn(timestampColumn, Wt::DescendingOrder);
		}
	}

	void EntryCycleList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id"));
		filtersTemplate()->addFilterModel(new RangeFilterModel(tr("Amount"), "amount")); filtersTemplate()->addFilter(2);
	}

	void IncomeCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT c.id, c.timestamp, e.name, c.startDate, c.endDate, c.amount, COUNT(a.id), c.interval, c.nIntervals, e.id e_id "
			"FROM " + std::string(IncomeCycle::tableName()) + " c "
			"INNER JOIN " + std::string(Entity::tableName()) + " e ON (e.id = c.entity_id) "
			"LEFT JOIN " + ClientAssignment::tableName() + " a ON (a.incomecycle_id = c.id)").groupBy("c.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "c.");

		if(_entityPtr.id() != -1)
			_baseQuery.where("c.entity_id = ?").bind(_entityPtr.id());

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("c.timestamp"), tr("CreatedOn"), DateTimeColumnWidth);

		if(_entityPtr.id() == -1)
			addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);

		addColumn(ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ViewExtra, model->addColumn("COUNT(a.id)"), tr("ClientAssignments"), ExtraColumnWidth);

		_proxyModel = new EntryCycleListProxyModel<IncomeCycleList>(IncomeCycle::viewInternalPath(""), _model, _model);
	}

	void ExpenseCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT c.id, c.timestamp, e.name, c.startDate, c.endDate, c.amount, COUNT(a.id), c.interval, c.nIntervals, e.id e_id "
			"FROM " + std::string(ExpenseCycle::tableName()) + " c "
			"INNER JOIN " + std::string(Entity::tableName()) + " e ON (e.id = c.entity_id) "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " a ON (a.expensecycle_id = c.id)").groupBy("c.id");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "c.");

		if(_entityPtr.id() != -1)
			_baseQuery.where("c.entity_id = ?").bind(_entityPtr.id());

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("c.timestamp"), tr("CreatedOn"), DateTimeColumnWidth);

		if(_entityPtr.id() == -1)
			addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);

		addColumn(ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ViewExtra, model->addColumn("COUNT(a.id)"), tr("EmployeeAssignments"), ExtraColumnWidth);

		_proxyModel = new EntryCycleListProxyModel<ExpenseCycleList>(ExpenseCycle::viewInternalPath(""), _model, _model);
	}

// 	boost::any IncomeCycleListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
// 	{
// 		return EntryCycleListProxyModel::data(idx, role);
// 	}
// 
// 	boost::any ExpenseCycleListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
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
	boost::any EntryCycleListProxyModel<FilteredList>::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	template<class FilteredList>
	Wt::WFlags<Wt::ItemFlag> EntryCycleListProxyModel<FilteredList>::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	template<class FilteredList>
	boost::any EntryCycleListProxyModel<FilteredList>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const typename FilteredList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<typename FilteredList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<FilteredList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, _pathPrefix + boost::lexical_cast<std::string>(id));
			}
		}

		boost::any viewIndexData = headerData(idx.column(), Wt::Horizontal, Wt::ViewIndexRole);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = boost::any_cast<int>(viewIndexData);

		const typename FilteredList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<typename FilteredList::ResultType>*>(sourceModel())->resultRow(idx.row());

		if(viewIndex == EntryCycleList::ViewAmount && role == Wt::DisplayRole)
		{
			return rsEveryNIntervals(Money(boost::get<FilteredList::ResAmount>(res), DEFAULT_CURRENCY),
				boost::get<FilteredList::ResInterval>(res),
				boost::get<FilteredList::ResNIntervals>(res));
		}

		if(viewIndex == EntryCycleList::ViewStartDate && role == Wt::DisplayRole)
		{
			const Wt::WDate &date = boost::get<FilteredList::ResStartDate>(res);
			if(date.isValid() && date > Wt::WDate(boost::gregorian::day_clock::local_day()))
				return Wt::WString::tr("XNotStarted").arg(boost::get<FilteredList::ResStartDate>(res).toString(Wt::WLocale::currentLocale().dateFormat()));
		}

		if(viewIndex == EntryCycleList::ViewEndDate && role == Wt::DisplayRole)
		{
			const Wt::WDate &date = boost::get<FilteredList::ResEndDate>(res);
			if(date.isValid() && Wt::WDate(boost::gregorian::day_clock::local_day()) >= date)
				return Wt::WString::tr("XEnded").arg(date.toString(Wt::WLocale::currentLocale().dateFormat()));
		}
		if(viewIndex == EntryCycleList::ViewEntity && role == Wt::LinkRole)
		{
			const typename FilteredList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<typename FilteredList::ResultType>*>(sourceModel())->resultRow(idx.row());
			return Wt::WLink(Wt::WLink::InternalPath, Entity::viewInternalPath(boost::get<FilteredList::ResEntityId>(res)));
		}

		if(role == Wt::StyleClassRole)
		{
			const Wt::WDate &startDate = boost::get<FilteredList::ResStartDate>(res);
			if(startDate.isValid() && startDate > Wt::WDate(boost::gregorian::day_clock::local_day()))
				return "text-info";

			const Wt::WDate &endDate = boost::get<FilteredList::ResEndDate>(res);
			if(endDate.isValid() && Wt::WDate(boost::gregorian::day_clock::local_day()) >= endDate)
				return "text-muted";
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}


	//EXPLICIT SPECIALIZATIONS
	template class EntryCycleListProxyModel<IncomeCycleList>;
	template class EntryCycleListProxyModel<ExpenseCycleList>;

}
