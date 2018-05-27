#include "ModelView/AttendanceMVC.h"
#include <Wt/WLengthValidator.h>
#include <Wt/WDateEdit.h>
#include <Wt/WTimeEdit.h>
#include <Wt/WTableView.h>

namespace ERP
{
	void AttendanceDeviceList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "d.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilter(1);
	}

	void AttendanceDeviceList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT d.id, dv.hostName, cnt.name, city.name, l.address FROM "
			+ AttendanceDevice::tStr() + " d "
			"INNER JOIN " + AttendanceDeviceV::tStr() + " dv "
			"ON dv.id = (SELECT dvi.id FROM "+AttendanceDeviceV::tStr()+" dvi WHERE dvi.parent_id = d.id ORDER BY dvi.`timestamp` DESC LIMIT 1) "
			"LEFT JOIN " + Location::tStr() + " l ON (l.id = dv.location_id) "
			"LEFT JOIN " + Country::tStr() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tStr() + " city ON (city.id = l.city_id)");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("d.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewHostName, model->addColumn("dv.hostName"), tr("Hostname"), 200);
		addColumn(ViewCountry, model->addColumn("cnt.name"), tr("Country"), 150);
		addColumn(ViewCity, model->addColumn("city.name"), tr("City"), 150);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), 300);

		_proxyModel = make_shared<AttendanceDeviceListProxyModel>(_model);
	}

	AttendanceDeviceListProxyModel::AttendanceDeviceListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void AttendanceDeviceListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::WFlags<Wt::ItemFlag> AttendanceDeviceListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::any AttendanceDeviceListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any AttendanceDeviceListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const AttendanceDeviceList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AttendanceDeviceList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<AttendanceDeviceList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, AttendanceDevice::viewInternalPath(id));
			}
		}
		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	const Wt::WFormModel::Field AttendanceDeviceFormModel::hostNameField = "hostName";
	const Wt::WFormModel::Field AttendanceDeviceFormModel::locationField = "location";

	AttendanceDeviceFormModel::AttendanceDeviceFormModel(AttendanceDeviceView *view, Dbo::ptr<AttendanceDevice> attendanceDevicePtr)
		: RecordFormModel(view, move(attendanceDevicePtr)), _view(view)
	{
		addField(hostNameField);
		addField(locationField);
	}

	void AttendanceDeviceFormModel::updateFromDb()
	{
		WApplication *app = APP;
		TRANSACTION(app);

		auto latestVersionPtr = app->dboSession().loadLatestVersion<AttendanceDeviceV>(_recordPtr);
		setValue(hostNameField, Wt::WString::fromUTF8(latestVersionPtr->hostName));
		setValue(locationField, latestVersionPtr->locationPtr);
	}

	unique_ptr<Wt::WWidget> AttendanceDeviceFormModel::createFormWidget(Field field)
	{
		if(field == hostNameField)
		{
			auto hostName = make_unique<Wt::WLineEdit>();
			hostName->setMaxLength(255);
			auto validator = make_shared<Wt::WLengthValidator>(0, 255);
			validator->setMandatory(true);
			setValidator(hostNameField, validator);
			return hostName;
		}
		if(field == locationField)
		{
			auto findLocationEdit = make_unique<FindLocationEdit>();
			auto findLocationValidator = make_shared<FindLocationValidator>(findLocationEdit.get(), true);
			setValidator(locationField, findLocationValidator);
			return findLocationEdit;
		}
		return nullptr;
	}

	bool AttendanceDeviceFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().addNew<AttendanceDevice>();
			_recordPtr.modify()->setCreatedByValues();
		}

		Dbo::ptr<AttendanceDeviceV> newVersionPtr = app->dboSession().addNew<AttendanceDeviceV>(_recordPtr);
		newVersionPtr.modify()->setModifiedByValues();
		newVersionPtr.modify()->hostName = valueText(hostNameField).toUTF8();
		newVersionPtr.modify()->locationPtr = Wt::any_cast<Dbo::ptr<Location>>(value(locationField));
		return true;
	}

	AttendanceDeviceView::AttendanceDeviceView(Dbo::ptr<AttendanceDevice> attendanceDevicePtr)
		: RecordFormView(tr("ERP.Admin.AttendanceDeviceView"))
	{
		_model = newFormModel<AttendanceDeviceFormModel>("attendance-device", this, move(attendanceDevicePtr));
	}

	Wt::WString AttendanceDeviceView::viewName() const
	{
		if(attendanceDevicePtr())
		{
			TRANSACTION(APP);
			return tr("AttendanceDeviceViewName").arg(attendanceDevicePtr().id());
		}
		return "AttendanceDeviceView";
	}

	const Wt::WFormModel::Field AttendanceEntryFormModel::entityField = "entity";
	const Wt::WFormModel::Field AttendanceEntryFormModel::dateInField = "dateIn";
	const Wt::WFormModel::Field AttendanceEntryFormModel::timeInField = "timeIn";
	const Wt::WFormModel::Field AttendanceEntryFormModel::dateOutField = "dateOut";
	const Wt::WFormModel::Field AttendanceEntryFormModel::timeOutField = "timeOut";
	const Wt::WFormModel::Field AttendanceEntryFormModel::locationField = "location";

	AttendanceEntryFormModel::AttendanceEntryFormModel(AttendanceEntryView *view, Dbo::ptr<AttendanceEntry> attendanceEntryPtr)
		: RecordFormModel(view, move(attendanceEntryPtr)), _view(view)
	{
		addField(entityField);
		addField(dateInField);
		addField(timeInField);
		addField(dateOutField);
		addField(timeOutField);
		addField(locationField);
	}

	void AttendanceEntryFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(entityField, _recordPtr->entityPtr);
		setValue(dateInField, _recordPtr->timestampIn.date());
		setValue(timeInField, _recordPtr->timestampIn.time());
		setValue(dateOutField, _recordPtr->timestampOut.date());
		setValue(timeOutField, _recordPtr->timestampOut.time());
		setValue(locationField, _recordPtr->locationPtr);
	}

	unique_ptr<Wt::WWidget> AttendanceEntryFormModel::createFormWidget(Field field)
	{
		if(field == entityField)
		{
			auto findEntityEdit = make_unique<FindEntityEdit>();
			auto findEntityValidator = make_shared<FindEntityValidator>(findEntityEdit.get(), true);
			findEntityValidator->setModifyPermissionRequired(true);
			setValidator(entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == locationField)
		{
			auto findLocationEdit = make_unique<FindLocationEdit>();
			auto findLocationValidator = make_shared<FindLocationValidator>(findLocationEdit.get(), false);
			setValidator(locationField, findLocationValidator);
			return findLocationEdit;
		}
		if(field == dateInField)
		{
			auto edit = make_unique<Wt::WDateEdit>();
			edit->changed().connect(this, std::bind(&AttendanceEntryFormModel::updateTimestampOutValidator, this, true));
			edit->validator()->setMandatory(true);
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == timeInField)
		{
			auto edit = make_unique<Wt::WTimeEdit>();
			edit->changed().connect(this, std::bind(&AttendanceEntryFormModel::updateTimestampOutValidator, this, true));
			edit->validator()->setMandatory(true);
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == dateOutField)
		{
			auto edit = make_unique<Wt::WDateEdit>();
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == timeOutField)
		{
			auto edit = make_unique<Wt::WTimeEdit>();
			setValidator(field, edit->validator());
			return edit;
		}
		return nullptr;
	}

	bool AttendanceEntryFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_recordPtr)
			_recordPtr = app->dboSession().addNew<AttendanceEntry>();

		_recordPtr.modify()->entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->locationPtr = Wt::any_cast<Dbo::ptr<Location>>(value(locationField));
		_recordPtr.modify()->timestampIn = Wt::WDateTime(Wt::any_cast<Wt::WDate>(value(dateInField)), Wt::any_cast<Wt::WTime>(value(timeInField)));
		Wt::WDateTime out(Wt::any_cast<Wt::WDate>(value(dateOutField)), Wt::any_cast<Wt::WTime>(value(timeOutField)));
		_recordPtr.modify()->timestampOut = out.isValid() ? out : Wt::WDateTime();
		return true;
	}

	void AttendanceEntryFormModel::updateTimestampOutValidator(bool update)
	{
		if(update)
		{
			_view->updateModelField(this, AttendanceEntryFormModel::dateInField);
			_view->updateModelField(this, AttendanceEntryFormModel::timeInField);
		}

		const Wt::any &dateInVal = value(AttendanceEntryFormModel::dateInField);
		const Wt::any &timeInVal = value(AttendanceEntryFormModel::timeInField);
		if(dateInVal.empty() || timeInVal.empty())
			return;

		Wt::WDateTime timestampIn = Wt::WDateTime(Wt::any_cast<Wt::WDate>(value(dateInField)), Wt::any_cast<Wt::WTime>(value(timeInField)));
		shared_ptr<Wt::WDateValidator> dateOutValidator = dynamic_pointer_cast<Wt::WDateValidator>(validator(AttendanceEntryFormModel::dateInField));
		shared_ptr<Wt::WTimeValidator> timeOutValidator = dynamic_pointer_cast<Wt::WTimeValidator>(validator(AttendanceEntryFormModel::timeInField));
		dateOutValidator->setBottom(timestampIn.date());
		timeOutValidator->setBottom(timestampIn.time());
	}

	AttendanceEntryView::AttendanceEntryView(Dbo::ptr<AttendanceEntry> attendanceEntryPtr)
		: RecordFormView(tr("ERP.Admin.AttendanceEntryView"))
	{
		_model = newFormModel<AttendanceEntryFormModel>("attendance-entry", this, move(attendanceEntryPtr));
	}

	Wt::WString AttendanceEntryView::viewName() const
	{
		if(attendanceEntryPtr())
		{
			TRANSACTION(APP);
			return tr("AttendanceEntryViewName").arg(attendanceEntryPtr().id()).arg(attendanceEntryPtr()->entityPtr->name);
		}
		return RecordFormView::viewName();
	}

	void AttendanceEntryView::updateView(Wt::WFormModel *model)
	{
		RecordFormView::updateView(model);
		_model->updateTimestampOutValidator(false);
	}

	void AttendanceEntryView::updateModel(Wt::WFormModel *model)
	{
		RecordFormView::updateModel(model);
		_model->updateTimestampOutValidator(false);
	}

	void AttendanceEntryList::load()
	{
		bool ld = !loaded();
		QueryModelFilteredList::load();

		if(ld)
		{
			int timestampColumn = viewIndexToColumn(ViewTimestampIn);
			if(timestampColumn != -1)
				_tableView->sortByColumn(timestampColumn, Wt::SortOrder::Descending);
		}
	}

	void AttendanceEntryList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "a.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilter(1);
	}

	void AttendanceEntryList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT a.id, e.name, a.timestampIn, a.timestampOut, cnt.name, city.name, l.address FROM "
			+ AttendanceEntry::tStr() + " a "
			"INNER JOIN " + Entity::tStr() + " e ON (e.id = a.entity_id) "
			"LEFT JOIN " + Location::tStr() + " l ON (l.id = a.location_id) "
			"LEFT JOIN " + Country::tStr() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tStr() + " city ON (city.id = l.city_id)");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("a.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), 200);
		addColumn(ViewTimestampIn, model->addColumn("a.timestampIn"), tr("TimeIn"), DateTimeColumnWidth);
		addColumn(ViewTimestampOut, model->addColumn("a.timestampOut"), tr("TimeOut"), DateTimeColumnWidth);
		addColumn(ViewCountry, model->addColumn("cnt.name"), tr("Country"), 150);
		addColumn(ViewCity, model->addColumn("city.name"), tr("City"), 150);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), 300);

		_proxyModel = make_shared<AttendanceEntryListProxyModel>(_model);
	}

	AttendanceEntryListProxyModel::AttendanceEntryListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void AttendanceEntryListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::WFlags<Wt::ItemFlag> AttendanceEntryListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::any AttendanceEntryListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any AttendanceEntryListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const AttendanceEntryList::ResultType &res = static_pointer_cast<Dbo::QueryModel<AttendanceEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<AttendanceEntryList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, AttendanceEntry::viewInternalPath(id));
			}
		}
		return Wt::WBatchEditProxyModel::data(idx, role);
	}

}