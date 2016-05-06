#include "Widgets/AttendanceMVC.h"
#include <Wt/WLengthValidator>

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

}