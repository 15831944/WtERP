#include "Widgets/EntryCycleMVC.h"
#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
#include "Widgets/HRMVC.h"
#include "Widgets/FindRecordEdit.h"
#include "Application/WApplication.h"

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
	//POSITION VIEW
	const Wt::WFormModel::Field PositionFormModel::titleField = "title";

	PositionFormModel::PositionFormModel(PositionView *view, Wt::Dbo::ptr<EmployeePosition> positionPtr /*= Wt::Dbo::ptr<EmployeePosition>()*/)
		: RecordFormModel(view, positionPtr), _view(view)
	{
		addField(titleField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(titleField, Wt::WString::fromUTF8(_recordPtr->title));
		}
	}

	Wt::WWidget * PositionFormModel::createFormWidget(Field field)
	{
		if(field == titleField)
		{
			Wt::WLineEdit *title = new Wt::WLineEdit();
			title->setMaxLength(70);
			auto titleValidator = new Wt::WLengthValidator(0, 70);
			titleValidator->setMandatory(true);
			setValidator(titleField, titleValidator);
			return title;
		}
		return RecordFormModel::createFormWidget(field);
	}

	bool PositionFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_recordPtr)
			_recordPtr = app->dboSession().add(new EmployeePosition());

		_recordPtr.modify()->title = valueText(titleField).toUTF8();

		if(app->positionQueryModel())
			app->positionQueryModel()->reload();

		t.commit();
		return true;
	}

	PositionView::PositionView()
		: RecordFormView(tr("GS.Admin.PositionView"))
	{ }

	PositionView::PositionView(Wt::Dbo::ptr<EmployeePosition> positionPtr)
		: RecordFormView(tr("GS.Admin.PositionView")), _tempPtr(positionPtr)
	{ }

	void PositionView::init()
	{
		_model = new PositionFormModel(this, _tempPtr);
		addFormModel("position", _model);
	}
	
	//POSITION PROXY MODEL
	PositionProxyModel::PositionProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<EmployeePosition>> *sourceModel, Wt::WObject *parent /*= nullptr*/)
		: QueryProxyModel<Wt::Dbo::ptr<EmployeePosition>>(parent)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &PositionProxyModel::addAdditionalRows);
	}

	void PositionProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), Wt::WString::tr("SelectPosition"));
			setData(index(0, 0), false, Wt::AdditionalRowRole);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), Wt::WString::tr("AddNewX").arg(Wt::WString::tr("position")));
			setData(index(lastRow, 0), true, Wt::AdditionalRowRole);
		}
	}

	//SERVICE VIEW
	const Wt::WFormModel::Field ServiceFormModel::titleField = "title";

	ServiceFormModel::ServiceFormModel(ServiceView *view, Wt::Dbo::ptr<ClientService> servicePtr /*= Wt::Dbo::ptr<ClientService>()*/)
		: RecordFormModel(view, servicePtr), _view(view)
	{
		addField(titleField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(titleField, Wt::WString::fromUTF8(_recordPtr->title));
		}
	}

	Wt::WWidget *ServiceFormModel::createFormWidget(Field field)
	{
		if(field == titleField)
		{
			Wt::WLineEdit *title = new Wt::WLineEdit();
			title->setMaxLength(70);
			auto titleValidator = new Wt::WLengthValidator(0, 70);
			titleValidator->setMandatory(true);
			setValidator(titleField, titleValidator);
			return title;
		}
		return RecordFormModel::createFormWidget(field);
	}

	bool ServiceFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_recordPtr)
			_recordPtr = app->dboSession().add(new ClientService());

		_recordPtr.modify()->title = valueText(titleField).toUTF8();

		if(app->serviceQueryModel())
			app->serviceQueryModel()->reload();

		t.commit();
		return true;
	}

	ServiceView::ServiceView()
		: RecordFormView(tr("GS.Admin.ServiceView"))
	{ }

	ServiceView::ServiceView(Wt::Dbo::ptr<ClientService> servicePtr)
		: RecordFormView(tr("GS.Admin.ServiceView")), _tempPtr(servicePtr)
	{ }

	void ServiceView::init()
	{
		_model = new ServiceFormModel(this, _tempPtr);
		addFormModel("service", _model);
	}

	//SERVICE PROXY MODEL
	ServiceProxyModel::ServiceProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<ClientService>> *sourceModel, Wt::WObject *parent /*= nullptr*/)
		: QueryProxyModel<Wt::Dbo::ptr<ClientService>>(parent)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &ServiceProxyModel::addAdditionalRows);
	}

	void ServiceProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), Wt::WString::tr("SelectService"));
			setData(index(0, 0), false, Wt::AdditionalRowRole);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), Wt::WString::tr("AddNewX").arg(Wt::WString::tr("service")));
			setData(index(lastRow, 0), true, Wt::AdditionalRowRole);
		}
	}

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
		ptr.modify()->nIntervals = boost::lexical_cast<int>(model->valueText(EntryCycleFormModel::nIntervalsField).toUTF8());
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
			startDateEdit->setPlaceholderText(Wt::WLocale::currentLocale().dateFormat());
			auto startDateValidator = new Wt::WDateValidator();
			startDateValidator->setMandatory(true);
			startDateValidator->setBottom(Wt::WDate(boost::gregorian::day_clock::local_day()));
			model->setValidator(EntryCycleFormModel::startDateField, startDateValidator);
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
				int nIntervals = boost::lexical_cast<int>(nIntervalsStr.toUTF8());
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
			int nIntervals = boost::lexical_cast<int>(nIntervalsEdit->valueText());
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
	const Wt::WFormModel::Field ExpenseCycleFormModel::purposeField = "purpose";
	const Wt::WFormModel::Field ExpenseCycleFormModel::positionField = "position";

	ExpenseCycleFormModel::ExpenseCycleFormModel(ExpenseCycleView *view, Wt::Dbo::ptr<ExpenseCycle> cyclePtr /*= Wt::Dbo::ptr<ExpenseCycle>()*/)
		: RecordFormModel(view, cyclePtr), _view(view)
	{
		EntryCycleFormModel::addFields(this);
		addField(purposeField);
		addField(positionField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			EntryCycleFormModel::updateModelFromCycle(this, *_recordPtr);
			setValue(purposeField, (int)(_recordPtr->positionPtr ? ExpenseCycle::Salary : ExpenseCycle::UnspecifiedPurpose));
			setValue(positionField, _recordPtr->positionPtr);
			setVisible(positionField, _recordPtr->positionPtr ? true : false);
		}
		else
			setVisible(positionField, false);
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

		const boost::any &positionVal = value(positionField);
		if(positionVal.empty())
			_recordPtr.modify()->positionPtr = Wt::Dbo::ptr<EmployeePosition>();
		else
			_recordPtr.modify()->positionPtr = boost::any_cast<Wt::Dbo::ptr<EmployeePosition>>(positionVal);

		if(newCycle && _recordPtr->firstEntryAfterCycle == false)
			app->accountsDatabase().createPendingCycleEntry(_recordPtr, Wt::Dbo::ptr<AccountEntry>(), boost::posix_time::microsec_clock::local_time());

		_recordPtr.flush();
		t.commit();
		return true;
	}

	//EXPENSE CYCLE VIEW
	ExpenseCycleView::ExpenseCycleView(Wt::Dbo::ptr<ExpenseCycle> cyclePtr)
		: EntryCycleView(tr("GS.Admin.ExpenseCycleView")), _tempPtr(cyclePtr)
	{ }

	void ExpenseCycleView::init()
	{
		_model = new ExpenseCycleFormModel(this, _tempPtr);
		addFormModel("expense", _model);
	}

	Wt::WWidget *ExpenseCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(auto result = EntryCycleView::createFormWidget(field))
			return result;

		if(field == ExpenseCycleFormModel::positionField)
		{
			WApplication *app = APP;
			app->initPositionQueryModel();
			_positionCombo = new QueryProxyModelCB<PositionProxyModel>(app->positionProxyModel());
			auto validator = new ProxyModelCBValidator<PositionProxyModel>(_positionCombo);
			validator->setErrorString(tr("MustSelectPosition"));
			_model->setValidator(field, validator);
			_positionCombo->changed().connect(this, &ExpenseCycleView::handlePositionChanged);
			return _positionCombo;
		}
		if(field == ExpenseCycleFormModel::purposeField)
		{
			_purposeCombo = new Wt::WComboBox();
			_purposeCombo->insertItem(ExpenseCycle::UnspecifiedPurpose, Wt::boost_any_traits<ExpenseCycle::Purpose>::asString(ExpenseCycle::UnspecifiedPurpose, ""));
			_purposeCombo->insertItem(ExpenseCycle::Salary, Wt::boost_any_traits<ExpenseCycle::Purpose>::asString(ExpenseCycle::Salary, ""));
			_purposeCombo->changed().connect(this, &ExpenseCycleView::handlePurposeChanged);
			return _purposeCombo;
		}
		return nullptr;
	}

	void ExpenseCycleView::handlePurposeChanged()
	{
		updateModelField(_model, ExpenseCycleFormModel::positionField);
		_model->setVisible(ExpenseCycleFormModel::positionField, _purposeCombo->currentIndex() == ExpenseCycle::Salary);
		updateViewField(_model, ExpenseCycleFormModel::positionField);
	}

	void ExpenseCycleView::handlePositionChanged()
	{
		boost::any v = _positionCombo->model()->index(_positionCombo->currentIndex(), 0).data(Wt::AdditionalRowRole);
		if(v.empty())
			return;
		
		if(boost::any_cast<bool>(v) == true)
			createAddPositionDialog();
	}

	Wt::WDialog *ExpenseCycleView::createAddPositionDialog()
	{
		Wt::WDialog *dialog = new Wt::WDialog(tr("AddNewX").arg(tr("position")), this);
		dialog->setClosable(true);
		dialog->setTransient(true);
		dialog->rejectWhenEscapePressed(true);
		dialog->setDeleteWhenHidden(true);
		dialog->setWidth(Wt::WLength(500));
		PositionView *positionView = new PositionView();
		dialog->contents()->addWidget(positionView);

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			if(code == Wt::WDialog::Rejected)
			{
				_model->setValue(ExpenseCycleFormModel::positionField, Wt::Dbo::ptr<EmployeePosition>());
				updateViewField(_model, ExpenseCycleFormModel::positionField);
			}
		}, std::placeholders::_1));

		positionView->submitted().connect(std::bind([=]() {
			_model->setValue(ExpenseCycleFormModel::positionField, positionView->positionPtr());
			updateViewField(_model, ExpenseCycleFormModel::positionField);
			dialog->accept();
		}));

		dialog->show();
		return dialog;
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
	const Wt::WFormModel::Field IncomeCycleFormModel::purposeField = "purpose";
	const Wt::WFormModel::Field IncomeCycleFormModel::serviceField = "service";

	IncomeCycleFormModel::IncomeCycleFormModel(IncomeCycleView *view, Wt::Dbo::ptr<IncomeCycle> cyclePtr /*= Wt::Dbo::ptr<IncomeCycle>()*/)
		: RecordFormModel(view, cyclePtr), _view(view)
	{
		EntryCycleFormModel::addFields(this);
		addField(purposeField);
		addField(serviceField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			EntryCycleFormModel::updateModelFromCycle(this, *_recordPtr);
			setValue(purposeField, (int)(_recordPtr->servicePtr ? IncomeCycle::Services : IncomeCycle::UnspecifiedPurpose));
			setValue(serviceField, _recordPtr->servicePtr);
			setVisible(serviceField, _recordPtr->servicePtr ? true : false);
		}
		else
			setVisible(serviceField, false);

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

		const boost::any &serviceVal = value(serviceField);
		if(serviceVal.empty())
			_recordPtr.modify()->servicePtr = Wt::Dbo::ptr<ClientService>();
		else
			_recordPtr.modify()->servicePtr = boost::any_cast<Wt::Dbo::ptr<ClientService>>(serviceVal);

		if(newCycle && _recordPtr->firstEntryAfterCycle == false)
			app->accountsDatabase().createPendingCycleEntry(_recordPtr, Wt::Dbo::ptr<AccountEntry>(), boost::posix_time::microsec_clock::local_time());

		t.commit();
		return true;
	}

	//INCOME CYCLE VIEW
	IncomeCycleView::IncomeCycleView(Wt::Dbo::ptr<IncomeCycle> cyclePtr)
		: EntryCycleView(tr("GS.Admin.IncomeCycleView")), _tempPtr(cyclePtr)
	{ }

	void IncomeCycleView::init()
	{
		_model = new IncomeCycleFormModel(this, _tempPtr);
		addFormModel("income", _model);
	}

	Wt::WWidget *IncomeCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(auto result = EntryCycleView::createFormWidget(field))
			return result;

		if(field == IncomeCycleFormModel::serviceField)
		{
			delete _serviceCombo;
			WApplication *app = APP;
			app->initServiceQueryModel();
			_serviceCombo = new QueryProxyModelCB<ServiceProxyModel>(app->serviceProxyModel());
			auto validator = new ProxyModelCBValidator<ServiceProxyModel>(_serviceCombo);
			validator->setErrorString(tr("MustSelectService"));
			_model->setValidator(field, validator);
			_serviceCombo->changed().connect(this, &IncomeCycleView::handleServiceChanged);
			return _serviceCombo;
		}
		if(field == IncomeCycleFormModel::purposeField)
		{
			_purposeCombo = new Wt::WComboBox();
			_purposeCombo->insertItem(IncomeCycle::UnspecifiedPurpose, Wt::boost_any_traits<IncomeCycle::Purpose>::asString(IncomeCycle::UnspecifiedPurpose, ""));
			_purposeCombo->insertItem(IncomeCycle::Services, Wt::boost_any_traits<IncomeCycle::Purpose>::asString(IncomeCycle::Services, ""));
			_purposeCombo->changed().connect(this, &IncomeCycleView::handlePurposeChanged);
			return _purposeCombo;
		}
		return nullptr;
	}

	void IncomeCycleView::handlePurposeChanged()
	{
		updateModelField(_model, IncomeCycleFormModel::serviceField);
		_model->setVisible(IncomeCycleFormModel::serviceField, _purposeCombo->currentIndex() == ExpenseCycle::Salary);
		updateViewField(_model, IncomeCycleFormModel::serviceField);
	}

	void IncomeCycleView::handleServiceChanged()
	{
		boost::any v = _serviceCombo->model()->index(_serviceCombo->currentIndex(), 0).data(Wt::AdditionalRowRole);
		if(v.empty())
			return;

		if(boost::any_cast<bool>(v) == true)
			createAddServiceDialog();
	}

	Wt::WDialog *IncomeCycleView::createAddServiceDialog()
	{
		updateModel(_model);
		Wt::WDialog *dialog = new Wt::WDialog(tr("AddNewX").arg(tr("service")), this);
		dialog->setTransient(true);
		dialog->rejectWhenEscapePressed(true);
		dialog->setDeleteWhenHidden(true);
		dialog->setClosable(true);
		dialog->setWidth(Wt::WLength(500));
		ServiceView *serviceView = new ServiceView();
		dialog->contents()->addWidget(serviceView);

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			if(code == Wt::WDialog::Rejected)
			{
				_model->setValue(IncomeCycleFormModel::serviceField, Wt::Dbo::ptr<ClientService>());
				updateViewField(_model, IncomeCycleFormModel::serviceField);
			}
		}, std::placeholders::_1));

		serviceView->submitted().connect(std::bind([=]() {
			_model->setValue(IncomeCycleFormModel::serviceField, serviceView->servicePtr());
			updateViewField(_model, IncomeCycleFormModel::serviceField);
			dialog->accept();
		}));

		dialog->show();
		return dialog;
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

