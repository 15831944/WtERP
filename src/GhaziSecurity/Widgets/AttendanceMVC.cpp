#include "Widgets/AttendanceMVC.h"
#include <Wt/WLengthValidator>
#include <Wt/WDateEdit>
#include <Wt/WTimeEdit>
#include <Wt/WTableView>

namespace GS
{

	void AttendanceDeviceList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "d.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1))); filtersTemplate()->addFilter(1);
	}

	void AttendanceDeviceList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT d.id d_id, d.hostName, cnt.name cnt_name, city.name city_name, l.address FROM "
			+ std::string(AttendanceDevice::tableName()) + " d "
			"LEFT JOIN " + Location::tableName() + " l ON (l.id = d.location_id) "
			"LEFT JOIN " + Country::tableName() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tableName() + " city ON (city.id = l.city_id)");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("d.id d_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewHostName, model->addColumn("d.hostName"), tr("Hostname"), 200);
		addColumn(ViewCountry, model->addColumn("cnt.name cnt_name"), tr("Country"), 150);
		addColumn(ViewCity, model->addColumn("city.name city_name"), tr("City"), 150);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), 300);

		_proxyModel = new AttendanceDeviceListProxyModel(_model, _model);
	}

	AttendanceDeviceListProxyModel::AttendanceDeviceListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
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
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	boost::any AttendanceDeviceListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	boost::any AttendanceDeviceListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const AttendanceDeviceList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AttendanceDeviceList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<AttendanceDeviceList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, AttendanceDevice::viewInternalPath(id));
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

	Wt::WWidget *AttendanceDeviceFormModel::createFormWidget(Field field)
	{
		if(field == hostNameField)
		{
			Wt::WLineEdit *hostName = new Wt::WLineEdit();
			hostName->setMaxLength(255);
			Wt::WLengthValidator *validator = new Wt::WLengthValidator(0, 255);
			validator->setMandatory(true);
			setValidator(hostNameField, validator);
			return hostName;
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

	bool AttendanceDeviceFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_recordPtr)
			_recordPtr = app->dboSession().add(new AttendanceDevice());

		_recordPtr.modify()->hostName = valueText(hostNameField).toUTF8();
		_recordPtr.modify()->locationPtr = boost::any_cast<Wt::Dbo::ptr<Location>>(value(locationField));
		return true;
	}

	AttendanceDeviceView::AttendanceDeviceView(Wt::Dbo::ptr<AttendanceDevice> attendanceDevicePtr /*= Wt::Dbo::ptr<AttendanceDevice>()*/)
		: RecordFormView(tr("GS.Admin.AttendanceDeviceView")), _tempPtr(attendanceDevicePtr)
	{ }

	void AttendanceDeviceView::init()
	{
		_model = new AttendanceDeviceFormModel(this, _tempPtr);
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

	Wt::WWidget * AttendanceEntryFormModel::createFormWidget(Field field)
	{
		if(field == entityField)
		{
			FindEntityEdit *findEntityEdit = new FindEntityEdit();
			FindEntityValidator *findEntityValidator = new FindEntityValidator(findEntityEdit, true);
			findEntityValidator->setModifyPermissionRequired(true);
			setValidator(entityField, findEntityValidator);
			return findEntityEdit;
		}
		if(field == locationField)
		{
			FindLocationEdit *findLocationEdit = new FindLocationEdit();
			FindLocationValidator *findLocationValidator = new FindLocationValidator(findLocationEdit, false);
			setValidator(locationField, findLocationValidator);
			return findLocationEdit;
		}
		if(field == dateInField)
		{
			auto edit = new Wt::WDateEdit();
			edit->changed().connect(boost::bind(&AttendanceEntryFormModel::updateTimestampOutValidator, this, true));
			edit->validator()->setMandatory(true);
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == timeInField)
		{
			auto edit = new Wt::WTimeEdit();
			edit->changed().connect(boost::bind(&AttendanceEntryFormModel::updateTimestampOutValidator, this, true));
			edit->validator()->setMandatory(true);
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == dateOutField)
		{
			auto edit = new Wt::WDateEdit();
			setValidator(field, edit->validator());
			return edit;
		}
		if(field == timeOutField)
		{
			auto edit = new Wt::WTimeEdit();
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
			_recordPtr = app->dboSession().add(new AttendanceEntry());

		_recordPtr.modify()->entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->locationPtr = boost::any_cast<Wt::Dbo::ptr<Location>>(value(locationField));
		_recordPtr.modify()->timestampIn = Wt::WDateTime(boost::any_cast<Wt::WDate>(value(dateInField)), boost::any_cast<Wt::WTime>(value(timeInField)));
		Wt::WDateTime out(boost::any_cast<Wt::WDate>(value(dateOutField)), boost::any_cast<Wt::WTime>(value(timeOutField)));
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

		const boost::any &dateInVal = value(AttendanceEntryFormModel::dateInField);
		const boost::any &timeInVal = value(AttendanceEntryFormModel::timeInField);
		if(dateInVal.empty() || timeInVal.empty())
			return;

		Wt::WDateTime timestampIn = Wt::WDateTime(boost::any_cast<Wt::WDate>(value(dateInField)), boost::any_cast<Wt::WTime>(value(timeInField)));
		Wt::WDateValidator *dateOutValidator = dynamic_cast<Wt::WDateValidator*>(validator(AttendanceEntryFormModel::dateInField));
		Wt::WTimeValidator *timeOutValidator = dynamic_cast<Wt::WTimeValidator*>(validator(AttendanceEntryFormModel::timeInField));
		dateOutValidator->setBottom(timestampIn.date());
		timeOutValidator->setBottom(timestampIn.time());
	}

	AttendanceEntryView::AttendanceEntryView(Wt::Dbo::ptr<AttendanceEntry> attendanceEntryPtr /*= Wt::Dbo::ptr<AttendanceEntry>()*/)
		: RecordFormView(tr("GS.Admin.AttendanceEntryView")), _tempPtr(attendanceEntryPtr)
	{ }

	void AttendanceEntryView::init()
	{
		_model = new AttendanceEntryFormModel(this, _tempPtr);
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
				_tableView->sortByColumn(timestampColumn, Wt::DescendingOrder);
		}
	}

	void AttendanceEntryList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "a.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1))); filtersTemplate()->addFilter(1);
	}

	void AttendanceEntryList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT a.id a_id, e.name e_name, a.timestampIn, a.timestampOut, cnt.name cnt_name, city.name city_name, l.address FROM "
			+ std::string(AttendanceEntry::tableName()) + " a "
			"INNER JOIN " + Entity::tableName() + " e ON (e.id = a.entity_id) "
			"LEFT JOIN " + Location::tableName() + " l ON (l.id = a.location_id) "
			"LEFT JOIN " + Country::tableName() + " cnt ON (cnt.code = l.country_code) "
			"LEFT JOIN " + City::tableName() + " city ON (city.id = l.city_id)");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("a.id a_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewEntity, model->addColumn("e.name e_name"), tr("Entity"), 200);
		addColumn(ViewTimestampIn, model->addColumn("a.timestampIn"), tr("TimeIn"), DateTimeColumnWidth);
		addColumn(ViewTimestampOut, model->addColumn("a.timestampOut"), tr("TimeOut"), DateTimeColumnWidth);
		addColumn(ViewCountry, model->addColumn("cnt.name cnt_name"), tr("Country"), 150);
		addColumn(ViewCity, model->addColumn("city.name city_name"), tr("City"), 150);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), 300);

		_proxyModel = new AttendanceEntryListProxyModel(_model, _model);
	}

	AttendanceEntryListProxyModel::AttendanceEntryListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
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
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	boost::any AttendanceEntryListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	boost::any AttendanceEntryListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const AttendanceEntryList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<AttendanceEntryList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<AttendanceEntryList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, AttendanceEntry::viewInternalPath(id));
			}
		}
		return Wt::WBatchEditProxyModel::data(idx, role);
	}

}