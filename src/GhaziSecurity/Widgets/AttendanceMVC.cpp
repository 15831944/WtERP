#include "Widgets/AttendanceMVC.h"
#include <Wt/WLengthValidator.h>
#include <Wt/WDateEdit.h>
#include <Wt/WTimeEdit.h>
#include <Wt/WTableView.h>

namespace GS
{
	using namespace std::placeholders;

	void AttendanceDeviceList::initFilters()
	{
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "d.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilter(1);
	}

	void AttendanceDeviceList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT d.id, d.hostName, cnt.name, city.name, l.address FROM "
			+ std::string(AttendanceDevice::tableName()) + " d "
			"LEFT JOIN " + Location::tableName() + " l ON (l.id = d.location_id) "
			"LEFT JOIN " + Country::tableName() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tableName() + " city ON (city.id = l.city_id)");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("d.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewHostName, model->addColumn("d.hostName"), tr("Hostname"), 200);
		addColumn(ViewCountry, model->addColumn("cnt.name"), tr("Country"), 150);
		addColumn(ViewCity, model->addColumn("city.name"), tr("City"), 150);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), 300);

		_proxyModel = std::make_shared<AttendanceDeviceListProxyModel>(_model);
	}

	AttendanceDeviceListProxyModel::AttendanceDeviceListProxyModel(std::shared_ptr<Wt::WAbstractItemModel> model)
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
				return tr("GS.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const AttendanceDeviceList::ResultType &res = std::static_pointer_cast<Wt::Dbo::QueryModel<AttendanceDeviceList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<AttendanceDeviceList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, AttendanceDevice::viewInternalPath(id));
			}
		}
		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	const Wt::WFormModel::Field AttendanceDeviceFormModel::hostNameField = "hostName";
	const Wt::WFormModel::Field AttendanceDeviceFormModel::locationField = "location";

	AttendanceDeviceFormModel::AttendanceDeviceFormModel(AttendanceDeviceView *view, Wt::Dbo::ptr<AttendanceDevice> attendanceDevicePtr /*= Wt::Dbo::ptr<AttendanceDevice>()*/)
		: RecordFormModel(view, attendanceDevicePtr), _view(view)
	{
		addField(hostNameField);
		addField(locationField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(hostNameField, Wt::WString::fromUTF8(_recordPtr->hostName));
			setValue(locationField, _recordPtr->locationPtr);
		}
	}

	std::unique_ptr<Wt::WWidget> AttendanceDeviceFormModel::createFormWidget(Field field)
	{
		if(field == hostNameField)
		{
			auto hostName = std::make_unique<Wt::WLineEdit>();
			hostName->setMaxLength(255);
			auto validator = std::make_shared<Wt::WLengthValidator>(0, 255);
			validator->setMandatory(true);
			setValidator(hostNameField, validator);
			return hostName;
		}
		if(field == locationField)
		{
			auto findLocationEdit = std::make_unique<FindLocationEdit>();
			auto findLocationValidator = std::make_shared<FindLocationValidator>(findLocationEdit.get(), true);
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
			_recordPtr = app->dboSession().add(std::make_unique<AttendanceDevice>());

		_recordPtr.modify()->hostName = valueText(hostNameField).toUTF8();
		_recordPtr.modify()->locationPtr = Wt::any_cast<Wt::Dbo::ptr<Location>>(value(locationField));
		return true;
	}

	AttendanceDeviceView::AttendanceDeviceView(Wt::Dbo::ptr<AttendanceDevice> attendanceDevicePtr /*= Wt::Dbo::ptr<AttendanceDevice>()*/)
		: RecordFormView(tr("GS.Admin.AttendanceDeviceView")), _tempPtr(attendanceDevicePtr)
	{ }

	void AttendanceDeviceView::initView()
	{
		_model = std::make_shared<AttendanceDeviceFormModel>(this, _tempPtr);
		addFormModel("attendance-device", _model);
	}

	const Wt::WFormModel::Field AttendanceEntryFormModel::entityField = "entity";
	const Wt::WFormModel::Field AttendanceEntryFormModel::dateInField = "dateIn";
	const Wt::WFormModel::Field AttendanceEntryFormModel::timeInField = "timeIn";
	const Wt::WFormModel::Field AttendanceEntryFormModel::dateOutField = "dateOut";
	const Wt::WFormModel::Field AttendanceEntryFormModel::timeOutField = "timeOut";
	const Wt::WFormModel::Field AttendanceEntryFormModel::locationField = "location";

	AttendanceEntryFormModel::AttendanceEntryFormModel(AttendanceEntryView *view, Wt::Dbo::ptr<AttendanceEntry> attendanceEntryPtr /*= Wt::Dbo::ptr<AttendanceEntry>()*/)
		: RecordFormModel(view, attendanceEntryPtr), _view(view)
	{
		addField(entityField);
		addField(dateInField);
		addField(timeInField);
		addField(dateOutField);
		addField(timeOutField);
		addField(locationField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(entityField, _recordPtr->entityPtr);
			setValue(dateInField, _recordPtr->timestampIn.date());
			setValue(timeInField, _recordPtr->timestampIn.time());
			setValue(dateOutField, _recordPtr->timestampOut.date());
			setValue(timeOutField, _recordPtr->timestampOut.time());
			setValue(locationField, _recordPtr->locationPtr);
		}
	}

	std::unique_ptr<Wt::WWidget> AttendanceEntryFormModel::createFormWidget(Field field)
	{
		if(field == entityField)
		{
			auto findEntityEdit = std::make_unique<FindEntityEdit>();
			auto findEntityValidator = std::make_shared<FindEntityValidator>(findEntityEdit.get(), true);
			findEntityValidator->setModifyPermissionRequired(true);
			setValidator(entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == locationField)
		{
			auto findLocationEdit = std::make_unique<FindLocationEdit>();
			auto findLocationValidator = std::make_shared<FindLocationValidator>(findLocationEdit.get(), false);
			setValidator(locationField, findLocationValidator);
			return findLocationEdit;
		}
		if(field == dateInField)
		{
			auto edit = std::make_unique<Wt::WDateEdit>();
			edit->changed().connect(this, std::bind(&AttendanceEntryFormModel::updateTimestampOutValidator, this, true));
			edit->validator()->setMandatory(true);
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == timeInField)
		{
			auto edit = std::make_unique<Wt::WTimeEdit>();
			edit->changed().connect(this, std::bind(&AttendanceEntryFormModel::updateTimestampOutValidator, this, true));
			edit->validator()->setMandatory(true);
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == dateOutField)
		{
			auto edit = std::make_unique<Wt::WDateEdit>();
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == timeOutField)
		{
			auto edit = std::make_unique<Wt::WTimeEdit>();
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
			_recordPtr = app->dboSession().add(std::make_unique<AttendanceEntry>());

		_recordPtr.modify()->entityPtr = Wt::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->locationPtr = Wt::any_cast<Wt::Dbo::ptr<Location>>(value(locationField));
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
		std::shared_ptr<Wt::WDateValidator> dateOutValidator = std::dynamic_pointer_cast<Wt::WDateValidator>(validator(AttendanceEntryFormModel::dateInField));
		std::shared_ptr<Wt::WTimeValidator> timeOutValidator = std::dynamic_pointer_cast<Wt::WTimeValidator>(validator(AttendanceEntryFormModel::timeInField));
		dateOutValidator->setBottom(timestampIn.date());
		timeOutValidator->setBottom(timestampIn.time());
	}

	AttendanceEntryView::AttendanceEntryView(Wt::Dbo::ptr<AttendanceEntry> attendanceEntryPtr)
		: RecordFormView(tr("GS.Admin.AttendanceEntryView")), _tempPtr(attendanceEntryPtr)
	{ }

	void AttendanceEntryView::initView()
	{
		_model = std::make_shared<AttendanceEntryFormModel>(this, _tempPtr);
		addFormModel("attendance-entry", _model);
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
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "a.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilter(1);
	}

	void AttendanceEntryList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT a.id, e.name, a.timestampIn, a.timestampOut, cnt.name, city.name, l.address FROM "
			+ std::string(AttendanceEntry::tableName()) + " a "
			"INNER JOIN " + Entity::tableName() + " e ON (e.id = a.entity_id) "
			"LEFT JOIN " + Location::tableName() + " l ON (l.id = a.location_id) "
			"LEFT JOIN " + Country::tableName() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tableName() + " city ON (city.id = l.city_id)");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("a.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.name"), tr("Entity"), 200);
		addColumn(ViewTimestampIn, model->addColumn("a.timestampIn"), tr("TimeIn"), DateTimeColumnWidth);
		addColumn(ViewTimestampOut, model->addColumn("a.timestampOut"), tr("TimeOut"), DateTimeColumnWidth);
		addColumn(ViewCountry, model->addColumn("cnt.name"), tr("Country"), 150);
		addColumn(ViewCity, model->addColumn("city.name"), tr("City"), 150);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), 300);

		_proxyModel = std::make_shared<AttendanceEntryListProxyModel>(_model);
	}

	AttendanceEntryListProxyModel::AttendanceEntryListProxyModel(std::shared_ptr<Wt::WAbstractItemModel> model)
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
				return tr("GS.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const AttendanceEntryList::ResultType &res = std::static_pointer_cast<Wt::Dbo::QueryModel<AttendanceEntryList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<AttendanceEntryList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, AttendanceEntry::viewInternalPath(id));
			}
		}
		return Wt::WBatchEditProxyModel::data(idx, role);
	}

}