#define IdColumnWidth 80
#define TimestampColumnWidth 160
#define DateColumnWidth 130
#define EntityColumnWidth 180
#define AmountColumnWidth 200
#define ExtraColumnWidth 180

	EntryCycleList::EntryCycleList()
		: QueryModelFilteredList()
	{ }

	EntityEntryCycleList::EntityEntryCycleList()
		: QueryModelFilteredList()
	{ }

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

	void EntityEntryCycleList::load()
	{
		bool ld = !loaded();
		QueryModelFilteredList::load();

		if(ld)
		{
			int timestampColumn = viewIndexToColumn(EntryCycleList::ViewCreatedOn);
			if(timestampColumn != -1)
				_tableView->sortByColumn(timestampColumn, Wt::DescendingOrder);
		}
	}

	void EntryCycleList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id"));
		filtersTemplate()->addFilterModel(new RangeFilterModel(tr("Amount"), "amount")); filtersTemplate()->addFilter(2);
	}

	void EntityEntryCycleList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "id"));
		filtersTemplate()->addFilterModel(new RangeFilterModel(tr("Amount"), "amount")); filtersTemplate()->addFilter(2);
	}

	IncomeCycleList::IncomeCycleList()
		: EntryCycleList()
	{ }

	EntityIncomeCycleList::EntityIncomeCycleList(Wt::Dbo::ptr<Entity> entityPtr)
		: EntityEntryCycleList(), _entityPtr(entityPtr)
	{ }

	void IncomeCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT c.id, c.timestamp, e.name, c.startDate, c.endDate, c.amount, s.title, c.interval, c.nIntervals, e.id e_id "
			"FROM " + std::string(IncomeCycle::tableName()) + " c "
			"INNER JOIN " + std::string(Entity::tableName()) + " e ON (e.id = c.entity_id) "
			"LEFT JOIN " + ClientService::tableName() + " s ON (s.id = c.clientservice_id)");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "c.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("c.timestamp"), tr("CreatedOn"), TimestampColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);
		addColumn(ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ViewExtra, model->addColumn("s.title"), tr("ClientService"), ExtraColumnWidth);

		_proxyModel = new EntryCycleListProxyModel<IncomeCycleList>(IncomeCycle::viewInternalPath(""), _model, _model);
	}

	void EntityIncomeCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT c.id, c.timestamp, c.startDate, c.endDate, c.amount, s.title, c.interval, c.nIntervals "
			"FROM " + std::string(IncomeCycle::tableName()) + " c "
			"LEFT JOIN " + ClientService::tableName() + " s ON (s.id = c.clientservice_id)")
			.where("c.entity_id = ?").bind(_entityPtr.id());
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "c.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(EntryCycleList::ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(EntryCycleList::ViewCreatedOn, model->addColumn("c.timestamp"), tr("CreatedOn"), TimestampColumnWidth);
		addColumn(EntryCycleList::ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(EntryCycleList::ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(EntryCycleList::ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(EntryCycleList::ViewExtra, model->addColumn("s.title"), tr("ClientService"), ExtraColumnWidth);

		_proxyModel = new BaseEntryCycleListProxyModel<EntityIncomeCycleList>(IncomeCycle::viewInternalPath(""), _model, _model);
	}

	ExpenseCycleList::ExpenseCycleList()
		: EntryCycleList()
	{ }

	EntityExpenseCycleList::EntityExpenseCycleList(Wt::Dbo::ptr<Entity> entityPtr)
		: EntityEntryCycleList(), _entityPtr(entityPtr)
	{ }

	void ExpenseCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT c.id, c.timestamp, e.name, c.startDate, c.endDate, c.amount, p.title, c.interval, c.nIntervals, e.id e_id "
			"FROM " + std::string(ExpenseCycle::tableName()) + " c "
			"INNER JOIN " + std::string(Entity::tableName()) + " e ON (e.id = c.entity_id) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = c.employeeposition_id)");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "c.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("c.timestamp"), tr("CreatedOn"), TimestampColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);
		addColumn(ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ViewExtra, model->addColumn("p.title"), tr("EmployeePosition"), ExtraColumnWidth);

		_proxyModel = new EntryCycleListProxyModel<ExpenseCycleList>(ExpenseCycle::viewInternalPath(""), _model, _model);
	}

	void EntityExpenseCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT c.id, c.timestamp, c.startDate, c.endDate, c.amount, p.title, c.interval, c.nIntervals "
			"FROM " + std::string(ExpenseCycle::tableName()) + " c "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = c.employeeposition_id)")
			.where("c.entity_id = ?").bind(_entityPtr.id());
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "c.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ExpenseCycleList::ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ExpenseCycleList::ViewCreatedOn, model->addColumn("c.timestamp"), tr("CreatedOn"), TimestampColumnWidth);
		addColumn(ExpenseCycleList::ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ExpenseCycleList::ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ExpenseCycleList::ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ExpenseCycleList::ViewExtra, model->addColumn("p.title"), tr("EmployeePosition"), ExtraColumnWidth);

		_proxyModel = new BaseEntryCycleListProxyModel<EntityExpenseCycleList>(ExpenseCycle::viewInternalPath(""), _model, _model);
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
	void BaseEntryCycleListProxyModel<FilteredList>::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	template<class FilteredList>
	boost::any BaseEntryCycleListProxyModel<FilteredList>::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
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
	Wt::WFlags<Wt::ItemFlag> BaseEntryCycleListProxyModel<FilteredList>::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	template<class FilteredList>
	boost::any BaseEntryCycleListProxyModel<FilteredList>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
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
			Wt::WString str;
			switch(boost::get<FilteredList::ResInterval>(res))
			{
			case DailyInterval: str = Wt::WString::trn("RsEveryNDays", boost::get<FilteredList::ResNIntervals>(res)); break;
			case WeeklyInterval: str = Wt::WString::trn("RsEveryNWeeks", boost::get<FilteredList::ResNIntervals>(res)); break;
			case MonthlyInterval: str = Wt::WString::trn("RsEveryNMonths", boost::get<FilteredList::ResNIntervals>(res)); break;
			case YearlyInterval: str = Wt::WString::trn("RsEveryNYears", boost::get<FilteredList::ResNIntervals>(res)); break;
			default: return boost::any();
			}

			str.arg(Wt::WLocale::currentLocale().toString(Money(boost::get<FilteredList::ResAmount>(res), DEFAULT_CURRENCY)));
			str.arg(boost::get<FilteredList::ResNIntervals>(res));
			return str;
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

	template<class FilteredList>
	boost::any EntryCycleListProxyModel<FilteredList>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		boost::any viewIndexData = headerData(idx.column(), Wt::Horizontal, Wt::ViewIndexRole);
		if(viewIndexData.empty())
			return BaseEntryCycleListProxyModel<FilteredList>::data(idx, role);
		int viewIndex = boost::any_cast<int>(viewIndexData);

		if(viewIndex == EntryCycleList::ViewEntity && role == Wt::LinkRole)
		{
			const typename FilteredList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<typename FilteredList::ResultType>*>(sourceModel())->resultRow(idx.row());
			return Wt::WLink(Wt::WLink::InternalPath, Entity::viewInternalPath(boost::get<FilteredList::ResEntityId>(res)));
		}

		return BaseEntryCycleListProxyModel<FilteredList>::data(idx, role);
	}

	//EXPLICIT SPECIALIZATIONS
	template class BaseEntryCycleListProxyModel<EntityIncomeCycleList>;
	template class BaseEntryCycleListProxyModel<EntityExpenseCycleList>;
	template class EntryCycleListProxyModel<IncomeCycleList>;
	template class EntryCycleListProxyModel<ExpenseCycleList>;

}
