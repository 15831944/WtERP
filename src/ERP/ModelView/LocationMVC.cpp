#include "ModelView/LocationMVC.h"
#include "ModelView/EntityView.h"
#include "ModelView/FindRecordEdit.h"

namespace ERP
{
	const Wt::WFormModel::Field CountryFormModel::codeField = "code";
	const Wt::WFormModel::Field CountryFormModel::nameField = "name";

	CountryFormModel::CountryFormModel(CountryView *view, Dbo::ptr<Country> countryPtr)
		: RecordFormModel(view, move(countryPtr)), _view(view)
	{
		addField(codeField);
		addField(nameField);
	}

	void CountryFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(codeField, Wt::WString::fromUTF8(_recordPtr->code));
		setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));
	}

	unique_ptr<Wt::WWidget> CountryFormModel::createFormWidget(Field field)
	{
		if(field == codeField)
		{
			auto code = make_unique<Wt::WLineEdit>();
			code->setMaxLength(3);
			auto validator = make_shared<CountryCodeValidator>(true);
			if(isRecordPersisted())
				validator->setAllowedCode(_recordPtr->code);
			setValidator(codeField, validator);
			code->changed().connect(this, std::bind(&AbstractRecordFormModel::validateUpdateField, this, codeField));
			return code;
		}
		if(field == nameField)
		{
			auto name = make_unique<Wt::WLineEdit>();
			name->setMaxLength(70);
			auto nameValidator = make_shared<Wt::WLengthValidator>(0, 70);
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
			_recordPtr = app->dboSession().addNew<Country>();
			//_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->code = valueText(codeField).toUTF8();
		_recordPtr.modify()->name = valueText(nameField).toUTF8();

		if(app->countryQueryModel())
			app->countryQueryModel()->reload();

		t.commit();

		app->dboSession().flush();
		auto codeValidator = static_pointer_cast<CountryCodeValidator>(validator(codeField));
		codeValidator->setAllowedCode(_recordPtr->code);
		return true;
	}

	CountryView::CountryView(Dbo::ptr<Country> countryPtr)
		: RecordFormView(tr("ERP.Admin.CountryView"))
	{
		_model = newFormModel<CountryFormModel>("country", this, move(countryPtr));
	}

	Wt::WValidator::Result CountryCodeValidator::validate(const Wt::WString &input) const
	{
		Result baseResult = Wt::WValidator::validate(input);
		if(baseResult.state() != Wt::ValidationState::Valid)
			return baseResult;

		if(input.empty())
			return baseResult;

		if(!_allowedCode.empty() && input == _allowedCode)
			return baseResult;

		WApplication *app = WApplication::instance();
		try
		{
			TRANSACTION(app);
			int rows = app->dboSession().query<int>("SELECT COUNT(1) FROM " + Country::tStr()).where("code = ?").bind(input);
			t.commit();

			if(rows != 0)
				return Result(Wt::ValidationState::Invalid, tr("CountryCodeExists"));
		}
		catch(Dbo::Exception &e)
		{
			Wt::log("error") << "CountryCodeValidator::validate(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
			return Result(Wt::ValidationState::Invalid, tr("DatabaseValidationFailed"));
		}

		return baseResult;
	}

	const Wt::WFormModel::Field CityFormModel::countryField = "country";
	const Wt::WFormModel::Field CityFormModel::nameField = "name";

	CityFormModel::CityFormModel(CityView *view, Dbo::ptr<City> cityPtr)
		: RecordFormModel(view, move(cityPtr)), _view(view)
	{
		addField(countryField);
		addField(nameField);
	}

	void CityFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(countryField, _recordPtr->countryPtr);
		setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));
	}

	unique_ptr<Wt::WWidget> CityFormModel::createFormWidget(Field field)
	{
		if(field == countryField)
		{
			auto country = make_unique<QueryProxyModelCB<CountryProxyModel>>(APP->countryProxyModel());
			auto countryValidator = make_shared<QueryProxyModelCBValidator<CountryProxyModel>>(country.get());
			countryValidator->setErrorString(tr("MustSelectCountry"));
			setValidator(countryField, countryValidator);
			country->changed().connect(country.get(), std::bind(&Wt::WComboBox::validate, country.get()));
			return country;
		}
		if(field == nameField)
		{
			auto name = make_unique<Wt::WLineEdit>();
			name->setMaxLength(70);
			auto nameValidator = make_shared<Wt::WLengthValidator>(0, 70);
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
			_recordPtr = app->dboSession().addNew<City>();
			//_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->countryPtr = Wt::any_cast<Dbo::ptr<Country>>(value(countryField));
		_recordPtr.modify()->name = valueText(nameField).toUTF8();

		if(app->cityQueryModel())
			app->cityQueryModel()->reload();

		t.commit();
		return true;
	}

	CityView::CityView(Dbo::ptr<City> cityPtr)
		: RecordFormView(tr("ERP.Admin.CityView"))
	{
		_model = newFormModel<CityFormModel>("city", this, move(cityPtr));
	}

	CityFilterModel::CityFilterModel()
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

	void CityFilterModel::setCountryCode(std::string code)
	{
		if(code != _countryCode)
		{
			_countryCode = move(code);
			invalidate();
		}
	}

	CountryProxyModel::CountryProxyModel(shared_ptr<QueryModel> sourceModel)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &CountryProxyModel::addAdditionalRows);
	}

	void CountryProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), tr("SelectCountry"));
			setData(index(0, 0), false, Wt::ItemDataRole::AdditionalRow);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), tr("AddCountry"));
			setData(index(lastRow, 0), true, Wt::ItemDataRole::AdditionalRow);
		}
	}

	CityProxyModel::CityProxyModel(shared_ptr<QueryModel> sourceModel)
	{
		_filterModel = make_shared<CityFilterModel>();
		_filterModel->setSourceModel(sourceModel);

		setSourceModel(_filterModel);
		addAdditionalRows();
		layoutChanged().connect(this, &CityProxyModel::addAdditionalRows);
	}

	void CityProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), tr("SelectCity"));
			setData(index(0, 0), false, Wt::ItemDataRole::AdditionalRow);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), tr("AddCity"));
			setData(index(lastRow, 0), true, Wt::ItemDataRole::AdditionalRow);
		}
	}

	//LocationModel
	const Wt::WFormModel::Field LocationFormModel::entityField = "entity";
	const Wt::WFormModel::Field LocationFormModel::countryField = "country";
	const Wt::WFormModel::Field LocationFormModel::cityField = "city";
	const Wt::WFormModel::Field LocationFormModel::addressField = "address";

	LocationFormModel::LocationFormModel(LocationView *view, Dbo::ptr<Location> locationPtr)
		: RecordFormModel(view, move(locationPtr)), _view(view)
	{
		addField(entityField);
		addField(countryField);
		addField(cityField);
		addField(addressField);
	}

	void LocationFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(entityField, _recordPtr->entityPtr);
		setValue(countryField, _recordPtr->countryPtr);
		setValue(cityField, _recordPtr->cityPtr);
		setValue(addressField, Wt::WString::fromUTF8(_recordPtr->address));
	}

	unique_ptr<Wt::WWidget> LocationFormModel::createFormWidget(Field field)
	{
		if(field == addressField)
		{
			auto addressEdit = make_unique<Wt::WLineEdit>();
			return addressEdit;
		}
		if(field == entityField)
		{
			auto w = make_unique<FindEntityEdit>();
			auto validator = make_shared<FindEntityValidator>(w.get(), false);
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
		auto countryPtr = Wt::any_cast<Dbo::ptr<Country>>(value(countryField));
		auto cityPtr = Wt::any_cast<Dbo::ptr<City>>(value(cityField));

		if(!_recordPtr)
		{
			if(address.empty() && !countryPtr && !cityPtr)
				return false;
			_recordPtr = app->dboSession().addNew<Location>();
		}

		_recordPtr.modify()->entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->address = address;
		_recordPtr.modify()->countryPtr = countryPtr;
		_recordPtr.modify()->cityPtr = cityPtr;

		t.commit();
		return true;
	}

	//LocationView
	LocationView::LocationView(Dbo::ptr<Location> locationPtr)
		: RecordFormView(tr("ERP.Admin.LocationView"))
	{
		_model = newFormModel<LocationFormModel>("location", this, move(locationPtr));
	}

	void LocationView::initView()
	{
		bindEmpty("index");

		WApplication *app = APP;
		app->initCountryQueryModel();
		app->initCityQueryModel();

		auto cntCombo = make_unique<QueryProxyModelCB<CountryProxyModel>>(app->countryProxyModel());
		_countryCombo = cntCombo.get();
		_countryCombo->changed().connect(this, std::bind(&LocationView::handleCountryChanged, this, true));
		setFormWidget(LocationFormModel::countryField, move(cntCombo));

		_cityProxyModel = make_shared<CityProxyModel>(app->cityQueryModel());
		auto citCombo = make_unique<QueryProxyModelCB<CityProxyModel>>(_cityProxyModel);
		_cityCombo = citCombo.get();
		_cityCombo->changed().connect(this, &LocationView::handleCityChanged);
		setFormWidget(LocationFormModel::cityField, move(citCombo));
	}

	void LocationView::updateView(Wt::WFormModel *model)
	{
		RecordFormView::updateView(model);
		handleCountryChanged(false);
	}

	void LocationView::handleCountryChanged(bool resetCity)
	{
		Wt::any v = _countryCombo->model()->index(_countryCombo->currentIndex(), 0).data(Wt::ItemDataRole::AdditionalRow);
		if(!v.empty() && Wt::any_cast<bool>(v) == true)
		{
			showAddCountryDialog();
			return;
		}

		auto cityProxyModel = static_pointer_cast<CityProxyModel>(_cityCombo->model());
		Wt::WString countryCode = Wt::asString(_countryCombo->model()->index(_countryCombo->currentIndex(), 1).data());
		cityProxyModel->filterModel()->setCountryCode(countryCode.toUTF8());

		if(resetCity)
		{
			model()->setValue(LocationFormModel::cityField, Dbo::ptr<City>());
			updateViewField(model(), LocationFormModel::cityField);
		}
	}

	void LocationView::handleCityChanged()
	{
		Wt::any v = _cityCombo->model()->index(_cityCombo->currentIndex(), 0).data(Wt::ItemDataRole::AdditionalRow);
		if(v.empty())
			return;

		if(Wt::any_cast<bool>(v) == true)
			showAddCountryDialog();
	}

	void LocationView::showAddCountryDialog()
	{
		if(_dialog)
			return;

		_dialog = addChild(make_unique<Wt::WDialog>(tr("AddCountry")));
		_dialog->setClosable(true);
		_dialog->setTransient(true);
		_dialog->rejectWhenEscapePressed(true);
		_dialog->setWidth(Wt::WLength(500));
		auto *countryView = _dialog->contents()->addNew<CountryView>();

		_dialog->finished().connect(this, std::bind([this](Wt::DialogCode code) {
			if(code == Wt::DialogCode::Rejected)
			{
				model()->setValue(LocationFormModel::countryField, Dbo::ptr<Country>());
				updateViewField(model(), LocationFormModel::countryField);
				handleCountryChanged(true);
			}
			_dialog->removeFromParent();
			_dialog = nullptr;
		}, _1));

		countryView->submitted().connect(this, std::bind([this, countryView]() {
			model()->setValue(LocationFormModel::countryField, countryView->countryPtr());
			updateViewField(model(), LocationFormModel::countryField);
			handleCountryChanged(true);
			_dialog->accept();
		}));

		_dialog->show();
	}

	void LocationView::showAddCityDialog()
	{
		if(_dialog)
			return;

		updateModel();
		_dialog = addChild(make_unique<Wt::WDialog>(tr("AddCity")));
		_dialog->setClosable(true);
		_dialog->setTransient(true);
		_dialog->rejectWhenEscapePressed(true);
		_dialog->setWidth(Wt::WLength(500));
		auto *cityView = _dialog->contents()->addNew<CityView>();

		_dialog->finished().connect(this, std::bind([this](Wt::DialogCode code) {
			if(code == Wt::DialogCode::Rejected)
			{
				model()->setValue(LocationFormModel::cityField, Dbo::ptr<City>());
				updateViewField(model(), LocationFormModel::cityField);
			}
			_dialog->removeFromParent();
			_dialog = nullptr;
		}, _1));

		cityView->load();
		cityView->model()->setValue(CityFormModel::countryField, model()->value(LocationFormModel::countryField));
		cityView->submitted().connect(this, std::bind([this, cityView]() {
			TRANSACTION(APP);
			model()->setValue(LocationFormModel::countryField, cityView->cityPtr()->countryPtr);
			updateViewField(model(), LocationFormModel::countryField);
			handleCountryChanged(false);
			model()->setValue(LocationFormModel::cityField, cityView->cityPtr());
			updateViewField(model(), LocationFormModel::cityField);
			_dialog->accept();
		}));

		_dialog->show();
	}

#define AddressColumnWidth 500
#define CountryColumnWidth 200
#define CityColumnWidth 200
#define EntityColumnWidth 250

	void LocationList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "l.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Address"), "l.address")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Country"), "cnt.name"));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("City"), "city.name"));
		filtersTemplate()->addFilterModel(make_shared<NameFilterModel>(tr("Entity"), "e.name"));
	}

	void LocationList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT l.id, l.address, cnt.name, city.name, e.name FROM " + Location::tStr() + " l "
			"LEFT JOIN " + Country::tStr() + " cnt ON cnt.code = l.country_code "
			"LEFT JOIN " + City::tStr() + " city ON city.id = l.city_id "
			"LEFT JOIN " + Entity::tStr() + " e ON e.id = l.entity_id ");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("l.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), AddressColumnWidth);
		addColumn(ViewCountryName, model->addColumn("cnt.name"), tr("Country"), CountryColumnWidth);
		addColumn(ViewCityName, model->addColumn("city.name"), tr("City"), CityColumnWidth);
		addColumn(ViewEntityName, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);

		//_proxyModel = make_shared<LocationListProxyModel>(_model);
	}

}
