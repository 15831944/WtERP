#include "Widgets/EntryCycleMVC.h"
#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
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
	const Wt::WFormModel::Field PositionView::titleField = "title";

	PositionView::PositionView(Wt::WContainerWidget *parent /*= nullptr*/)
		: MyTemplateFormView(tr("GS.Admin.PositionView"), parent)
	{
		_model = new Wt::WFormModel(this);
		model()->addField(titleField);

		Wt::WLineEdit *title = new Wt::WLineEdit();
		title->setMaxLength(70);
		auto titleValidator = new Wt::WLengthValidator(0, 70);
		titleValidator->setMandatory(true);
		model()->setValidator(titleField, titleValidator);
		setFormWidget(titleField, title);

		Wt::WPushButton *submit = new Wt::WPushButton(tr("Submit"));
		submit->clicked().connect(this, &PositionView::submit);
		bindWidget("submit", submit);

		updateView(model());
	}

	void PositionView::submit()
	{
		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		updateModel(model());
		if(!model()->validate())
		{
			updateView(model());
			return;
		}

		try
		{
			if(!_positionPtr)
				_positionPtr = app->session().add(new EmployeePosition());

			_positionPtr.modify()->title = model()->valueText(titleField).toUTF8();

			if(app->positionQueryModel())
				app->positionQueryModel()->reload();

			t.commit();

			updateView(model());
			submitted().emit();
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "PositionView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
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
			setData(index(lastRow, 0), Wt::WString::tr("AddNewPosition"));
			setData(index(lastRow, 0), true, Wt::AdditionalRowRole);
		}
	}

	//SERVICE VIEW
	const Wt::WFormModel::Field ServiceView::titleField = "title";

	ServiceView::ServiceView(Wt::WContainerWidget *parent /*= nullptr*/)
		: MyTemplateFormView(tr("GS.Admin.ServiceView"), parent)
	{
		_model = new Wt::WFormModel(this);
		model()->addField(titleField);

		Wt::WLineEdit *title = new Wt::WLineEdit();
		title->setMaxLength(70);
		auto titleValidator = new Wt::WLengthValidator(0, 70);
		titleValidator->setMandatory(true);
		model()->setValidator(titleField, titleValidator);
		setFormWidget(titleField, title);

		Wt::WPushButton *submit = new Wt::WPushButton(tr("Submit"));
		submit->clicked().connect(this, &ServiceView::submit);
		bindWidget("submit", submit);

		updateView(model());
	}

	void ServiceView::submit()
	{
		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		updateModel(model());
		if(!model()->validate())
		{
			updateView(model());
			return;
		}

		try
		{
			if(!_servicePtr)
				_servicePtr = app->session().add(new ClientService());

			_servicePtr.modify()->title = model()->valueText(titleField).toUTF8();

			if(app->serviceQueryModel())
				app->serviceQueryModel()->reload();

			t.commit();

			updateView(model());
			submitted().emit();
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "ServiceView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
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
			setData(index(lastRow, 0), Wt::WString::tr("AddNewService"));
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

	EntryCycleFormModel::EntryCycleFormModel(Wt::WObject *parent)
		: Wt::WFormModel(parent)
	{
		addField(entityField);
		addField(startDateField);
		addField(endDateField);
		addField(intervalField);
		addField(nIntervalsField);
		addField(amountField);
		addField(firstEntryAfterCycleField);
	}

	//ENTRY CYCLE VIEW
	void EntryCycleView::initEntryCycleView(Wt::WFormModel *model)
	{
		FindEntityEdit *entityFindEdit = new FindEntityEdit();
		setFormWidget(EntryCycleFormModel::entityField, entityFindEdit);
		bindWidget("new-entity", entityFindEdit->newEntityBtn());
		bindWidget("list-entity", entityFindEdit->showListBtn());
		model->setValidator(EntryCycleFormModel::entityField, new FindEntityValidator(entityFindEdit, true));

		auto startDateEdit = new Wt::WDateEdit();
		startDateEdit->setPlaceholderText(APP->locale().dateFormat());
		auto startDateValidator = new Wt::WDateValidator();
		startDateValidator->setMandatory(true);
		startDateValidator->setBottom(Wt::WDate(boost::gregorian::day_clock::local_day()));
		model->setValidator(EntryCycleFormModel::startDateField, startDateValidator);
		setFormWidget(EntryCycleFormModel::startDateField, startDateEdit);

		auto endDateEdit = new Wt::WDateEdit();
		endDateEdit->setPlaceholderText(tr("EmptyEndDate"));
		auto endDateValidator = new Wt::WDateValidator();
		model->setValidator(EntryCycleFormModel::endDateField, endDateValidator);

		startDateEdit->changed().connect(std::bind([startDateEdit, endDateValidator]() {
			endDateValidator->setBottom(startDateEdit->date().addDays(1));
		}));
		setFormWidget(EntryCycleFormModel::endDateField, endDateEdit);

		auto intervalCombo = new Wt::WComboBox();
		intervalCombo->insertItem(DailyInterval, tr("Days"));
		intervalCombo->insertItem(WeeklyInterval, tr("Weeks"));
		intervalCombo->insertItem(MonthlyInterval, tr("Months"));
		intervalCombo->insertItem(YearlyInterval, tr("Years"));
		intervalCombo->setCurrentIndex(MonthlyInterval);
		setFormWidget(EntryCycleFormModel::intervalField, intervalCombo);

		auto amountEdit = new Wt::WLineEdit();
		auto amountValidator = new Wt::WDoubleValidator();
		amountValidator->setBottom(0);
		amountValidator->setMandatory(true);
		model->setValidator(EntryCycleFormModel::amountField, amountValidator);
		setFormWidget(EntryCycleFormModel::amountField, amountEdit);

		auto nIntervalsEdit = new Wt::WLineEdit();
		auto nIntervalsValidator = new Wt::WIntValidator();
		nIntervalsValidator->setBottom(1);
		nIntervalsValidator->setMandatory(true);
		model->setValidator(EntryCycleFormModel::nIntervalsField, nIntervalsValidator);
		setFormWidget(EntryCycleFormModel::nIntervalsField, nIntervalsEdit);

		auto firstEntryOnEdit = new Wt::WComboBox();
		firstEntryOnEdit->insertItem(0, tr("OnStartingDate"));
		firstEntryOnEdit->insertItem(1, Wt::WString());
		setFormWidget(EntryCycleFormModel::firstEntryAfterCycleField, firstEntryOnEdit);

		nIntervalsEdit->changed().connect(this, &EntryCycleView::handleIntervalChanged);
		intervalCombo->changed().connect(this, &EntryCycleView::handleIntervalChanged);
		handleIntervalChanged();

		Wt::WPushButton *submit = new Wt::WPushButton(tr("Submit"));
		submit->clicked().connect(this, &EntryCycleView::submit);
		bindWidget("submit", submit);
	}

	void EntryCycleView::handleIntervalChanged()
	{
		auto nIntervalsEdit = resolve<Wt::WLineEdit*>(EntryCycleFormModel::nIntervalsField);
		auto intervalCombo = resolve<Wt::WComboBox*>(EntryCycleFormModel::intervalField);
		auto firstEntryOnEdit = resolve<Wt::WComboBox*>(EntryCycleFormModel::firstEntryAfterCycleField);

		firstEntryOnEdit->setItemText(1, tr("AfterTheInterval"));

		if(!nIntervalsEdit->valueText().empty() && nIntervalsEdit->validate() == Wt::WValidator::Valid)
		{
			int nIntervals = boost::lexical_cast<int>(nIntervalsEdit->valueText());
			switch(intervalCombo->currentIndex())
			{
			case DailyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("AfterNDays", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case WeeklyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("AfterNWeeks", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case MonthlyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("AfterNMonths", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case YearlyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("AfterNYears", nIntervals).arg(nIntervalsEdit->valueText())); break;
			}
		}
	}

	//EXPENSE CYCLE MODEL
	const Wt::WFormModel::Field ExpenseCycleFormModel::purposeField = "purpose";
	const Wt::WFormModel::Field ExpenseCycleFormModel::positionField = "position";

	ExpenseCycleFormModel::ExpenseCycleFormModel(ExpenseCycleView *view, Wt::Dbo::ptr<ExpenseCycle> cyclePtr /*= Wt::Dbo::ptr<ExpenseCycle>()*/)
		: EntryCycleFormModel(view), _view(view), _cyclePtr(cyclePtr)
	{
		addField(purposeField);
		addField(positionField);

		if(cyclePtr)
		{
			setValue(entityField, cyclePtr->entityPtr);
			setValue(startDateField, cyclePtr->startDate);
			setValue(endDateField, cyclePtr->endDate);
			setValue(purposeField, (int)(cyclePtr->positionPtr ? ExpenseCycle::Salary : ExpenseCycle::UnspecifiedPurpose));
			setValue(positionField, cyclePtr->positionPtr);
			setValue(intervalField, (int)cyclePtr->interval);
			setValue(nIntervalsField, cyclePtr->nIntervals);
			setValue(amountField, cyclePtr->amount);
			setValue(firstEntryAfterCycleField, cyclePtr->firstEntryAfterCycle ? 1 : 0);
		}
	}

	void ExpenseCycleFormModel::saveChanges()
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		bool newCycle = false;
		if(!_cyclePtr)
		{
			_cyclePtr = app->session().add(new ExpenseCycle());
			newCycle = true;
		}

		_cyclePtr.modify()->entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		_cyclePtr.modify()->startDate = boost::any_cast<Wt::WDate>(value(startDateField));
		_cyclePtr.modify()->endDate = boost::any_cast<Wt::WDate>(value(endDateField));
		_cyclePtr.modify()->interval = CycleInterval(boost::any_cast<int>(value(intervalField)));
		_cyclePtr.modify()->nIntervals = boost::lexical_cast<int>(valueText(nIntervalsField).toUTF8());
		_cyclePtr.modify()->amount = boost::lexical_cast<double>(valueText(amountField).toUTF8());
		_cyclePtr.modify()->firstEntryAfterCycle = boost::any_cast<int>(value(firstEntryAfterCycleField)) == 1;

		const boost::any &positionVal = value(positionField);
		if(positionVal.empty())
			_cyclePtr.modify()->positionPtr = Wt::Dbo::ptr<EmployeePosition>();
		else
			_cyclePtr.modify()->positionPtr = boost::any_cast<Wt::Dbo::ptr<EmployeePosition>>(positionVal);

		if(newCycle && _cyclePtr->firstEntryAfterCycle == false)
			app->accountsDatabase().createPendingCycleEntry(_cyclePtr, Wt::Dbo::ptr<AccountEntry>(), boost::posix_time::microsec_clock::local_time());

		t.commit();
	}

	//EXPENSE CYCLE VIEW
	ExpenseCycleView::ExpenseCycleView(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::WContainerWidget *parent /*= nullptr*/)
		: EntryCycleView(tr("GS.Admin.ExpenseCycleView"), parent)
	{
		_model = new ExpenseCycleFormModel(this, cyclePtr);
		initEntryCycleView(_model);

		WApplication *app = WApplication::instance();
		app->initPositionQueryModel();

		_purposeCombo = new Wt::WComboBox();
		_purposeCombo->insertItem(ExpenseCycle::UnspecifiedPurpose, Wt::boost_any_traits<ExpenseCycle::Purpose>::asString(ExpenseCycle::UnspecifiedPurpose, ""));
		_purposeCombo->insertItem(ExpenseCycle::Salary, Wt::boost_any_traits<ExpenseCycle::Purpose>::asString(ExpenseCycle::Salary, ""));
		_purposeCombo->changed().connect(this, &ExpenseCycleView::handlePurposeChanged);
		setFormWidget(ExpenseCycleFormModel::purposeField, _purposeCombo);

		model()->setVisible(ExpenseCycleFormModel::positionField, false);
		updateView(model());
	}

	Wt::WWidget *ExpenseCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == ExpenseCycleFormModel::positionField)
		{
			delete _positionCombo;
			_positionCombo = new ProxyModelComboBox<PositionProxyModel>(APP->positionProxyModel());
			auto validator = new ProxyModelComboBoxValidator<PositionProxyModel>(_positionCombo);
			validator->setErrorString(tr("MustSelectPosition"));
			model()->setValidator(field, validator);
			_positionCombo->changed().connect(this, &ExpenseCycleView::handlePositionChanged);
			return _positionCombo;
		}
		return nullptr;
	}

	void ExpenseCycleView::handlePurposeChanged()
	{
		updateModel(model());
		model()->setVisible(ExpenseCycleFormModel::positionField, _purposeCombo->currentIndex() == ExpenseCycle::Salary);
		updateView(model());
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
		updateModel(model());
		Wt::WDialog *dialog = new Wt::WDialog(tr("AddNewPosition"), this);
		dialog->setClosable(true);
		dialog->setTransient(true);
		dialog->rejectWhenEscapePressed(true);
		dialog->setWidth(Wt::WLength(500));
		PositionView *positionView = new PositionView(dialog->contents());

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			delete dialog;
			if(code == Wt::WDialog::Rejected)
			{
				updateModel(model());
				model()->setValue(ExpenseCycleFormModel::positionField, Wt::Dbo::ptr<EmployeePosition>());
				updateView(model());
			}
		}, std::placeholders::_1));

		positionView->submitted().connect(std::bind([=]() {
			updateModel(model());
			model()->setValue(ExpenseCycleFormModel::positionField, positionView->positionPtr());
			updateView(model());
			dialog->accept();
		}));

		dialog->show();
		return dialog;
	}

	Wt::WString ExpenseCycleView::viewName() const
	{
		Wt::Dbo::Transaction t(APP->session());
		if(cyclePtr())
		{
			if(cyclePtr()->entityPtr)
				return tr("ExpenseCycleViewName").arg(cyclePtr().id()).arg(cyclePtr()->entityPtr->name);
			else
				return tr("ExpenseCycleViewNameWithoutEntity").arg(cyclePtr().id());
		}
		return MyTemplateFormView::viewName();
	}

	void ExpenseCycleView::submit()
	{
		WApplication *app = APP;
		Wt::Dbo::Transaction t(app->session());

		updateModel(model());
		if(!model()->validate())
		{
			updateView(model());
			return;
		}

		try
		{
			model()->saveChanges();
			t.commit();

			updateView(model());
			submitted().emit();
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "ExpenseCycleView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	//INCOME CYCLE MODEL
	const Wt::WFormModel::Field IncomeCycleFormModel::purposeField = "purpose";
	const Wt::WFormModel::Field IncomeCycleFormModel::serviceField = "service";

	IncomeCycleFormModel::IncomeCycleFormModel(IncomeCycleView *view, Wt::Dbo::ptr<IncomeCycle> cyclePtr /*= Wt::Dbo::ptr<IncomeCycle>()*/)
		: EntryCycleFormModel(view), _view(view), _cyclePtr(cyclePtr)
	{
		addField(purposeField);
		addField(serviceField);

		if(cyclePtr)
		{
			setValue(entityField, cyclePtr->entityPtr);
			setValue(startDateField, cyclePtr->startDate);
			setValue(endDateField, cyclePtr->endDate);
			setValue(purposeField, (int)(cyclePtr->servicePtr ? IncomeCycle::Services : IncomeCycle::UnspecifiedPurpose));
			setValue(serviceField, cyclePtr->servicePtr);
			setValue(intervalField, (int)cyclePtr->interval);
			setValue(nIntervalsField, cyclePtr->nIntervals);
			setValue(amountField, cyclePtr->amount);
			setValue(firstEntryAfterCycleField, cyclePtr->firstEntryAfterCycle ? 1 : 0);
		}
	}

	void IncomeCycleFormModel::saveChanges()
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		bool newCycle = false;
		if(!_cyclePtr)
		{
			_cyclePtr = app->session().add(new IncomeCycle());
			newCycle = true;
		}

		_cyclePtr.modify()->entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		_cyclePtr.modify()->startDate = boost::any_cast<Wt::WDate>(value(startDateField));
		_cyclePtr.modify()->endDate = boost::any_cast<Wt::WDate>(value(endDateField));
		_cyclePtr.modify()->interval = CycleInterval(boost::any_cast<int>(value(intervalField)));
		_cyclePtr.modify()->nIntervals = boost::lexical_cast<int>(valueText(nIntervalsField).toUTF8());
		_cyclePtr.modify()->amount = boost::lexical_cast<double>(valueText(amountField).toUTF8());
		_cyclePtr.modify()->firstEntryAfterCycle = boost::any_cast<int>(value(firstEntryAfterCycleField)) == 1;

		const boost::any &serviceVal = value(serviceField);
		if(serviceVal.empty())
			_cyclePtr.modify()->servicePtr = Wt::Dbo::ptr<ClientService>();
		else
			_cyclePtr.modify()->servicePtr = boost::any_cast<Wt::Dbo::ptr<ClientService>>(serviceVal);

		if(newCycle && _cyclePtr->firstEntryAfterCycle == false)
			app->accountsDatabase().createPendingCycleEntry(_cyclePtr, Wt::Dbo::ptr<AccountEntry>(), boost::posix_time::microsec_clock::local_time());

		t.commit();
	}

	//INCOME CYCLE VIEW
	IncomeCycleView::IncomeCycleView(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::WContainerWidget *parent /*= nullptr*/)
		: EntryCycleView(tr("GS.Admin.IncomeCycleView"), parent)
	{
		_model = new IncomeCycleFormModel(this, cyclePtr);
		initEntryCycleView(_model);

		WApplication *app = WApplication::instance();
		app->initServiceQueryModel();

		_purposeCombo = new Wt::WComboBox();
		_purposeCombo->insertItem(IncomeCycle::UnspecifiedPurpose, Wt::boost_any_traits<IncomeCycle::Purpose>::asString(IncomeCycle::UnspecifiedPurpose, ""));
		_purposeCombo->insertItem(IncomeCycle::Services, Wt::boost_any_traits<IncomeCycle::Purpose>::asString(IncomeCycle::Services, ""));
		_purposeCombo->changed().connect(this, &IncomeCycleView::handlePurposeChanged);
		setFormWidget(IncomeCycleFormModel::purposeField, _purposeCombo);

		model()->setVisible(IncomeCycleFormModel::serviceField, false);
		updateView(model());
	}

	Wt::WWidget *IncomeCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == IncomeCycleFormModel::serviceField)
		{
			delete _serviceCombo;
			_serviceCombo = new ProxyModelComboBox<ServiceProxyModel>(APP->serviceProxyModel());
			auto validator = new ProxyModelComboBoxValidator<ServiceProxyModel>(_serviceCombo);
			validator->setErrorString(tr("MustSelectService"));
			model()->setValidator(field, validator);
			_serviceCombo->changed().connect(this, &IncomeCycleView::handleServiceChanged);
			return _serviceCombo;
		}
		return nullptr;
	}

	void IncomeCycleView::handlePurposeChanged()
	{
		updateModel(model());
		model()->setVisible(IncomeCycleFormModel::serviceField, _purposeCombo->currentIndex() == IncomeCycle::Services);
		updateView(model());
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
		updateModel(model());
		Wt::WDialog *dialog = new Wt::WDialog(tr("AddNewService"), this);
		dialog->setTransient(true);
		dialog->rejectWhenEscapePressed(true);
		dialog->setClosable(true);
		dialog->setWidth(Wt::WLength(500));
		ServiceView *serviceView = new ServiceView(dialog->contents());

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			delete dialog;
			if(code == Wt::WDialog::Rejected)
			{
				updateModel(model());
				model()->setValue(IncomeCycleFormModel::serviceField, Wt::Dbo::ptr<ClientService>());
				updateView(model());
			}
		}, std::placeholders::_1));

		serviceView->submitted().connect(std::bind([=]() {
			updateModel(model());
			model()->setValue(IncomeCycleFormModel::serviceField, serviceView->servicePtr());
			updateView(model());
			dialog->accept();
		}));

		dialog->show();
		return dialog;
	}

	Wt::WString IncomeCycleView::viewName() const
	{
		Wt::Dbo::Transaction t(APP->session());
		if(cyclePtr())
		{
			if(cyclePtr()->entityPtr)
				return tr("IncomeCycleViewName").arg(cyclePtr().id()).arg(cyclePtr()->entityPtr->name);
			else
				return tr("IncomeCycleViewNameWithoutEntity").arg(cyclePtr().id());
		}
		return MyTemplateFormView::viewName();
	}

	void IncomeCycleView::submit()
	{
		WApplication *app = APP;
		Wt::Dbo::Transaction t(app->session());

		updateModel(model());
		if(!model()->validate())
		{
			updateView(model());
			return;
		}

		try
		{
			model()->saveChanges();
			t.commit();

			updateView(model());
			submitted().emit();
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "IncomeCycleView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

#define IdColumnWidth 80
#define TimestampColumnWidth 160
#define DateColumnWidth 130
#define EntityColumnWidth 180
#define AmountColumnWidth 200
#define ExtraColumnWidth 180

	EntryCycleList::EntryCycleList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{ }

	EntityEntryCycleList::EntityEntryCycleList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{ }

	void EntryCycleList::initEntryCycleList()
	{
		init();

		int timestampColumn = viewIndexToColumn(ViewCreatedOn);
		if(timestampColumn != -1)
			_tableView->sortByColumn(timestampColumn, Wt::DescendingOrder);
	}

	void EntityEntryCycleList::initEntityEntryCycleList()
	{
		init();

		int timestampColumn = viewIndexToColumn(EntryCycleList::ViewCreatedOn);
		if(timestampColumn != -1)
			_tableView->sortByColumn(timestampColumn, Wt::DescendingOrder);
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

	IncomeCycleList::IncomeCycleList(Wt::WContainerWidget *parent /*= nullptr*/)
		: EntryCycleList(parent)
	{
		initEntryCycleList();
	}

	EntityIncomeCycleList::EntityIncomeCycleList(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent /*= nullptr*/)
		: EntityEntryCycleList(parent), _entityPtr(entityPtr)
	{
		initEntityEntryCycleList();
	}

	void IncomeCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>(
			"SELECT c.id, c.creationDt, e.name, c.startDate, c.endDate, c.amount, s.title, c.interval, c.nIntervals, e.id e_id "
			"FROM " + std::string(IncomeCycle::tableName()) + " c "
			"INNER JOIN " + std::string(Entity::tableName()) + " e ON (e.id = c.entity_id) "
			"LEFT JOIN " + ClientService::tableName() + " s ON (s.id = c.clientservice_id)");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("c.creationDt"), tr("CreatedOn"), TimestampColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);
		addColumn(ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ViewExtra, model->addColumn("s.title"), tr("ClientService"), ExtraColumnWidth);

		_proxyModel = new EntryCycleListProxyModel<IncomeCycleList>(_model, _model);
	}

	void EntityIncomeCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>(
			"SELECT c.id, c.creationDt, c.startDate, c.endDate, c.amount, s.title, c.interval, c.nIntervals "
			"FROM " + std::string(IncomeCycle::tableName()) + " c "
			"LEFT JOIN " + ClientService::tableName() + " s ON (s.id = c.clientservice_id)")
			.where("c.entity_id = ?").bind(_entityPtr.id());

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(EntryCycleList::ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(EntryCycleList::ViewCreatedOn, model->addColumn("c.creationDt"), tr("CreatedOn"), TimestampColumnWidth);
		addColumn(EntryCycleList::ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(EntryCycleList::ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(EntryCycleList::ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(EntryCycleList::ViewExtra, model->addColumn("s.title"), tr("ClientService"), ExtraColumnWidth);

		_proxyModel = new BaseEntryCycleListProxyModel<EntityIncomeCycleList>(_model, _model);
	}

	ExpenseCycleList::ExpenseCycleList(Wt::WContainerWidget *parent /*= nullptr*/)
		: EntryCycleList(parent)
	{
		initEntryCycleList();
	}

	EntityExpenseCycleList::EntityExpenseCycleList(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent /*= nullptr*/)
		: EntityEntryCycleList(parent), _entityPtr(entityPtr)
	{
		initEntityEntryCycleList();
	}

	void ExpenseCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>(
			"SELECT c.id, c.creationDt, e.name, c.startDate, c.endDate, c.amount, p.title, c.interval, c.nIntervals, e.id e_id "
			"FROM " + std::string(ExpenseCycle::tableName()) + " c "
			"INNER JOIN " + std::string(Entity::tableName()) + " e ON (e.id = c.entity_id) "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = c.employeeposition_id)");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("c.creationDt"), tr("CreatedOn"), TimestampColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);
		addColumn(ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ViewExtra, model->addColumn("p.title"), tr("EmployeePosition"), ExtraColumnWidth);

		_proxyModel = new EntryCycleListProxyModel<ExpenseCycleList>(_model, _model);
	}

	void EntityExpenseCycleList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>(
			"SELECT c.id, c.creationDt, c.startDate, c.endDate, c.amount, p.title, c.interval, c.nIntervals "
			"FROM " + std::string(ExpenseCycle::tableName()) + " c "
			"LEFT JOIN " + EmployeePosition::tableName() + " p ON (p.id = c.employeeposition_id)")
			.where("c.entity_id = ?").bind(_entityPtr.id());

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ExpenseCycleList::ViewId, model->addColumn("c.id"), tr("ID"), IdColumnWidth);
		addColumn(ExpenseCycleList::ViewCreatedOn, model->addColumn("c.creationDt"), tr("CreatedOn"), TimestampColumnWidth);
		addColumn(ExpenseCycleList::ViewStartDate, model->addColumn("c.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ExpenseCycleList::ViewEndDate, model->addColumn("c.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ExpenseCycleList::ViewAmount, model->addColumn("c.amount"), tr("RecurringAmount"), AmountColumnWidth);
		addColumn(ExpenseCycleList::ViewExtra, model->addColumn("p.title"), tr("EmployeePosition"), ExtraColumnWidth);

		_proxyModel = new BaseEntryCycleListProxyModel<EntityExpenseCycleList>(_model, _model);
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

}
