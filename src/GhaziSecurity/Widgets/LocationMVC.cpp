#include "Widgets/LocationMVC.h"
#include "Widgets/EntityView.h"
#include "Widgets/FindRecordEdit.h"
#include "Application/WApplication.h"

#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WComboBox>
#include <Wt/WDialog>

namespace GS
{

	const Wt::WFormModel::Field CountryFormModel::codeField = "code";
	const Wt::WFormModel::Field CountryFormModel::nameField = "name";

	CountryFormModel::CountryFormModel(CountryView *view, Wt::Dbo::ptr<Country> countryPtr /*= Wt::Dbo::ptr<Country>()*/)
		: RecordFormModel(view, countryPtr), _view(view)
	{
		addField(codeField);
		addField(nameField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(codeField, Wt::WString::fromUTF8(_recordPtr->code));
			setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));
		}
	}

	Wt::WWidget *CountryFormModel::createFormWidget(Field field)
	{
		if(field == codeField)
		{
			Wt::WLineEdit *code = new Wt::WLineEdit();
			code->setMaxLength(3);
			CountryCodeValidator* validator = new CountryCodeValidator(true);
			if(isRecordPersisted())
				validator->setAllowedCode(_recordPtr->code);
			setValidator(codeField, validator);
			code->changed().connect(boost::bind(&AbstractRecordFormModel::validateUpdateField, this, codeField));
			return code;
		}
		if(field == nameField)
		{
			Wt::WLineEdit *name = new Wt::WLineEdit();
			name->setMaxLength(70);
			auto nameValidator = new Wt::WLengthValidator(0, 70);
			nameValidator->setMandatory(true);
			setValidator(nameField, nameValidator);
			return name;
		}
		return RecordFormModel::createFormWidget(field);
	}

	bool CountryFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().add(new Country());
			//_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->code = valueText(codeField).toUTF8();
		_recordPtr.modify()->name = valueText(nameField).toUTF8();

		if(app->countryQueryModel())
			app->countryQueryModel()->reload();

		t.commit();

		app->dboSession().flush();
		auto codeValidator = dynamic_cast<CountryCodeValidator*>(validator(codeField));
		codeValidator->setAllowedCode(_recordPtr->code);
		return true;
	}

	CountryView::CountryView(Wt::Dbo::ptr<Country> countryPtr)
		: RecordFormView(tr("GS.Admin.CountryView")), _tempPtr(countryPtr)
	{ }

	CountryView::CountryView()
		: RecordFormView(tr("GS.Admin.CountryView"))
	{ }

	void CountryView::initView()
	{
		_model = new CountryFormModel(this, _tempPtr);
		addFormModel("country", _model);
	}

	Wt::WValidator::Result CountryCodeValidator::validate(const Wt::WString &input) const
	{
		Result baseResult = Wt::WValidator::validate(input);
		if(baseResult.state() != Valid)
			return baseResult;

		if(input.empty())
			return baseResult;

		if(!_allowedCode.empty() && input == _allowedCode)
			return baseResult;

		WApplication *app = WApplication::instance();
		try
		{
			TRANSACTION(app);
			int rows = app->dboSession().query<int>("SELECT COUNT(1) FROM " + std::string(Country::tableName())).where("code = ?").bind(input);
			t.commit();

			if(rows != 0)
				return Result(Invalid, Wt::WString::tr("CountryCodeExists"));
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "CountryCodeValidator::validate(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
			return Result(Invalid, Wt::WString::tr("DatabaseValidationFailed"));
		}

		return baseResult;
	}

	const Wt::WFormModel::Field CityFormModel::countryField = "country";
	const Wt::WFormModel::Field CityFormModel::nameField = "name";

	CityFormModel::CityFormModel(CityView *view, Wt::Dbo::ptr<City> cityPtr /*= Wt::Dbo::ptr<City>()*/)
		: RecordFormModel(view, cityPtr), _view(view)
	{
		addField(countryField);
		addField(nameField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(countryField, _recordPtr->countryPtr);
			setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));
		}
	}

	Wt::WWidget *CityFormModel::createFormWidget(Field field)
	{
		if(field == countryField)
		{
			auto country = new QueryProxyModelCB<CountryProxyModel>(APP->countryProxyModel());
			auto countryValidator = new QueryProxyModelCBValidator<CountryProxyModel>(country);
			countryValidator->setErrorString(Wt::WString::tr("MustSelectCountry"));
			setValidator(countryField, countryValidator);
			country->changed().connect(boost::bind(&Wt::WComboBox::validate, country));
			return country;
		}
		if(field == nameField)
		{
			Wt::WLineEdit *name = new Wt::WLineEdit();
			name->setMaxLength(70);
			auto nameValidator = new Wt::WLengthValidator(0, 70);
			nameValidator->setMandatory(true);
			setValidator(nameField, nameValidator);
			return name;
		}
		return RecordFormModel::createFormWidget(field);
	}

	bool CityFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().add(new City());
			//_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->countryPtr = boost::any_cast<Wt::Dbo::ptr<Country>>(value(countryField));
		_recordPtr.modify()->name = valueText(nameField).toUTF8();

		if(app->cityQueryModel())
			app->cityQueryModel()->reload();

		t.commit();
		return true;
	}

	CityView::CityView(Wt::Dbo::ptr<City> cityPtr /*= Wt::Dbo::ptr<City>()*/)
		: RecordFormView(tr("GS.Admin.CityView")), _tempPtr(cityPtr)
	{ }

	CityView::CityView()
		: RecordFormView(tr("GS.Admin.CityView"))
	{ }

	void CityView::initView()
	{
		_model = new CityFormModel(this, _tempPtr);
		addFormModel("city", _model);
	}

	CityFilterModel::CityFilterModel(Wt::WObject *parent)
		: Wt::WSortFilterProxyModel(parent)
	{
		setDynamicSortFilter(true);
	}

	bool CityFilterModel::filterAcceptRow(int sourceRow, const Wt::WModelIndex &sourceParent) const
	{
		if(countryCode().empty())
			return false;

		Wt::WString code = Wt::asString(sourceModel()->index(sourceRow, 2, sourceParent).data(filterRole()));
		return code == countryCode();
	}

	void CityFilterModel::setCountryCode(const std::string code)
	{
		if(code != _countryCode)
		{
			_countryCode = code;
			invalidate();
		}
	}

	CountryProxyModel::CountryProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<Country>> *sourceModel, Wt::WObject *parent /*= nullptr*/)
		: QueryProxyModel<Wt::Dbo::ptr<Country>>(parent)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &CountryProxyModel::addAdditionalRows);
	}

	void CountryProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), Wt::WString::tr("SelectCountry"));
			setData(index(0, 0), false, Wt::AdditionalRowRole);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), Wt::WString::tr("AddCountry"));
			setData(index(lastRow, 0), true, Wt::AdditionalRowRole);
		}
	}

	CityProxyModel::CityProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<City>> *sourceModel, Wt::WObject *parent /*= nullptr*/)
		: QueryProxyModel<Wt::Dbo::ptr<City>>(parent)
	{
		_filterModel = new CityFilterModel(this);
		_filterModel->setSourceModel(sourceModel);

		setSourceModel(_filterModel);
		addAdditionalRows();
		layoutChanged().connect(this, &CityProxyModel::addAdditionalRows);
	}

	void CityProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), Wt::WString::tr("SelectCity"));
			setData(index(0, 0), false, Wt::AdditionalRowRole);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), Wt::WString::tr("AddCity"));
			setData(index(lastRow, 0), true, Wt::AdditionalRowRole);
		}
	}

	//LocationModel
	const Wt::WFormModel::Field LocationFormModel::entityField = "entity";
	const Wt::WFormModel::Field LocationFormModel::countryField = "country";
	const Wt::WFormModel::Field LocationFormModel::cityField = "city";
	const Wt::WFormModel::Field LocationFormModel::addressField = "address";

	LocationFormModel::LocationFormModel(LocationView *view, Wt::Dbo::ptr<Location> locationPtr /*= Wt::Dbo::ptr<Location>()*/)
		: RecordFormModel(view, locationPtr), _view(view)
	{
		addField(entityField);
		addField(countryField);
		addField(cityField);
		addField(addressField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(entityField, _recordPtr->entityPtr);
			setValue(countryField, _recordPtr->countryPtr);
			setValue(cityField, _recordPtr->cityPtr);
			setValue(addressField, Wt::WString::fromUTF8(_recordPtr->address));
		}
	}

	Wt::WWidget *LocationFormModel::createFormWidget(Field field)
	{
		if(field == addressField)
		{
			auto addressEdit = new Wt::WLineEdit();
			return addressEdit;
		}
		if(field == entityField)
		{
			auto w = new FindEntityEdit();
			auto validator = new FindEntityValidator(w, false);
			validator->setModifyPermissionRequired(true);
			setValidator(entityField, validator);
			return w;
		}
		return RecordFormModel::createFormWidget(field);
	}

	bool LocationFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);

		auto address = valueText(addressField).toUTF8();
		auto countryPtr = boost::any_cast<Wt::Dbo::ptr<Country>>(value(countryField));
		auto cityPtr = boost::any_cast<Wt::Dbo::ptr<City>>(value(cityField));

		if(!_recordPtr)
		{
			if(address.empty() && !countryPtr && !cityPtr)
				return false;

			_recordPtr = app->dboSession().add(new Location());
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->entityPtr = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->address = address;
		_recordPtr.modify()->countryPtr = countryPtr;
		_recordPtr.modify()->cityPtr = cityPtr;

		t.commit();
		return true;
	}

	//LocationView
	LocationView::LocationView(Wt::Dbo::ptr<Location> locationPtr)
		: RecordFormView(tr("GS.Admin.LocationView")), _tempPtr(locationPtr)
	{
		bindEmpty("index");
	}

	void LocationView::initView()
	{
		_model = new LocationFormModel(this, _tempPtr);
		addFormModel("location", _model);

		WApplication *app = WApplication::instance();
		app->initCountryQueryModel();
		app->initCityQueryModel();

		_countryCombo = new QueryProxyModelCB<CountryProxyModel>(app->countryProxyModel());
		_countryCombo->changed().connect(boost::bind(&LocationView::handleCountryChanged, this, true));
		setFormWidget(LocationFormModel::countryField, _countryCombo);

		_cityProxyModel = new CityProxyModel(app->cityQueryModel(), this);
		_cityCombo = new QueryProxyModelCB<CityProxyModel>(_cityProxyModel);
		_cityCombo->changed().connect(this, &LocationView::handleCityChanged);
		setFormWidget(LocationFormModel::cityField, _cityCombo);
	}

	void LocationView::updateView(Wt::WFormModel *model)
	{
		RecordFormView::updateView(model);
		handleCountryChanged(false);
	}

	void LocationView::handleCountryChanged(bool resetCity)
{
		boost::any v = _countryCombo->model()->index(_countryCombo->currentIndex(), 0).data(Wt::AdditionalRowRole);
		if(!v.empty() && boost::any_cast<bool>(v) == true)
		{
			createAddCountryDialog();
			return;
		}

		auto cityProxyModel = dynamic_cast<CityProxyModel*>(_cityCombo->model());
		Wt::WString countryCode = Wt::asString(_countryCombo->model()->index(_countryCombo->currentIndex(), 1).data());
		cityProxyModel->filterModel()->setCountryCode(countryCode.toUTF8());

		if(resetCity)
		{
			model()->setValue(LocationFormModel::cityField, Wt::Dbo::ptr<City>());
			updateViewField(model(), LocationFormModel::cityField);
		}
	}

	void LocationView::handleCityChanged()
	{
		boost::any v = _cityCombo->model()->index(_cityCombo->currentIndex(), 0).data(Wt::AdditionalRowRole);
		if(v.empty())
			return;

		if(boost::any_cast<bool>(v) == true)
			createAddCityDialog();
	}

	Wt::WDialog *LocationView::createAddCountryDialog()
	{
		Wt::WDialog *dialog = new Wt::WDialog(tr("AddCountry"), this);
		dialog->setClosable(true);
		dialog->setTransient(true);
		dialog->rejectWhenEscapePressed(true);
		dialog->setDeleteWhenHidden(true);
		dialog->setWidth(Wt::WLength(500));
		CountryView *countryView = new CountryView();
		dialog->contents()->addWidget(countryView);

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			if(code == Wt::WDialog::Rejected)
			{
				model()->setValue(LocationFormModel::countryField, Wt::Dbo::ptr<Country>());
				updateViewField(model(), LocationFormModel::countryField);
				handleCountryChanged(true);
			}
		}, std::placeholders::_1));

		countryView->submitted().connect(std::bind([=]() {
			model()->setValue(LocationFormModel::countryField, countryView->countryPtr());
			updateViewField(model(), LocationFormModel::countryField);
			handleCountryChanged(true);
			dialog->accept();
		}));

		dialog->show();
		return dialog;
	}

	Wt::WDialog *LocationView::createAddCityDialog()
	{
		updateModel();
		Wt::WDialog *dialog = new Wt::WDialog(tr("AddCity"), this);
		dialog->setClosable(true);
		dialog->setTransient(true);
		dialog->rejectWhenEscapePressed(true);
		dialog->setDeleteWhenHidden(true);
		dialog->setWidth(Wt::WLength(500));
		CityView *cityView = new CityView();
		dialog->contents()->addWidget(cityView);

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			if(code == Wt::WDialog::Rejected)
			{
				model()->setValue(LocationFormModel::cityField, Wt::Dbo::ptr<City>());
				updateViewField(model(), LocationFormModel::cityField);
			}
		}, std::placeholders::_1));

		cityView->model()->setValue(CityFormModel::countryField, model()->value(LocationFormModel::countryField));
		cityView->submitted().connect(std::bind([=]() {
			TRANSACTION(APP);
			model()->setValue(LocationFormModel::countryField, cityView->cityPtr()->countryPtr);
			updateViewField(model(), LocationFormModel::countryField);
			handleCountryChanged(false);
			model()->setValue(LocationFormModel::cityField, cityView->cityPtr());
			updateViewField(model(), LocationFormModel::cityField);
			dialog->accept();
		}));

		dialog->show();
		return dialog;
	}

