#include "HRMVC.h"
#include "Widgets/EntryCycleMVC.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/WTextArea.h>
#include <Wt/WPushButton.h>
#include <Wt/WDateEdit.h>
#include <Wt/WTableView.h>
#include <Wt/WLengthValidator.h>
#include <Wt/WAnchor.h>
#include <Wt/WSplitButton.h>

namespace GS
{
	using namespace std::placeholders;
	//POSITION VIEW
	const Wt::WFormModel::Field PositionFormModel::titleField = "title";
	const Wt::WFormModel::Field PositionFormModel::typeField = "type";

	PositionFormModel::PositionFormModel(PositionView *view, Wt::Dbo::ptr<EmployeePosition> positionPtr /*= Wt::Dbo::ptr<EmployeePosition>()*/)
		: RecordFormModel(view, positionPtr), _view(view)
	{
		addField(titleField);
		addField(typeField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(titleField, Wt::WString::fromUTF8(_recordPtr->title));
			setValue(typeField, (int)_recordPtr->type);
		}
	}

	std::unique_ptr<Wt::WWidget> PositionFormModel::createFormWidget(Field field)
	{
		if(field == titleField)
		{
			auto title = std::make_unique<Wt::WLineEdit>();
			title->setMaxLength(70);
			auto titleValidator = std::make_shared<Wt::WLengthValidator>(0, 70);
			titleValidator->setMandatory(true);
			setValidator(titleField, titleValidator);
			return title;
		}
		if(field == typeField)
		{
			//WApplication *app = APP;
			auto cb = std::make_unique<Wt::WComboBox>();
			cb->insertItem(EmployeePosition::OtherType, Wt::any_traits<EmployeePosition::Type>::asString(EmployeePosition::OtherType, ""));
			cb->insertItem(EmployeePosition::PersonnelType, Wt::any_traits<EmployeePosition::Type>::asString(EmployeePosition::PersonnelType, ""));

			auto proxyModel = std::make_shared<Wt::WBatchEditProxyModel>();
			proxyModel->setSourceModel(cb->model());
			proxyModel->insertRow(0);
			proxyModel->setData(proxyModel->index(0, 0), tr("SelectType"));
			cb->setModel(proxyModel);

			auto validator = std::make_shared<ProxyModelCBValidator>(cb.get());
			validator->setErrorString(tr("MustSelectType"));
			setValidator(field, validator);
			return cb;
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
			_recordPtr = app->dboSession().add(std::make_unique<EmployeePosition>());

		_recordPtr.modify()->title = valueText(titleField).toUTF8();
		_recordPtr.modify()->type = EmployeePosition::Type(Wt::any_cast<int>(value(typeField)) - 1);

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

	void PositionView::initView()
	{
		_model = std::make_shared<PositionFormModel>(this, _tempPtr);
		addFormModel("position", _model);
	}

	//POSITION PROXY MODEL
	PositionProxyModel::PositionProxyModel(std::shared_ptr<QueryModel> sourceModel)
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

	std::unique_ptr<Wt::WWidget> ServiceFormModel::createFormWidget(Field field)
	{
		if(field == titleField)
		{
			auto title = std::make_unique<Wt::WLineEdit>();
			title->setMaxLength(70);
			auto titleValidator = std::make_shared<Wt::WLengthValidator>(0, 70);
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
			_recordPtr = app->dboSession().add(std::make_unique<ClientService>());

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

	void ServiceView::initView()
	{
		_model = std::make_shared<ServiceFormModel>(this, _tempPtr);
		addFormModel("service", _model);
	}

	//SERVICE PROXY MODEL
	ServiceProxyModel::ServiceProxyModel(std::shared_ptr<QueryModel> sourceModel)
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

	EmployeeAssignmentFormModel::EmployeeAssignmentFormModel(EmployeeAssignmentView *view, Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr /*= Wt::Dbo::ptr<EmployeeAssignment>()*/)
		: RecordFormModel(view, employeeAssignmentPtr), _view(view)
	{
		addField(descriptionField);
		addField(startDateField);
		addField(endDateField);
		addField(entityField);
		addField(locationField);
		addField(cycleField);
		addField(positionField);

		setVisible(cycleField, false);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(descriptionField, Wt::WString::fromUTF8(_recordPtr->description));
			setValue(startDateField, _recordPtr->startDate);
			setValue(endDateField, _recordPtr->endDate);
			setValue(entityField, _recordPtr->entityPtr);
			setValue(locationField, _recordPtr->locationPtr);
			setValue(cycleField, _recordPtr->expenseCyclePtr);
			setValue(positionField, _recordPtr->positionPtr);
		}
	}

	std::unique_ptr<Wt::WWidget> EmployeeAssignmentFormModel::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == descriptionField)
		{
			auto description = std::make_unique<Wt::WTextArea>();
			description->setRows(3);
			return description;
		}
		if(field == startDateField)
		{
			auto startDateEdit = std::make_unique<Wt::WDateEdit>();
			startDateEdit->setPlaceholderText(Wt::WLocale::currentLocale().dateFormat());
			auto startDateValidator = std::make_shared<Wt::WDateValidator>();
			startDateValidator->setMandatory(true);
			setValidator(startDateField, startDateValidator);
			startDateEdit->changed().connect(this, std::bind(&EmployeeAssignmentFormModel::updateEndDateValidator, this, true));
			return startDateEdit;
		}
		if(field == endDateField)
		{
			auto endDateEdit = std::make_unique<Wt::WDateEdit>();
			endDateEdit->setPlaceholderText(tr("EmptyEndDate"));
			auto endDateValidator = std::make_shared<Wt::WDateValidator>();
			setValidator(endDateField, endDateValidator);
			return endDateEdit;
		}
		if(field == entityField)
		{
			auto findEntityEdit = std::make_unique<FindEntityEdit>(Entity::PersonType);
			auto findEntityValidator = std::make_shared<FindEntityValidator>(findEntityEdit.get(), true);
			findEntityValidator->setModifyPermissionRequired(true);
			setValidator(entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == locationField)
		{
			auto findLocationEdit = std::make_unique<FindLocationEdit>();
			auto findLocationValidator = std::make_shared<FindLocationValidator>(findLocationEdit.get(), true);
			setValidator(locationField, findLocationValidator);
			return findLocationEdit;
		}
		if(field == positionField)
		{
			WApplication *app = APP;
			app->initPositionQueryModel();

			auto posCombo = std::make_unique<QueryProxyModelCB<PositionProxyModel>>(app->positionProxyModel());
			auto validator = std::make_shared<QueryProxyModelCBValidator<PositionProxyModel>>(posCombo.get());
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
			_recordPtr = app->dboSession().add(std::make_unique<EmployeeAssignment>());
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->description = valueText(descriptionField).toUTF8();
		_recordPtr.modify()->startDate = Wt::any_cast<Wt::WDate>(value(startDateField));
		_recordPtr.modify()->endDate = Wt::any_cast<Wt::WDate>(value(endDateField));
		_recordPtr.modify()->entityPtr = Wt::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->locationPtr = Wt::any_cast<Wt::Dbo::ptr<Location>>(value(locationField));
		_recordPtr.modify()->positionPtr = Wt::any_cast<Wt::Dbo::ptr<EmployeePosition>>(value(positionField));

		const Wt::any &expenseCycleVal = value(cycleField);
		_recordPtr.modify()->expenseCyclePtr = expenseCycleVal.empty() ? Wt::Dbo::ptr<ExpenseCycle>() : Wt::any_cast<Wt::Dbo::ptr<ExpenseCycle>>(expenseCycleVal);

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

		auto selectionDialog = addChild(std::make_unique<ListSelectionDialog<ExpenseCycleList>>(tr("SelectXFromList").arg(tr("recurringExpense"))));
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

		auto selectionDialog = addChild(std::make_unique<ListSelectionDialog<ClientAssignmentList>>(tr("SelectXFromList").arg(tr("recurringExpense"))));
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
			Wt::Dbo::ptr<ExpenseCycle> ptr = app->dboSession().loadLazy<ExpenseCycle>(id);
			_recordPtr.modify()->expenseCyclePtr = ptr;
			t.commit();
		}
		catch(const Wt::Dbo::StaleObjectException &)
		{
			app->dboSession().rereadAll();
			app->showStaleObjectError();
		}
		catch(const Wt::Dbo::Exception &e)
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
			Wt::Dbo::ptr<ClientAssignment> ptr = app->dboSession().loadLazy<ClientAssignment>(id);
			_recordPtr.modify()->clientAssignmentPtr = ptr;
			t.commit();
		}
		catch(const Wt::Dbo::StaleObjectException &)
		{
			app->dboSession().rereadAll();
			app->showStaleObjectError();
		}
		catch(const Wt::Dbo::Exception &e)
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

	EmployeeAssignmentView::EmployeeAssignmentView(Wt::Dbo::ptr<EmployeeAssignment> assignmentPtr /*= Wt::Dbo::ptr<EmployeeAssignment>()*/)
		: _tempPtr(assignmentPtr)
	{
		setTemplateText(tr("GS.Admin.EmployeeAssignmentView"));
	}

	void EmployeeAssignmentView::initView()
	{
		_model = std::make_shared<EmployeeAssignmentFormModel>(this, _tempPtr);
		addFormModel("assignment", _model);
	}

	void EmployeeAssignmentFormModel::persistedHandler()
	{
		setReadOnly(startDateField, true);

		_view->bindNew<Wt::WAnchor>("expenseCycle");
		_view->bindNew<Wt::WAnchor>("clientAssignment");

		Wt::WPushButton *setExpenseCycle = _view->bindNew<Wt::WPushButton>("setExpenseCycle", tr("AssociateWithRecurringExpense"));
		setExpenseCycle->clicked().connect(this, &EmployeeAssignmentFormModel::showExpenseCycleDialog);

		Wt::WPushButton *setClientAssignment = _view->bindNew<Wt::WPushButton>("setClientAssignment", tr("AssociateWithClientAssignment"));
		setClientAssignment->clicked().connect(this, &EmployeeAssignmentFormModel::showClientAssignmentDialog);

		_view->reload();
	}

	void EmployeeAssignmentView::reload()
	{
		TRANSACTION(APP);
		bool isPersisted = _model->isRecordPersisted();
		if(isPersisted)
		{
			Wt::Dbo::ptr<EmployeeAssignment> assignmentPtr = _model->recordPtr();
			assignmentPtr.reread();

			try
			{
				if(assignmentPtr->expenseCyclePtr)
				{
					Wt::WString text = tr("AssociatedEntryCycleSummary").arg(assignmentPtr->expenseCyclePtr->entityPtr->name)
						.arg(rsEveryNIntervals(assignmentPtr->expenseCyclePtr->amount(), assignmentPtr->expenseCyclePtr->interval, assignmentPtr->expenseCyclePtr->nIntervals));
					Wt::WAnchor *a = resolve<Wt::WAnchor*>("expenseCycle");
					a->setText(text);
					a->setLink(Wt::WLink(Wt::LinkType::InternalPath, ExpenseCycle::viewInternalPath(assignmentPtr->expenseCyclePtr.id())));
				}
				setCondition("show-expenseCycle", (bool)assignmentPtr->expenseCyclePtr);
				resolveWidget("setExpenseCycle")->setHidden((bool)assignmentPtr->expenseCyclePtr);
			}
			catch(const Wt::Dbo::Exception &e)
			{
				Wt::log("error") << "EmployeeAssignmentView::reload(): Dbo error(" << e.code() << ") reloading expense cycle summary: " << e.what();
				setCondition("show-expenseCycle", false);
				resolveWidget("setExpenseCycle")->setHidden(true);
			}

			try
			{
				if(assignmentPtr->clientAssignmentPtr)
				{
					Wt::WString text = tr("AssociatedClientAssignmentSummary").arg(assignmentPtr->clientAssignmentPtr->entityPtr->name)
						.arg(assignmentPtr->clientAssignmentPtr->servicePtr->title);
					Wt::WAnchor *a = resolve<Wt::WAnchor*>("clientAssignment");
					a->setText(text);
					a->setLink(Wt::WLink(Wt::LinkType::InternalPath, ClientAssignment::viewInternalPath(assignmentPtr->clientAssignmentPtr.id())));
				}
				setCondition("show-clientAssignment", (bool)assignmentPtr->clientAssignmentPtr);
				resolveWidget("setClientAssignment")->setHidden((bool)assignmentPtr->clientAssignmentPtr);
			}
			catch(const Wt::Dbo::Exception &e)
			{
				Wt::log("error") << "EmployeeAssignmentView::reload(): Dbo error(" << e.code() << ") reloading client assignment summary: " << e.what();
				setCondition("show-clientAssignment", false);
				resolveWidget("setClientAssignment")->setHidden(true);
			}
		}
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

		_dialog = addChild(std::make_unique<Wt::WDialog>(tr("AddNewX").arg(tr("position"))));
		_dialog->setClosable(true);
		_dialog->setTransient(true);
		_dialog->rejectWhenEscapePressed(true);
		_dialog->setWidth(Wt::WLength(500));
		PositionView *positionView = _dialog->contents()->addNew<PositionView>();

		_dialog->finished().connect(this, std::bind([this](Wt::DialogCode code) {
			if(code == Wt::DialogCode::Rejected)
			{
				_model->setValue(EmployeeAssignmentFormModel::positionField, Wt::Dbo::ptr<EmployeePosition>());
				updateViewField(_model.get(), EmployeeAssignmentFormModel::positionField);
			}
			_dialog->removeFromParent();
			_dialog = nullptr;
		}, _1));

		positionView->submitted().connect(this, std::bind([this, positionView]() {
			_model->setValue(EmployeeAssignmentFormModel::positionField, positionView->positionPtr());
			updateViewField(_model.get(), EmployeeAssignmentFormModel::positionField);
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

		return RecordFormView::viewName();
	}

	void EmployeeAssignmentFormModel::updateEndDateValidator(bool update)
	{
		if(update)
			_view->updateModelField(this, EmployeeAssignmentFormModel::startDateField);

		const Wt::any &startDateVal = value(EmployeeAssignmentFormModel::startDateField);
		if(startDateVal.empty())
			return;

		Wt::WDate startDate = Wt::any_cast<Wt::WDate>(startDateVal);
		auto endDateValidator = std::static_pointer_cast<Wt::WDateValidator>(validator(EmployeeAssignmentFormModel::endDateField));
		endDateValidator->setBottom(startDate.isValid() ? startDate.addDays(1) : Wt::WDate());
	}

	EmployeeAssignmentListProxyModel::EmployeeAssignmentListProxyModel(std::shared_ptr<Wt::WAbstractItemModel> model)
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
				return tr("GS.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const EmployeeAssignmentList::ResultType &res = std::static_pointer_cast<Wt::Dbo::QueryModel<EmployeeAssignmentList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<EmployeeAssignmentList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, EmployeeAssignment::viewInternalPath(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = Wt::any_cast<int>(viewIndexData);

		const EmployeeAssignmentList::ResultType &res = std::static_pointer_cast<Wt::Dbo::QueryModel<EmployeeAssignmentList::ResultType>>(sourceModel())->resultRow(idx.row());

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
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "a.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1))); filtersTemplate()->addFilter(1);
	}

	void EmployeeAssignmentList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT a.id, a.timestamp, e.name, a.startDate, a.endDate, cnt.name, city.name, l.address FROM "
			+ std::string(EmployeeAssignment::tableName()) + " a "
			"INNER JOIN " + Entity::tableName() + " e ON (e.id = a.entity_id) "
			"LEFT JOIN " + Location::tableName() + " l ON (l.id = a.location_id) "
			"LEFT JOIN " + Country::tableName() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tableName() + " city ON (city.id = l.city_id)");

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

		_proxyModel = std::make_shared<EmployeeAssignmentListProxyModel>(_model);
	}

	const Wt::WFormModel::Field ClientAssignmentFormModel::descriptionField = "description";
	const Wt::WFormModel::Field ClientAssignmentFormModel::startDateField = "startDate";
	const Wt::WFormModel::Field ClientAssignmentFormModel::endDateField = "endDate";
	const Wt::WFormModel::Field ClientAssignmentFormModel::entityField = "entity";
	const Wt::WFormModel::Field ClientAssignmentFormModel::cycleField = "cycle";
	const Wt::WFormModel::Field ClientAssignmentFormModel::serviceField = "service";

	ClientAssignmentFormModel::ClientAssignmentFormModel(ClientAssignmentView *view, Wt::Dbo::ptr<ClientAssignment> clientAssignmentPtr /*= Wt::Dbo::ptr<ClientAssignment>()*/)
		: RecordFormModel(view, clientAssignmentPtr), _view(view)
	{
		addField(descriptionField);
		addField(startDateField);
		addField(endDateField);
		addField(entityField);
		addField(cycleField);
		addField(serviceField);

		setVisible(cycleField, false);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(descriptionField, Wt::WString::fromUTF8(_recordPtr->description));
			setValue(startDateField, _recordPtr->startDate);
			setValue(endDateField, _recordPtr->endDate);
			setValue(entityField, _recordPtr->entityPtr);
			setValue(cycleField, _recordPtr->incomeCyclePtr);
			setValue(serviceField, _recordPtr->servicePtr);
		}
	}

	std::unique_ptr<Wt::WWidget> ClientAssignmentFormModel::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == descriptionField)
		{
			auto description = std::make_unique<Wt::WTextArea>();
			description->setRows(3);
			return description;
		}
		if(field == startDateField)
		{
			auto startDateEdit = std::make_unique<Wt::WDateEdit>();
			startDateEdit->setPlaceholderText(Wt::WLocale::currentLocale().dateFormat());
			auto startDateValidator = std::make_shared<Wt::WDateValidator>();
			startDateValidator->setMandatory(true);
			setValidator(startDateField, startDateValidator);
			startDateEdit->changed().connect(this, std::bind(&ClientAssignmentFormModel::updateEndDateValidator, this, true));
			return startDateEdit;
		}
		if(field == endDateField)
		{
			auto endDateEdit = std::make_unique<Wt::WDateEdit>();
			endDateEdit->setPlaceholderText(tr("EmptyEndDate"));
			auto endDateValidator = std::make_shared<Wt::WDateValidator>();
			setValidator(endDateField, endDateValidator);
			return endDateEdit;
		}
		if(field == entityField)
		{
			auto findEntityEdit = std::make_unique<FindEntityEdit>();
			auto findEntityValidator = std::make_shared<FindEntityValidator>(findEntityEdit.get(), true);
			findEntityValidator->setModifyPermissionRequired(true);
			setValidator(entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == serviceField)
		{
			WApplication *app = APP;
			app->initServiceQueryModel();

			auto serviceCombo = std::make_unique<QueryProxyModelCB<ServiceProxyModel>>(app->serviceProxyModel());
			auto validator = std::make_shared<QueryProxyModelCBValidator<ServiceProxyModel>>(serviceCombo.get());
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
			_recordPtr = app->dboSession().add(std::make_unique<ClientAssignment>());
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->description = valueText(descriptionField).toUTF8();
		_recordPtr.modify()->startDate = Wt::any_cast<Wt::WDate>(value(startDateField));
		_recordPtr.modify()->endDate = Wt::any_cast<Wt::WDate>(value(endDateField));
		_recordPtr.modify()->entityPtr = Wt::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->servicePtr = Wt::any_cast<Wt::Dbo::ptr<ClientService>>(value(serviceField));

		const Wt::any &cycleVal = value(cycleField);
		_recordPtr.modify()->incomeCyclePtr = cycleVal.empty() ? Wt::Dbo::ptr<IncomeCycle>() : Wt::any_cast<Wt::Dbo::ptr<IncomeCycle>>(cycleVal);

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

		auto selectionDialog = addChild(std::make_unique<ListSelectionDialog<IncomeCycleList>>(tr("SelectXFromList").arg(tr("recurringIncome"))));
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
			Wt::Dbo::ptr<IncomeCycle> ptr = app->dboSession().loadLazy<IncomeCycle>(id);
			_recordPtr.modify()->incomeCyclePtr = ptr;
			t.commit();
		}
		catch(const Wt::Dbo::StaleObjectException &)
		{
			app->dboSession().rereadAll();
			app->showStaleObjectError();
		}
		catch(const Wt::Dbo::Exception &e)
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

	ClientAssignmentView::ClientAssignmentView(Wt::Dbo::ptr<ClientAssignment> assignmentPtr /*= Wt::Dbo::ptr<ClientAssignment>()*/)
		: _tempPtr(assignmentPtr)
	{
		setTemplateText(tr("GS.Admin.ClientAssignmentView"));
	}

	void ClientAssignmentView::initView()
	{
		_model = std::make_shared<ClientAssignmentFormModel>(this, _tempPtr);
		addFormModel("assignment", _model);

		bindNew<Wt::WAnchor>("incomeCycle");
	}

	void ClientAssignmentView::reload()
	{
		TRANSACTION(APP);
		bool isPersisted = _model->isRecordPersisted();
		if(isPersisted)
		{
			Wt::Dbo::ptr<ClientAssignment> assignmentPtr = _model->recordPtr();
			assignmentPtr.reread();

			try
			{
				if(assignmentPtr->incomeCyclePtr)
				{
					Wt::WString text = tr("AssociatedEntryCycleSummary").arg(assignmentPtr->incomeCyclePtr->entityPtr->name)
						.arg(rsEveryNIntervals(assignmentPtr->incomeCyclePtr->amount(), assignmentPtr->incomeCyclePtr->interval, assignmentPtr->incomeCyclePtr->nIntervals));
					Wt::WAnchor *a = resolve<Wt::WAnchor*>("incomeCycle");
					a->setText(text);
					a->setLink(Wt::WLink(Wt::LinkType::InternalPath, IncomeCycle::viewInternalPath(assignmentPtr->incomeCyclePtr.id())));
				}
				setCondition("show-incomeCycle", (bool)assignmentPtr->incomeCyclePtr);
				setCondition("set-incomeCycle", !assignmentPtr->incomeCyclePtr);
			}
			catch(const Wt::Dbo::Exception &e)
			{
				Wt::log("error") << "ClientAssignmentView::reload(): Dbo error(" << e.code() << ") reloading income cycle summary: " << e.what();
				setCondition("show-incomeCycle", false);
				setCondition("set-incomeCycle", false);
			}
		}
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

		_dialog = addChild(std::make_unique<Wt::WDialog>(tr("AddNewX").arg(tr("service"))));
		_dialog->setTransient(true);
		_dialog->rejectWhenEscapePressed(true);
		_dialog->setClosable(true);
		_dialog->setWidth(Wt::WLength(500));
		ServiceView *serviceView  = _dialog->contents()->addNew<ServiceView>();

		_dialog->finished().connect(this, std::bind([this](Wt::DialogCode code) {
			if(code == Wt::DialogCode::Rejected)
			{
				_model->setValue(ClientAssignmentFormModel::serviceField, Wt::Dbo::ptr<ClientService>());
				updateViewField(_model.get(), ClientAssignmentFormModel::serviceField);
			}
			_dialog->removeFromParent();
			_dialog = nullptr;
		}, _1));

		serviceView->submitted().connect(this, std::bind([this, serviceView]() {
			_model->setValue(ClientAssignmentFormModel::serviceField, serviceView->servicePtr());
			updateViewField(_model.get(), ClientAssignmentFormModel::serviceField);
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

		return RecordFormView::viewName();
	}

	void ClientAssignmentFormModel::persistedHandler()
	{
		setReadOnly(startDateField, true);

		_view->bindNew<EmployeeAssignmentList>("employeeAssignments", recordPtr());

		Wt::WPushButton *setIncomeCycle = _view->bindNew<Wt::WPushButton>("setIncomeCycle", tr("AssociateWithRecurringIncome"));
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
		auto endDateValidator = std::static_pointer_cast<Wt::WDateValidator>(validator(ClientAssignmentFormModel::endDateField));
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
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "ca.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1))); filtersTemplate()->addFilter(1);
	}

	void ClientAssignmentList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT ca.id, ca.timestamp, e.name, ca.startDate, ca.endDate, COUNT(ea.id) FROM "
			+ std::string(ClientAssignment::tableName()) + " ca "
			"INNER JOIN " + Entity::tableName() + " e ON (e.id = ca.entity_id) "
			"LEFT JOIN " + EmployeeAssignment::tableName() + " ea ON ea.clientassignment_id = ca.id"
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

		_proxyModel = std::make_shared<ClientAssignmentListProxyModel>(_model);
	}

	ClientAssignmentListProxyModel::ClientAssignmentListProxyModel(std::shared_ptr<Wt::WAbstractItemModel> model)
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

	Wt::any ClientAssignmentListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Orientation::Horizontal*/, Wt::ItemDataRole role /*= Wt::ItemDataRole::Display*/) const
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
				return tr("GS.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const ClientAssignmentList::ResultType &res = std::static_pointer_cast<Wt::Dbo::QueryModel<ClientAssignmentList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<ClientAssignmentList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, ClientAssignment::viewInternalPath(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = Wt::any_cast<int>(viewIndexData);

		const ClientAssignmentList::ResultType &res = std::static_pointer_cast<Wt::Dbo::QueryModel<ClientAssignmentList::ResultType>>(sourceModel())->resultRow(idx.row());

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

// 
// 	Wt::WString MultipleViewTemplate::viewName() const
// 	{
// 		if(_mainView)
// 			return _mainView->viewName();
// 
// 		return SubmittableRecordWidget::viewName();
// 	}
// 
// 	std::string MultipleViewTemplate::viewInternalPath() const
// 	{
// 		if(_mainView)
// 			return _mainView->viewInternalPath();
// 
// 		return SubmittableRecordWidget::viewInternalPath();
// 	}
// 
// 	EmployeeExpenseView::EmployeeExpenseView(bool isEmployeeAssignment)
// 		: _isEmployeeAssignment(isEmployeeAssignment)
// 	{
// 		setTemplateText(_isEmployeeAssignment ? tr("GS.Admin.EmployeeExpenseView.Assignment") : tr("GS.Admin.EmployeeExpenseView.ExpenseCycle"));
// 	}
// 
// 	EmployeeExpenseView::EmployeeExpenseView(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr)
// 		: MultipleViewTemplate(tr("GS.Admin.EmployeeExpenseView.Assignment")), _isEmployeeAssignment(true), _tempAssignment(employeeAssignmentPtr)
// 	{ }
// 
// 	EmployeeExpenseView::EmployeeExpenseView(Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr)
// 		: MultipleViewTemplate(tr("GS.Admin.EmployeeExpenseView.ExpenseCycle")), _isEmployeeAssignment(false), _tempCycle(expenseCyclePtr)
// 	{ }
// 
// 	void EmployeeExpenseView::load()
// 	{
// 		if(!loaded())
// 		{
// 			_assignments = new Wt::WContainerWidget();
// 			_cycles = new Wt::WContainerWidget();
// 
// 			auto addAssignmentBtn = new ShowEnabledButton(tr("AssignEmployeeBtnLabel"));
// 			addAssignmentBtn->clicked().connect(this, &EmployeeExpenseView::handleAddAssignment);
// 			bindWidget("add-assignment", addAssignmentBtn);
// 
// 			auto addCycleBtn = new ShowEnabledButton(tr("AssignRecurringExpenseBtnLabel"));
// 			addCycleBtn->clicked().connect(this, &EmployeeExpenseView::handleAddCycle);
// 			bindWidget("add-cycle", addCycleBtn);
// 
// 			if(_isEmployeeAssignment)
// 			{
// 				_mainView = addAssignment(_tempAssignment);
// 				_mainView->load();
// 				if(_tempAssignment)
// 				{
// 					TRANSACTION(APP);
// 					if(_tempAssignment->expenseCyclePtr)
// 						addCycle(_tempAssignment->expenseCyclePtr);
// 				}
// 			}
// 			else
// 			{
// 				_mainView = addCycle(_tempCycle);
// 				_mainView->load();
// 				if(_tempCycle)
// 				{
// 					TRANSACTION(APP);
// 					EmployeeAssignmentCollection collection = _tempCycle->employeeAssignmentCollection;
// 					for(const auto &ptr : collection)
// 						addAssignment(ptr);
// 				}
// 			}
// 
// 			_mainView->submitted().connect(std::bind([this]() {
// 				handleMainViewSubmitted();
// 				submitted().emit();
// 			}));
// 
// 			bindWidget("assignments", _assignments);
// 			bindWidget("cycles", _cycles);
// 			updateFromViewCounts();
// 		}
// 
// 		Wt::WTemplate::load();
// 	}
// 
// 	// 	EmployeeAssignmentView *EmployeeExpenseView::assignmentView(int index) const
// 	// 	{
// 	// 		return _assignments ? (index < _assignments->count()) ? dynamic_cast<EmployeeAssignmentView*>(_assignments->widget(index)) : nullptr : nullptr;
// 	// 	}
// 	// 
// 	// 	ExpenseCycleView *EmployeeExpenseView::cycleView(int index) const
// 	// 	{
// 	// 		return _cycles ? (index < _cycles->count()) ? dynamic_cast<ExpenseCycleView*>(_cycles->widget(index)) : nullptr : nullptr;
// 	// 	}
// 
// 	void EmployeeExpenseView::handleAddCycle()
// 	{
// 		if(!canAddCycle())
// 			return;
// 
// 		addCycle(Wt::Dbo::ptr<ExpenseCycle>());
// 		updateFromViewCounts();
// 	}
// 
// 	void EmployeeExpenseView::handleAddAssignment()
// 	{
// 		if(!canAddAssignment())
// 			return;
// 
// 		addAssignment(Wt::Dbo::ptr<EmployeeAssignment>());
// 		updateFromViewCounts();
// 	}
// 
// 	bool EmployeeExpenseView::canAddAssignment() const
// 	{
// 		return _cycleView && _cycleView->model()->isRecordPersisted();
// 	}
// 
// 	EmployeeAssignmentView *EmployeeExpenseView::addAssignment(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr)
// 	{
// 		EmployeeAssignmentView *newView = new EmployeeAssignmentView(employeeAssignmentPtr);
// 		if(!_isEmployeeAssignment)
// 		{
// 			if(ExpenseCycleView *cMainView = dynamic_cast<ExpenseCycleView*>(_mainView))
// 			{
// 				//_mainView->loaded() == true is asserted
// 				if(cMainView->model()->isRecordPersisted())
// 				{
// 					newView->load();
// 					newView->model()->setValue(EmployeeAssignmentFormModel::cycleField, cMainView->cyclePtr());
// 
// 					const Wt::any &entityVal = cMainView->model()->value(EntryCycleFormModel::entityField);
// 					if(!entityVal.empty())
// 						newView->model()->setValue(EmployeeAssignmentFormModel::entityField, Wt::any_cast<Wt::Dbo::ptr<Entity>>(entityVal));
// 
// 					newView->updateViewField(newView->model(), EmployeeAssignmentFormModel::entityField);
// 				}
// 			}
// 		}
// 
// 		_assignments->addWidget(newView);
// 		return newView;
// 	}
// 
// 	ExpenseCycleView *EmployeeExpenseView::addCycle(Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr)
// 	{
// 		ExpenseCycleView *newView = new ExpenseCycleView(expenseCyclePtr);
// 		if(_isEmployeeAssignment)
// 		{
// 			if(EmployeeAssignmentView *cMainView = dynamic_cast<EmployeeAssignmentView*>(_mainView))
// 			{
// 				//_mainView->loaded() == true is asserted
// 				if(cMainView->model()->isRecordPersisted())
// 				{
// 					newView->load();
// 					const Wt::any &entityVal = cMainView->model()->value(EmployeeAssignmentFormModel::entityField);
// 					if(!entityVal.empty())
// 						newView->model()->setValue(EntryCycleFormModel::entityField, Wt::any_cast<Wt::Dbo::ptr<Entity>>(entityVal));
// 
// 					newView->updateViewField(newView->model(), EntryCycleFormModel::entityField);
// 				}
// 			}
// 		}
// 
// 		_cycles->addWidget(newView);
// 		_cycleView = newView;
// 		return newView;
// 	}
// 
// 	void EmployeeExpenseView::handleMainViewSubmitted()
// 	{
// 		if(!_isEmployeeAssignment)
// 		{
// 			auto cMainView = dynamic_cast<ExpenseCycleView*>(_mainView);
// 			for(int i = 0; i < _assignments->count(); ++i)
// 			{
// 				auto view = dynamic_cast<EmployeeAssignmentView*>(_assignments->widget(i));
// 				view->model()->setValue(EmployeeAssignmentFormModel::cycleField, cMainView->cyclePtr());
// 				view->updateViewField(view->model(), EmployeeAssignmentFormModel::entityField);
// 			}
// 		}
// 		updateFromViewCounts();
// 	}
// 
// 	void EmployeeExpenseView::updateFromViewCounts()
// 	{
// 		if(_isEmployeeAssignment)
// 			_cycles->setDisabled(!canSubmitAdditionalViews());
// 		else
// 			_assignments->setDisabled(!canSubmitAdditionalViews());
// 
// 		resolveWidget("add-cycle")->setDisabled(!canAddCycle());
// 		resolveWidget("add-assignment")->setDisabled(!canAddAssignment());
// 	}
// 
// ClientIncomeView::ClientIncomeView(bool isClientAssignment)
// 	: _isClientAssignment(isClientAssignment)
// {
// 	setTemplateText(_isClientAssignment ? tr("GS.Admin.ClientIncomeView.Assignment") : tr("GS.Admin.ClientIncomeView.IncomeCycle"));
// }
// 
// ClientIncomeView::ClientIncomeView(Wt::Dbo::ptr<ClientAssignment> clientAssignmentPtr)
// 	: MultipleViewTemplate(tr("GS.Admin.ClientIncomeView.Assignment")), _isClientAssignment(true), _tempAssignment(clientAssignmentPtr)
// { }
// 
// ClientIncomeView::ClientIncomeView(Wt::Dbo::ptr<IncomeCycle> incomeCyclePtr)
// 	: MultipleViewTemplate(tr("GS.Admin.ClientIncomeView.IncomeCycle")), _isClientAssignment(false), _tempCycle(incomeCyclePtr)
// { }
// 
// void ClientIncomeView::load()
// {
// 	if(!loaded())
// 	{
// 		_assignments = new Wt::WContainerWidget();
// 		_cycles = new Wt::WContainerWidget();
// 
// 		auto addAssignmentBtn = new ShowEnabledButton(tr("AssignClientBtnLabel"));
// 		addAssignmentBtn->clicked().connect(this, &ClientIncomeView::handleAddAssignment);
// 		bindWidget("add-assignment", addAssignmentBtn);
// 
// 		auto addCycleBtn = new ShowEnabledButton(tr("AssignRecurringIncomeBtnLabel"));
// 		addCycleBtn->clicked().connect(this, &ClientIncomeView::handleAddCycle);
// 		bindWidget("add-cycle", addCycleBtn);
// 
// 		if(_isClientAssignment)
// 		{
// 			_mainView = addAssignment(_tempAssignment);
// 			_mainView->load();
// 			if(_tempAssignment)
// 			{
// 				TRANSACTION(APP);
// 				if(_tempAssignment->incomeCyclePtr)
// 					addCycle(_tempAssignment->incomeCyclePtr);
// 			}
// 		}
// 		else
// 		{
// 			_mainView = addCycle(_tempCycle);
// 			_mainView->load();
// 			if(_tempCycle)
// 			{
// 				TRANSACTION(APP);
// 				ClientAssignmentCollection collection = _tempCycle->clientAssignmentCollection;
// 				for(const auto &ptr : collection)
// 					addAssignment(ptr);
// 			}
// 		}
// 
// 		_mainView->submitted().connect(std::bind([this]() {
// 			handleMainViewSubmitted();
// 			submitted().emit();
// 		}));
// 
// 		bindWidget("assignments", _assignments);
// 		bindWidget("cycles", _cycles);
// 		updateFromViewCounts();
// 	}
// 
// 	Wt::WTemplate::load();
// }
// 
// // 	ClientAssignmentView *ClientIncomeView::assignmentView(int index) const
// // 	{
// // 		return _assignments ? (index < _assignments->count()) ? dynamic_cast<ClientAssignmentView*>(_assignments->widget(index)) : nullptr : nullptr;
// // 	}
// // 
// // 	IncomeCycleView *ClientIncomeView::cycleView(int index) const
// // 	{
// // 		return _cycles ? (index < _cycles->count()) ? dynamic_cast<IncomeCycleView*>(_cycles->widget(index)) : nullptr : nullptr;
// // 	}
// 
// void ClientIncomeView::handleAddCycle()
// {
// 	if(!canAddCycle())
// 		return;
// 
// 	addCycle(Wt::Dbo::ptr<IncomeCycle>());
// 	updateFromViewCounts();
// }
// 
// void ClientIncomeView::handleAddAssignment()
// {
// 	if(!canAddAssignment())
// 		return;
// 
// 	addAssignment(Wt::Dbo::ptr<ClientAssignment>());
// 	updateFromViewCounts();
// }
// 
// bool ClientIncomeView::canAddAssignment() const
// {
// 	return _cycleView && _cycleView->model()->isRecordPersisted();
// }
// 
// ClientAssignmentView *ClientIncomeView::addAssignment(Wt::Dbo::ptr<ClientAssignment> clientAssignmentPtr)
// {
// 	ClientAssignmentView *newView = new ClientAssignmentView(clientAssignmentPtr);
// 	if(!_isClientAssignment)
// 	{
// 		if(IncomeCycleView *cMainView = dynamic_cast<IncomeCycleView*>(_mainView))
// 		{
// 			//_mainView->loaded() == true is asserted
// 			if(cMainView->model()->isRecordPersisted())
// 			{
// 				newView->load();
// 				newView->model()->setValue(ClientAssignmentFormModel::cycleField, cMainView->cyclePtr());
// 
// 				const Wt::any &entityVal = cMainView->model()->value(EntryCycleFormModel::entityField);
// 				if(!entityVal.empty())
// 					newView->model()->setValue(ClientAssignmentFormModel::entityField, Wt::any_cast<Wt::Dbo::ptr<Entity>>(entityVal));
// 
// 				newView->updateViewField(newView->model(), ClientAssignmentFormModel::entityField);
// 			}
// 		}
// 	}
// 
// 	_assignments->addWidget(newView);
// 	return newView;
// }
// 
// IncomeCycleView *ClientIncomeView::addCycle(Wt::Dbo::ptr<IncomeCycle> incomeCyclePtr)
// {
// 	IncomeCycleView *newView = new IncomeCycleView(incomeCyclePtr);
// 	if(_isClientAssignment)
// 	{
// 		if(ClientAssignmentView *cMainView = dynamic_cast<ClientAssignmentView*>(_mainView))
// 		{
// 			//_mainView->loaded() == true is asserted
// 			if(cMainView->model()->isRecordPersisted())
// 			{
// 				newView->load();
// 				const Wt::any &entityVal = cMainView->model()->value(ClientAssignmentFormModel::entityField);
// 				if(!entityVal.empty())
// 					newView->model()->setValue(EntryCycleFormModel::entityField, Wt::any_cast<Wt::Dbo::ptr<Entity>>(entityVal));
// 
// 				newView->updateViewField(newView->model(), EntryCycleFormModel::entityField);
// 			}
// 		}
// 	}
// 
// 	_cycles->addWidget(newView);
// 	_cycleView = newView;
// 	return newView;
// }
// 
// void ClientIncomeView::handleMainViewSubmitted()
// {
// 	if(!_isClientAssignment)
// 	{
// 		auto cMainView = dynamic_cast<IncomeCycleView*>(_mainView);
// 		for(int i = 0; i < _assignments->count(); ++i)
// 		{
// 			auto view = dynamic_cast<ClientAssignmentView*>(_assignments->widget(i));
// 			view->model()->setValue(ClientAssignmentFormModel::cycleField, cMainView->cyclePtr());
// 			view->updateViewField(view->model(), ClientAssignmentFormModel::entityField);
// 		}
// 	}
// 	updateFromViewCounts();
// }
// 
// void ClientIncomeView::updateFromViewCounts()
// {
// 	if(_isClientAssignment)
// 		_cycles->setDisabled(!canSubmitAdditionalViews());
// 	else
// 		_assignments->setDisabled(!canSubmitAdditionalViews());
// 
// 	resolveWidget("add-cycle")->setDisabled(!canAddCycle());
// 	resolveWidget("add-assignment")->setDisabled(!canAddAssignment());
// }

}
