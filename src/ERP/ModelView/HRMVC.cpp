#include "HRMVC.h"
#include "ModelView/EntryCycleMVC.h"
#include "ModelView/FindRecordEdit.h"

#include <Wt/WTextArea.h>
#include <Wt/WDateEdit.h>
#include <Wt/WTableView.h>
#include <Wt/WLengthValidator.h>
#include <Wt/WSplitButton.h>

namespace ERP
{
	//POSITION VIEW
	const Wt::WFormModel::Field PositionFormModel::titleField = "title";

	PositionFormModel::PositionFormModel(PositionView *view, Dbo::ptr<EmployeePosition> positionPtr)
		: RecordFormModel(view, move(positionPtr)), _view(view)
	{
		addField(titleField);
	}

	void PositionFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(titleField, Wt::WString::fromUTF8(_recordPtr->title));
	}

	unique_ptr<Wt::WWidget> PositionFormModel::createFormWidget(Field field)
	{
		if(field == titleField)
		{
			auto title = make_unique<Wt::WLineEdit>();
			title->setMaxLength(70);
			auto titleValidator = make_shared<Wt::WLengthValidator>(0, 70);
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
			_recordPtr = app->dboSession().addNew<EmployeePosition>();

		_recordPtr.modify()->title = valueText(titleField).toUTF8();

		if(app->positionQueryModel())
			app->positionQueryModel()->reload();

		t.commit();
		return true;
	}

	PositionView::PositionView(Dbo::ptr<EmployeePosition> positionPtr)
		: RecordFormView(tr("ERP.Admin.PositionView"))
	{
		_model = newFormModel<PositionFormModel>("position", this, move(positionPtr));
	}

	//POSITION PROXY MODEL
	PositionProxyModel::PositionProxyModel(shared_ptr<QueryModel> sourceModel)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &PositionProxyModel::addAdditionalRows);
	}

	void PositionProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), tr("SelectPosition"));
			setData(index(0, 0), false, Wt::ItemDataRole::AdditionalRow);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), tr("AddNewX").arg(tr("position")));
			setData(index(lastRow, 0), true, Wt::ItemDataRole::AdditionalRow);
		}
	}

	//SERVICE VIEW
	const Wt::WFormModel::Field ServiceFormModel::titleField = "title";

	ServiceFormModel::ServiceFormModel(ServiceView *view, Dbo::ptr<ClientService> servicePtr)
		: RecordFormModel(view, move(servicePtr)), _view(view)
	{
		addField(titleField);
	}

	void ServiceFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(titleField, Wt::WString::fromUTF8(_recordPtr->title));
	}

	unique_ptr<Wt::WWidget> ServiceFormModel::createFormWidget(Field field)
	{
		if(field == titleField)
		{
			auto title = make_unique<Wt::WLineEdit>();
			title->setMaxLength(70);
			auto titleValidator = make_shared<Wt::WLengthValidator>(0, 70);
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
			_recordPtr = app->dboSession().addNew<ClientService>();

		_recordPtr.modify()->title = valueText(titleField).toUTF8();

		if(app->serviceQueryModel())
			app->serviceQueryModel()->reload();

		t.commit();
		return true;
	}

	ServiceView::ServiceView(Dbo::ptr<ClientService> servicePtr)
		: RecordFormView(tr("ERP.Admin.ServiceView"))
	{
		_model = newFormModel<ServiceFormModel>("service", this, move(servicePtr));
	}

	//SERVICE PROXY MODEL
	ServiceProxyModel::ServiceProxyModel(shared_ptr<QueryModel> sourceModel)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &ServiceProxyModel::addAdditionalRows);
	}

	void ServiceProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), tr("SelectService"));
			setData(index(0, 0), false, Wt::ItemDataRole::AdditionalRow);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), tr("AddNewX").arg(tr("service")));
			setData(index(lastRow, 0), true, Wt::ItemDataRole::AdditionalRow);
		}
	}

	const Wt::WFormModel::Field EmployeeAssignmentFormModel::descriptionField = "description";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::startDateField = "startDate";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::endDateField = "endDate";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::entityField = "entity";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::locationField = "location";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::cycleField = "cycle";
	const Wt::WFormModel::Field EmployeeAssignmentFormModel::positionField = "position";

	EmployeeAssignmentFormModel::EmployeeAssignmentFormModel(EmployeeAssignmentView *view, Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr)
		: RecordFormModel(view, move(employeeAssignmentPtr)), _view(view)
	{
		addField(descriptionField);
		addField(startDateField);
		addField(endDateField);
		addField(entityField);
		addField(locationField);
		addField(cycleField);
		addField(positionField);

		setVisible(cycleField, false);
	}

	void EmployeeAssignmentFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(descriptionField, Wt::WString::fromUTF8(_recordPtr->description));
		setValue(startDateField, _recordPtr->startDate);
		setValue(endDateField, _recordPtr->endDate);
		setValue(entityField, _recordPtr->entityPtr);
		setValue(locationField, _recordPtr->locationPtr);
		setValue(cycleField, _recordPtr->expenseCyclePtr);
		setValue(positionField, _recordPtr->positionPtr);

		if(_recordPtr->expenseCyclePtr)
		{
			Wt::WString text = tr("AssociatedEntryCycleSummary").arg(_recordPtr->expenseCyclePtr->entityPtr->name)
				.arg(rsEveryNIntervals(_recordPtr->expenseCyclePtr->amount(), _recordPtr->expenseCyclePtr->interval, _recordPtr->expenseCyclePtr->nIntervals));
			auto *a = _view->resolve<Wt::WAnchor*>("expenseCycle");
			a->setText(text);
			a->setLink(Wt::WLink(Wt::LinkType::InternalPath, ExpenseCycle::viewInternalPath(_recordPtr->expenseCyclePtr.id())));
		}
		_view->setCondition("show-expenseCycle", (bool)_recordPtr->expenseCyclePtr);
		_view->resolveWidget("setExpenseCycle")->setHidden((bool)_recordPtr->expenseCyclePtr);

		if(_recordPtr->clientAssignmentPtr)
		{
			Wt::WString text = tr("AssociatedClientAssignmentSummary").arg(_recordPtr->clientAssignmentPtr->entityPtr->name)
					.arg(_recordPtr->clientAssignmentPtr->servicePtr->title);
			auto *a = _view->resolve<Wt::WAnchor*>("clientAssignment");
			a->setText(text);
			a->setLink(Wt::WLink(Wt::LinkType::InternalPath, ClientAssignment::viewInternalPath(_recordPtr->clientAssignmentPtr.id())));
		}
		_view->setCondition("show-clientAssignment", (bool)_recordPtr->clientAssignmentPtr);
		_view->resolveWidget("setClientAssignment")->setHidden((bool)_recordPtr->clientAssignmentPtr);
	}

	unique_ptr<Wt::WWidget> EmployeeAssignmentFormModel::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == descriptionField)
		{
			auto description = make_unique<Wt::WTextArea>();
			description->setRows(3);
			return description;
		}
		if(field == startDateField)
		{
			auto startDateEdit = make_unique<Wt::WDateEdit>();
			startDateEdit->setPlaceholderText(Wt::WLocale::currentLocale().dateFormat());
			auto startDateValidator = make_shared<Wt::WDateValidator>();
			startDateValidator->setMandatory(true);
			setValidator(startDateField, startDateValidator);
			startDateEdit->changed().connect(this, std::bind(&EmployeeAssignmentFormModel::updateEndDateValidator, this, true));
			return startDateEdit;
		}
		if(field == endDateField)
		{
			auto endDateEdit = make_unique<Wt::WDateEdit>();
			endDateEdit->setPlaceholderText(tr("EmptyEndDate"));
			auto endDateValidator = make_shared<Wt::WDateValidator>();
			setValidator(endDateField, endDateValidator);
			return endDateEdit;
		}
		if(field == entityField)
		{
			auto findEntityEdit = make_unique<FindEntityEdit>(Entity::PersonType);
			auto findEntityValidator = make_shared<FindEntityValidator>(findEntityEdit.get(), true);
			findEntityValidator->setModifyPermissionRequired(true);
			setValidator(entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == locationField)
		{
			auto findLocationEdit = make_unique<FindLocationEdit>();
			auto findLocationValidator = make_shared<FindLocationValidator>(findLocationEdit.get(), true);
			setValidator(locationField, findLocationValidator);
			return findLocationEdit;
		}
		if(field == positionField)
		{
			WApplication *app = APP;
			app->initPositionQueryModel();

			auto posCombo = make_unique<QueryProxyModelCB<PositionProxyModel>>(app->positionProxyModel());
			auto validator = make_shared<QueryProxyModelCBValidator<PositionProxyModel>>(posCombo.get());
			validator->setErrorString(tr("MustSelectPosition"));
			setValidator(field, validator);
			posCombo->changed().connect(_view, &EmployeeAssignmentView::handlePositionChanged);
			_view->_positionCombo = posCombo.get();
			return posCombo;
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
			_recordPtr = app->dboSession().addNew<EmployeeAssignment>();
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->description = valueText(descriptionField).toUTF8();
		_recordPtr.modify()->startDate = Wt::any_cast<Wt::WDate>(value(startDateField));
		_recordPtr.modify()->endDate = Wt::any_cast<Wt::WDate>(value(endDateField));
		_recordPtr.modify()->entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->locationPtr = Wt::any_cast<Dbo::ptr<Location>>(value(locationField));
		_recordPtr.modify()->positionPtr = Wt::any_cast<Dbo::ptr<EmployeePosition>>(value(positionField));

		const Wt::any &expenseCycleVal = value(cycleField);
		_recordPtr.modify()->expenseCyclePtr = expenseCycleVal.empty() ? nullptr : Wt::any_cast<Dbo::ptr<ExpenseCycle>>(expenseCycleVal);

		t.commit();
		return true;
	}

	void EmployeeAssignmentFormModel::showExpenseCycleDialog()
	{
		if(_dialog)
			return;

		TRANSACTION(APP);
		if(_recordPtr->expenseCyclePtr)
			return;

		auto selectionDialog = addChild(make_unique<ListSelectionDialog<ExpenseCycleList>>(tr("SelectXFromList").arg(tr("recurringExpense"))));
		selectionDialog->selected().connect(this, &EmployeeAssignmentFormModel::handleExpenseCycleSelected);
		selectionDialog->finished().connect(this, &EmployeeAssignmentFormModel::handleDialogFinished);
		_dialog = selectionDialog;
		selectionDialog->show();
	}

	void EmployeeAssignmentFormModel::showClientAssignmentDialog()
	{
		if(_dialog)
			return;

		TRANSACTION(APP);
		if(_recordPtr->clientAssignmentPtr)
			return;

		auto selectionDialog = addChild(make_unique<ListSelectionDialog<ClientAssignmentList>>(tr("SelectXFromList").arg(tr("recurringExpense"))));
		selectionDialog->selected().connect(this, &EmployeeAssignmentFormModel::handleClientAssignmentSelected);
		selectionDialog->finished().connect(this, &EmployeeAssignmentFormModel::handleDialogFinished);
		_dialog = selectionDialog;
		selectionDialog->show();
	}

	void EmployeeAssignmentFormModel::handleExpenseCycleSelected(long long id)
	{
		WApplication *app = APP;
		TRANSACTION(app);

		if(_recordPtr->expenseCyclePtr)
			return;

		try
		{
			Dbo::ptr<ExpenseCycle> ptr = app->dboSession().loadLazy<ExpenseCycle>(id);
			_recordPtr.modify()->expenseCyclePtr = ptr;
			t.commit();
		}
		catch(const Dbo::StaleObjectException &)
		{
			app->dboSession().rereadAll();
			app->showStaleObjectError();
		}
		catch(const Dbo::Exception &e)
		{
			Wt::log("error") << "EmployeeAssignmentFormModel::handleExpenseCycleSelected(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
		_view->reload();
	}

	void EmployeeAssignmentFormModel::handleClientAssignmentSelected(long long id)
	{
		WApplication *app = APP;
		TRANSACTION(app);

		if(_recordPtr->clientAssignmentPtr)
			return;

		try
		{
			Dbo::ptr<ClientAssignment> ptr = app->dboSession().loadLazy<ClientAssignment>(id);
			_recordPtr.modify()->clientAssignmentPtr = ptr;
			t.commit();
		}
		catch(const Dbo::StaleObjectException &)
		{
			app->dboSession().rereadAll();
			app->showStaleObjectError();
		}
		catch(const Dbo::Exception &e)
		{
			Wt::log("error") << "EmployeeAssignmentFormModel::handleClientAssignmentSelected(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
		_view->reload();
	}

	void EmployeeAssignmentFormModel::handleDialogFinished()
	{
		if(!_dialog)
			return;

		_dialog->removeFromParent();
		_dialog = nullptr;
	}

	EmployeeAssignmentView::EmployeeAssignmentView(Dbo::ptr<EmployeeAssignment> assignmentPtr)
		: RecordFormView(tr("ERP.Admin.EmployeeAssignmentView"))
	{
		_model = newFormModel<EmployeeAssignmentFormModel>("assignment", this, move(assignmentPtr));
	}

	void EmployeeAssignmentFormModel::persistedHandler()
	{
		setReadOnly(startDateField, true);

		_view->bindNew<Wt::WAnchor>("expenseCycle");
		_view->bindNew<Wt::WAnchor>("clientAssignment");

		auto *setExpenseCycle = _view->bindNew<Wt::WPushButton>("setExpenseCycle", tr("AssociateWithRecurringExpense"));
		setExpenseCycle->clicked().connect(this, &EmployeeAssignmentFormModel::showExpenseCycleDialog);

		auto *setClientAssignment = _view->bindNew<Wt::WPushButton>("setClientAssignment", tr("AssociateWithClientAssignment"));
		setClientAssignment->clicked().connect(this, &EmployeeAssignmentFormModel::showClientAssignmentDialog);

		_view->reload();
	}

	void EmployeeAssignmentView::handlePositionChanged()
	{
		Wt::any v = _positionCombo->model()->index(_positionCombo->currentIndex(), 0).data(Wt::ItemDataRole::AdditionalRow);
		if(v.empty())
			return;

		if(Wt::any_cast<bool>(v) == true)
			showAddPositionDialog();
	}

	void EmployeeAssignmentView::showAddPositionDialog()
	{
		if(_dialog)
			return;

		_dialog = addChild(make_unique<Wt::WDialog>(tr("AddNewX").arg(tr("position"))));
		_dialog->setClosable(true);
		_dialog->setTransient(true);
		_dialog->rejectWhenEscapePressed(true);
		_dialog->setWidth(Wt::WLength(500));
		auto *positionView = _dialog->contents()->addNew<PositionView>();

		_dialog->finished().connect(this, std::bind([this](Wt::DialogCode code) {
			if(code == Wt::DialogCode::Rejected)
			{
				_model->setValue(EmployeeAssignmentFormModel::positionField, Dbo::ptr<EmployeePosition>());
				updateViewField(_model, EmployeeAssignmentFormModel::positionField);
			}
			_dialog->removeFromParent();
			_dialog = nullptr;
		}, _1));

		positionView->submitted().connect(this, std::bind([this, positionView]() {
			_model->setValue(EmployeeAssignmentFormModel::positionField, positionView->positionPtr());
			updateViewField(_model, EmployeeAssignmentFormModel::positionField);
			_dialog->accept();
		}));

		_dialog->show();
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

		return "EmployeeAssignmentView";
	}

	void EmployeeAssignmentFormModel::updateEndDateValidator(bool update)
	{
		if(update)
			_view->updateModelField(this, EmployeeAssignmentFormModel::startDateField);

		const Wt::any &startDateVal = value(EmployeeAssignmentFormModel::startDateField);
		if(startDateVal.empty())
			return;

		Wt::WDate startDate = Wt::any_cast<Wt::WDate>(startDateVal);
		auto endDateValidator = static_pointer_cast<Wt::WDateValidator>(validator(EmployeeAssignmentFormModel::endDateField));
		endDateValidator->setBottom(startDate.isValid() ? startDate.addDays(1) : Wt::WDate());
	}

	EmployeeAssignmentListProxyModel::EmployeeAssignmentListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
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
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::any EmployeeAssignmentListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any EmployeeAssignmentListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const EmployeeAssignmentList::ResultType &res = static_pointer_cast<Dbo::QueryModel<EmployeeAssignmentList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<EmployeeAssignmentList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, EmployeeAssignment::viewInternalPath(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		auto viewIndex = Wt::any_cast<int>(viewIndexData);

		const EmployeeAssignmentList::ResultType &res = static_pointer_cast<Dbo::QueryModel<EmployeeAssignmentList::ResultType>>(sourceModel())->resultRow(idx.row());

		if(viewIndex == EmployeeAssignmentList::ViewStartDate && role == Wt::ItemDataRole::Display)
		{
			const Wt::WDate &date = std::get<EmployeeAssignmentList::ResStartDate>(res);
			if(date.isValid() && date > Wt::WDate::currentServerDate())
				return tr("XNotStarted").arg(std::get<EmployeeAssignmentList::ResStartDate>(res).toString(Wt::WLocale::currentLocale().dateFormat()));
		}

		if(viewIndex == EmployeeAssignmentList::ViewEndDate && role == Wt::ItemDataRole::Display)
		{
			const Wt::WDate &date = std::get<EmployeeAssignmentList::ResEndDate>(res);
			if(date.isValid() && Wt::WDate::currentServerDate() >= date)
				return tr("XEnded").arg(date.toString(Wt::WLocale::currentLocale().dateFormat()));
		}

		if(role == Wt::ItemDataRole::StyleClass)
		{
			const Wt::WDate &startDate = std::get<EmployeeAssignmentList::ResStartDate>(res);
			if(startDate.isValid() && startDate > Wt::WDate::currentServerDate())
				return "text-info";

			const Wt::WDate &endDate = std::get<EmployeeAssignmentList::ResEndDate>(res);
			if(endDate.isValid() && Wt::WDate::currentServerDate() >= endDate)
				return "text-muted";
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	void EmployeeAssignmentList::load()
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

	void EmployeeAssignmentList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "a.id", std::bind(&FiltersTemplate::initIdEdit, _1))); filtersTemplate()->addFilter(1);
	}

	void EmployeeAssignmentList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT a.id, a.timestamp, e.name, a.startDate, a.endDate, cnt.name, city.name, l.address FROM "
			+ EmployeeAssignment::tStr() + " a "
			"INNER JOIN " + Entity::tStr() + " e ON (e.id = a.entity_id) "
			"LEFT JOIN " + Location::tStr() + " l ON (l.id = a.location_id) "
			"LEFT JOIN " + Country::tStr() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tStr() + " city ON (city.id = l.city_id)");

		if(_entityPtr.id() != -1)
			_baseQuery.where("a.entity_id = ?").bind(_entityPtr.id());

		if(_clientAssignmentPtr.id() != -1)
			_baseQuery.where("a.clientassignment_id = ?").bind(_clientAssignmentPtr.id());

		if(_cyclePtr.id() != -1)
			_baseQuery.where("a.expensecycle_id = ?").bind(_cyclePtr.id());

		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "a.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("a.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("a.timestamp"), tr("CreatedOn"), DateTimeColumnWidth);

		if(_entityPtr.id() == -1)
			addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), 200);

		addColumn(ViewStartDate, model->addColumn("a.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("a.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewCountry, model->addColumn("cnt.name"), tr("Country"), 150);
		addColumn(ViewCity, model->addColumn("city.name"), tr("City"), 150);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), 300);

		_proxyModel = make_shared<EmployeeAssignmentListProxyModel>(_model);
	}

	const Wt::WFormModel::Field ClientAssignmentFormModel::descriptionField = "description";
	const Wt::WFormModel::Field ClientAssignmentFormModel::startDateField = "startDate";
	const Wt::WFormModel::Field ClientAssignmentFormModel::endDateField = "endDate";
	const Wt::WFormModel::Field ClientAssignmentFormModel::entityField = "entity";
	const Wt::WFormModel::Field ClientAssignmentFormModel::cycleField = "cycle";
	const Wt::WFormModel::Field ClientAssignmentFormModel::serviceField = "service";

	ClientAssignmentFormModel::ClientAssignmentFormModel(ClientAssignmentView *view, Dbo::ptr<ClientAssignment> clientAssignmentPtr)
		: RecordFormModel(view, move(clientAssignmentPtr)), _view(view)
	{
		addField(descriptionField);
		addField(startDateField);
		addField(endDateField);
		addField(entityField);
		addField(cycleField);
		addField(serviceField);

		setVisible(cycleField, false);
	}

	void ClientAssignmentFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(descriptionField, Wt::WString::fromUTF8(_recordPtr->description));
		setValue(startDateField, _recordPtr->startDate);
		setValue(endDateField, _recordPtr->endDate);
		setValue(entityField, _recordPtr->entityPtr);
		setValue(cycleField, _recordPtr->incomeCyclePtr);
		setValue(serviceField, _recordPtr->servicePtr);

		if(_recordPtr->incomeCyclePtr)
		{
			Wt::WString text = tr("AssociatedEntryCycleSummary").arg(_recordPtr->incomeCyclePtr->entityPtr->name)
				.arg(rsEveryNIntervals(_recordPtr->incomeCyclePtr->amount(), _recordPtr->incomeCyclePtr->interval, _recordPtr->incomeCyclePtr->nIntervals));
			auto *a = _view->resolve<Wt::WAnchor*>("incomeCycle");
			a->setText(text);
			a->setLink(Wt::WLink(Wt::LinkType::InternalPath, IncomeCycle::viewInternalPath(_recordPtr->incomeCyclePtr.id())));
		}
		_view->setCondition("show-incomeCycle", (bool)_recordPtr->incomeCyclePtr);
		_view->setCondition("set-incomeCycle", !_recordPtr->incomeCyclePtr);
	}

	unique_ptr<Wt::WWidget> ClientAssignmentFormModel::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == descriptionField)
		{
			auto description = make_unique<Wt::WTextArea>();
			description->setRows(3);
			return description;
		}
		if(field == startDateField)
		{
			auto startDateEdit = make_unique<Wt::WDateEdit>();
			startDateEdit->setPlaceholderText(Wt::WLocale::currentLocale().dateFormat());
			auto startDateValidator = make_shared<Wt::WDateValidator>();
			startDateValidator->setMandatory(true);
			setValidator(startDateField, startDateValidator);
			startDateEdit->changed().connect(this, std::bind(&ClientAssignmentFormModel::updateEndDateValidator, this, true));
			return startDateEdit;
		}
		if(field == endDateField)
		{
			auto endDateEdit = make_unique<Wt::WDateEdit>();
			endDateEdit->setPlaceholderText(tr("EmptyEndDate"));
			auto endDateValidator = make_shared<Wt::WDateValidator>();
			setValidator(endDateField, endDateValidator);
			return endDateEdit;
		}
		if(field == entityField)
		{
			auto findEntityEdit = make_unique<FindEntityEdit>();
			auto findEntityValidator = make_shared<FindEntityValidator>(findEntityEdit.get(), true);
			findEntityValidator->setModifyPermissionRequired(true);
			setValidator(entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == serviceField)
		{
			WApplication *app = APP;
			app->initServiceQueryModel();

			auto serviceCombo = make_unique<QueryProxyModelCB<ServiceProxyModel>>(app->serviceProxyModel());
			auto validator = make_shared<QueryProxyModelCBValidator<ServiceProxyModel>>(serviceCombo.get());
			validator->setErrorString(tr("MustSelectService"));
			setValidator(field, validator);
			serviceCombo->changed().connect(_view, &ClientAssignmentView::handleServiceChanged);
			_view->_serviceCombo = serviceCombo.get();
			return serviceCombo;
		}
		return nullptr;
	}

	bool ClientAssignmentFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);

		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().addNew<ClientAssignment>();
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->description = valueText(descriptionField).toUTF8();
		_recordPtr.modify()->startDate = Wt::any_cast<Wt::WDate>(value(startDateField));
		_recordPtr.modify()->endDate = Wt::any_cast<Wt::WDate>(value(endDateField));
		_recordPtr.modify()->entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->servicePtr = Wt::any_cast<Dbo::ptr<ClientService>>(value(serviceField));

		const Wt::any &cycleVal = value(cycleField);
		_recordPtr.modify()->incomeCyclePtr = cycleVal.empty() ? nullptr : Wt::any_cast<Dbo::ptr<IncomeCycle>>(cycleVal);

		t.commit();
		return true;
	}

	void ClientAssignmentFormModel::showIncomeCycleDialog()
	{
		if(_dialog)
			return;

		TRANSACTION(APP);
		if(_recordPtr->incomeCyclePtr)
			return;

		auto selectionDialog = addChild(make_unique<ListSelectionDialog<IncomeCycleList>>(tr("SelectXFromList").arg(tr("recurringIncome"))));
		selectionDialog->selected().connect(this, &ClientAssignmentFormModel::handleIncomeCycleSelected);
		selectionDialog->finished().connect(this, &ClientAssignmentFormModel::handleDialogFinished);
		_dialog = selectionDialog;
		selectionDialog->show();
	}

	void ClientAssignmentFormModel::handleIncomeCycleSelected(long long id)
	{
		WApplication *app = APP;
		TRANSACTION(app);

		if(_recordPtr->incomeCyclePtr)
			return;

		try
		{
			Dbo::ptr<IncomeCycle> ptr = app->dboSession().loadLazy<IncomeCycle>(id);
			_recordPtr.modify()->incomeCyclePtr = ptr;
			t.commit();
		}
		catch(const Dbo::StaleObjectException &)
		{
			app->dboSession().rereadAll();
			app->showStaleObjectError();
		}
		catch(const Dbo::Exception &e)
		{
			Wt::log("error") << "EmployeeAssignmentFormModel::handleClientAssignmentSelected(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
		_view->reload();
	}

	void ClientAssignmentFormModel::handleDialogFinished()
	{
		if(!_dialog)
			return;

		_dialog->removeFromParent();
		_dialog = nullptr;
	}

	ClientAssignmentView::ClientAssignmentView(Dbo::ptr<ClientAssignment> assignmentPtr)
		: RecordFormView(tr("ERP.Admin.ClientAssignmentView"))
	{
		_model = newFormModel<ClientAssignmentFormModel>("assignment", this, move(assignmentPtr));
	}

	void ClientAssignmentView::initView()
	{
		bindNew<Wt::WAnchor>("incomeCycle");
	}

	void ClientAssignmentView::handleServiceChanged()
	{
		Wt::any v = _serviceCombo->model()->index(_serviceCombo->currentIndex(), 0).data(Wt::ItemDataRole::AdditionalRow);
		if(v.empty())
			return;

		if(Wt::any_cast<bool>(v) == true)
			showAddServiceDialog();
	}

	void ClientAssignmentView::showAddServiceDialog()
	{
		if(_dialog)
			return;

		_dialog = addChild(make_unique<Wt::WDialog>(tr("AddNewX").arg(tr("service"))));
		_dialog->setTransient(true);
		_dialog->rejectWhenEscapePressed(true);
		_dialog->setClosable(true);
		_dialog->setWidth(Wt::WLength(500));
		auto *serviceView  = _dialog->contents()->addNew<ServiceView>();

		_dialog->finished().connect(this, std::bind([this](Wt::DialogCode code) {
			if(code == Wt::DialogCode::Rejected)
			{
				_model->setValue(ClientAssignmentFormModel::serviceField, Dbo::ptr<ClientService>());
				updateViewField(_model, ClientAssignmentFormModel::serviceField);
			}
			_dialog->removeFromParent();
			_dialog = nullptr;
		}, _1));

		serviceView->submitted().connect(this, std::bind([this, serviceView]() {
			_model->setValue(ClientAssignmentFormModel::serviceField, serviceView->servicePtr());
			updateViewField(_model, ClientAssignmentFormModel::serviceField);
			_dialog->accept();
		}));

		_dialog->show();
	}

	void ClientAssignmentView::updateView(Wt::WFormModel *model)
	{
		RecordFormView::updateView(model);
		_model->updateEndDateValidator(false);
	}

	void ClientAssignmentView::updateModel(Wt::WFormModel *model)
	{
		RecordFormView::updateModel(model);
		_model->updateEndDateValidator(false);
	}

	Wt::WString ClientAssignmentView::viewName() const
	{
		if(clientAssignmentPtr())
			return tr("ClientAssignmentViewName").arg(clientAssignmentPtr().id());

		return "ClientAssingmentView";
	}

	void ClientAssignmentFormModel::persistedHandler()
	{
		setReadOnly(startDateField, true);

		_view->bindNew<EmployeeAssignmentList>("employeeAssignments", recordPtr());

		auto *setIncomeCycle = _view->bindNew<Wt::WPushButton>("setIncomeCycle", tr("AssociateWithRecurringIncome"));
		setIncomeCycle->clicked().connect(this, &ClientAssignmentFormModel::showIncomeCycleDialog);

		_view->reload();
	}

	void ClientAssignmentFormModel::updateEndDateValidator(bool update)
	{
		if(update)
			_view->updateModelField(this, ClientAssignmentFormModel::startDateField);

		const Wt::any &startDateVal = value(ClientAssignmentFormModel::startDateField);
		if(startDateVal.empty())
			return;

		Wt::WDate startDate = Wt::any_cast<Wt::WDate>(startDateVal);
		auto endDateValidator = static_pointer_cast<Wt::WDateValidator>(validator(ClientAssignmentFormModel::endDateField));
		endDateValidator->setBottom(startDate.isValid() ? startDate.addDays(1) : Wt::WDate());
	}

	void ClientAssignmentList::load()
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

	void ClientAssignmentList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "ca.id", std::bind(&FiltersTemplate::initIdEdit, _1))); filtersTemplate()->addFilter(1);
	}

	void ClientAssignmentList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT ca.id, ca.timestamp, e.name, ca.startDate, ca.endDate, COUNT(ea.id) FROM "
			+ ClientAssignment::tStr() + " ca "
			"INNER JOIN " + Entity::tStr() + " e ON (e.id = ca.entity_id) "
			"LEFT JOIN " + EmployeeAssignment::tStr() + " ea ON ea.clientassignment_id = ca.id"
			).groupBy("ca.id");

		if(_entityPtr.id() != -1)
			_baseQuery.where("ca.entity_id = ?").bind(_entityPtr.id());

		if(_cyclePtr.id() != -1)
			_baseQuery.where("ca.incomecycle_id = ?").bind(_cyclePtr.id());

		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "ca.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("ca.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewCreatedOn, model->addColumn("ca.timestamp"), tr("CreatedOn"), DateTimeColumnWidth);

		if(_entityPtr.id() == -1)
			addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), 200);

		addColumn(ViewStartDate, model->addColumn("ca.startDate"), tr("StartDate"), DateColumnWidth);
		addColumn(ViewEndDate, model->addColumn("ca.endDate"), tr("EndDate"), DateColumnWidth);
		addColumn(ViewEmployeesAssigned, model->addColumn("COUNT(ea.id)"), tr("EmployeesAssigned"), 80);

		_proxyModel = make_shared<ClientAssignmentListProxyModel>(_model);
	}

	ClientAssignmentListProxyModel::ClientAssignmentListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void ClientAssignmentListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::WFlags<Wt::ItemFlag> ClientAssignmentListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::any ClientAssignmentListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any ClientAssignmentListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const ClientAssignmentList::ResultType &res = static_pointer_cast<Dbo::QueryModel<ClientAssignmentList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<ClientAssignmentList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, ClientAssignment::viewInternalPath(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		auto viewIndex = Wt::any_cast<int>(viewIndexData);

		const ClientAssignmentList::ResultType &res = static_pointer_cast<Dbo::QueryModel<ClientAssignmentList::ResultType>>(sourceModel())->resultRow(idx.row());

		if(viewIndex == ClientAssignmentList::ViewStartDate && role == Wt::ItemDataRole::Display)
		{
			const Wt::WDate &date = std::get<ClientAssignmentList::ResStartDate>(res);
			if(date.isValid() && date > Wt::WDate::currentServerDate())
				return tr("XNotStarted").arg(std::get<ClientAssignmentList::ResStartDate>(res).toString(Wt::WLocale::currentLocale().dateFormat()));
		}

		if(viewIndex == ClientAssignmentList::ViewEndDate && role == Wt::ItemDataRole::Display)
		{
			const Wt::WDate &date = std::get<ClientAssignmentList::ResEndDate>(res);
			if(date.isValid() && Wt::WDate::currentServerDate() >= date)
				return tr("XEnded").arg(date.toString(Wt::WLocale::currentLocale().dateFormat()));
		}

		if(role == Wt::ItemDataRole::StyleClass)
		{
			const Wt::WDate &startDate = std::get<ClientAssignmentList::ResStartDate>(res);
			if(startDate.isValid() && startDate > Wt::WDate::currentServerDate())
				return "text-info";

			const Wt::WDate &endDate = std::get<ClientAssignmentList::ResEndDate>(res);
			if(endDate.isValid() && Wt::WDate::currentServerDate() >= endDate)
				return "text-muted";
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}
}