#define AddressColumnWidth 500
#define CountryColumnWidth 200
#define CityColumnWidth 200
#define EntityColumnWidth 250

	LocationList::LocationList()
		: QueryModelFilteredList()
	{ }

	void LocationList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "l.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1)));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Address"), "l.address")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Country"), "cnt.name"));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("City"), "city.name"));
		filtersTemplate()->addFilterModel(new NameFilterModel(tr("Entity"), "e.name"));
	}

	void LocationList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT l.id, l.address, cnt.name cnt_name, city.name city_name, e.name e_name FROM " + std::string(Location::tableName()) + " l "
			"LEFT JOIN " + Country::tableName() + " cnt ON cnt.code = l.country_code "
			"LEFT JOIN " + City::tableName() + " city ON city.id = l.city_id "
			"LEFT JOIN " + Entity::tableName() + " e ON e.id = l.entity_id ");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, true, "l.");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("l.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), AddressColumnWidth);
		addColumn(ViewCountryName, model->addColumn("cnt.name cnt_name"), tr("Country"), CountryColumnWidth);
		addColumn(ViewCityName, model->addColumn("city.name city_name"), tr("City"), CityColumnWidth);
		addColumn(ViewEntityName, model->addColumn("e.name e_name"), tr("Entity"), EntityColumnWidth);

		//_proxyModel = new LocationListProxyModel(_model, _model);
	}

}
