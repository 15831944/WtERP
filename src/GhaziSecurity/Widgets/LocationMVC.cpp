#include "Widgets/LocationMVC.h"
#include "Widgets/EntityView.h"
#include "Widgets/FindRecordEdit.h"
#include "Application/WApplication.h"

#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/WDialog.h>

namespace GS
{
	using namespace std::placeholders;

	const Wt::WFormModel::Field CountryFormModel::codeField = "code";
	const Wt::WFormModel::Field CountryFormModel::nameField = "name";

	CountryFormModel::CountryFormModel(CountryView *view, Wt::Dbo::ptr<Country> countryPtr)
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

	std::unique_ptr<Wt::WWidget> CountryFormModel::createFormWidget(Field field)
	{
		if(field == codeField)
		{
			auto code = std::make_unique<Wt::WLineEdit>();
			code->setMaxLength(3);
			auto validator = std::make_shared<CountryCodeValidator>(true);
			if(isRecordPersisted())
				validator->setAllowedCode(_recordPtr->code);
			setValidator(codeField, validator);
			code->changed().connect(this, std::bind(&AbstractRecordFormModel::validateUpdateField, this, codeField));
			return code;
		}
		if(field == nameField)
		{
			auto name = std::make_unique<Wt::WLineEdit>();
			name->setMaxLength(70);
			auto nameValidator = std::make_shared<Wt::WLengthValidator>(0, 70);
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
			_recordPtr = app->dboSession().add(std::make_unique<Country>());
			//_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->code = valueText(codeField).toUTF8();
		_recordPtr.modify()->name = valueText(nameField).toUTF8();

		if(app->countryQueryModel())
			app->countryQueryModel()->reload();

		t.commit();

		app->dboSession().flush();
		auto codeValidator = std::static_pointer_cast<CountryCodeValidator>(validator(codeField));
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
		_model = std::make_shared<CountryFormModel>(this, _tempPtr);
		addFormModel("country", _model);
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
			int rows = app->dboSession().query<int>("SELECT COUNT(1) FROM " + std::string(Country::tableName())).where("code = ?").bind(input);
			t.commit();

			if(rows != 0)
				return Result(Wt::ValidationState::Invalid, tr("CountryCodeExists"));
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "CountryCodeValidator::validate(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
			return Result(Wt::ValidationState::Invalid, tr("DatabaseValidationFailed"));
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

	std::unique_ptr<Wt::WWidget> CityFormModel::createFormWidget(Field field)
	{
		if(field == countryField)
		{
			auto country = std::make_unique<QueryProxyModelCB<CountryProxyModel>>(APP->countryProxyModel());
			auto countryValidator = std::make_shared<QueryProxyModelCBValidator<CountryProxyModel>>(country.get());
			countryValidator->setErrorString(tr("MustSelectCountry"));
			setValidator(countryField, countryValidator);
			country->changed().connect(country.get(), std::bind(&Wt::WComboBox::validate, country.get()));
			return country;
		}
		if(field == nameField)
		{
			auto name = std::make_unique<Wt::WLineEdit>();
			name->setMaxLength(70);
			auto nameValidator = std::make_shared<Wt::WLengthValidator>(0, 70);
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
			_recordPtr = app->dboSession().add(std::make_unique<City>());
			//_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->countryPtr = Wt::any_cast<Wt::Dbo::ptr<Country>>(value(countryField));
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
		_model = std::make_shared<CityFormModel>(this, _tempPtr);
		addFormModel("city", _model);
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

	void CityFilterModel::setCountryCode(const std::string code)
	{
		if(code != _countryCode)
		{
			_countryCode = code;
			invalidate();
		}
	}

	CountryProxyModel::CountryProxyModel(std::shared_ptr<QueryModel> sourceModel)
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

	CityProxyModel::CityProxyModel(std::shared_ptr<QueryModel> sourceModel)
	{
		_filterModel = std::make_shared<CityFilterModel>();
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

	std::unique_ptr<Wt::WWidget> LocationFormModel::createFormWidget(Field field)
	{
		if(field == addressField)
		{
			auto addressEdit = std::make_unique<Wt::WLineEdit>();
			return addressEdit;
		}
		if(field == entityField)
		{
			auto w = std::make_unique<FindEntityEdit>();
			auto validator = std::make_shared<FindEntityValidator>(w.get(), false);
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
		auto countryPtr = Wt::any_cast<Wt::Dbo::ptr<Country>>(value(countryField));
		auto cityPtr = Wt::any_cast<Wt::Dbo::ptr<City>>(value(cityField));

		if(!_recordPtr)
		{
			if(address.empty() && !countryPtr && !cityPtr)
				return false;

			_recordPtr = app->dboSession().add(std::make_unique<Location>());
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->entityPtr = Wt::any_cast<Wt::Dbo::ptr<Entity>>(value(entityField));
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
		_model = std::make_shared<LocationFormModel>(this, _tempPtr);
		addFormModel("location", _model);

		WApplication *app = WApplication::instance();
		app->initCountryQueryModel();
		app->initCityQueryModel();

		auto cntCombo = std::make_unique<QueryProxyModelCB<CountryProxyModel>>(app->countryProxyModel());
		_countryCombo = cntCombo.get();
		_countryCombo->changed().connect(this, std::bind(&LocationView::handleCountryChanged, this, true));
		setFormWidget(LocationFormModel::countryField, std::move(cntCombo));

		_cityProxyModel = std::make_shared<CityProxyModel>(app->cityQueryModel());
		auto citCombo = std::make_unique<QueryProxyModelCB<CityProxyModel>>(_cityProxyModel);
		_cityCombo = citCombo.get();
		_cityCombo->changed().connect(this, &LocationView::handleCityChanged);
		setFormWidget(LocationFormModel::cityField, std::move(citCombo));
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

		auto cityProxyModel = std::static_pointer_cast<CityProxyModel>(_cityCombo->model());
		Wt::WString countryCode = Wt::asString(_countryCombo->model()->index(_countryCombo->currentIndex(), 1).data());
		cityProxyModel->filterModel()->setCountryCode(countryCode.toUTF8());

		if(resetCity)
		{
			model()->setValue(LocationFormModel::cityField, Wt::Dbo::ptr<City>());
			updateViewField(model().get(), LocationFormModel::cityField);
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

		_dialog = addChild(std::make_unique<Wt::WDialog>(tr("AddCountry")));
		_dialog->setClosable(true);
		_dialog->setTransient(true);
		_dialog->rejectWhenEscapePressed(true);
		_dialog->setWidth(Wt::WLength(500));
		CountryView *countryView = _dialog->contents()->addNew<CountryView>();

		_dialog->finished().connect(this, std::bind([this](Wt::DialogCode code) {
			if(code == Wt::DialogCode::Rejected)
			{
				model()->setValue(LocationFormModel::countryField, Wt::Dbo::ptr<Country>());
				updateViewField(model().get(), LocationFormModel::countryField);
				handleCountryChanged(true);
			}
			_dialog->removeFromParent();
			_dialog = nullptr;
		}, _1));

		countryView->submitted().connect(this, std::bind([this, countryView]() {
			model()->setValue(LocationFormModel::countryField, countryView->countryPtr());
			updateViewField(model().get(), LocationFormModel::countryField);
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
		_dialog = addChild(std::make_unique<Wt::WDialog>(tr("AddCity")));
		_dialog->setClosable(true);
		_dialog->setTransient(true);
		_dialog->rejectWhenEscapePressed(true);
		_dialog->setWidth(Wt::WLength(500));
		CityView *cityView = _dialog->contents()->addNew<CityView>();

		_dialog->finished().connect(this, std::bind([this](Wt::DialogCode code) {
			if(code == Wt::DialogCode::Rejected)
			{
				model()->setValue(LocationFormModel::cityField, Wt::Dbo::ptr<City>());
				updateViewField(model().get(), LocationFormModel::cityField);
			}
			_dialog->removeFromParent();
			_dialog = nullptr;
		}, _1));

		cityView->load();
		cityView->model()->setValue(CityFormModel::countryField, model()->value(LocationFormModel::countryField));
		cityView->submitted().connect(this, std::bind([this, cityView]() {
			TRANSACTION(APP);
			model()->setValue(LocationFormModel::countryField, cityView->cityPtr()->countryPtr);
			updateViewField(model().get(), LocationFormModel::countryField);
			handleCountryChanged(false);
			model()->setValue(LocationFormModel::cityField, cityView->cityPtr());
			updateViewField(model().get(), LocationFormModel::cityField);
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
		filtersTemplate()->addFilterModel(std::make_shared<WLineEditFilterModel>(tr("ID"), "l.id", std::bind(&FiltersTemplate::initIdEdit, _1)));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Address"), "l.address")); filtersTemplate()->addFilter(2);
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Country"), "cnt.name"));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("City"), "city.name"));
		filtersTemplate()->addFilterModel(std::make_shared<NameFilterModel>(tr("Entity"), "e.name"));
	}

	void LocationList::initModel()
	{
		std::shared_ptr<QueryModelType> model;
		_model = model = std::make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT l.id, l.address, cnt.name, city.name, e.name FROM " + std::string(Location::tableName()) + " l "
			"LEFT JOIN " + Country::tableName() + " cnt ON cnt.code = l.country_code "
			"LEFT JOIN " + City::tableName() + " city ON city.id = l.city_id "
			"LEFT JOIN " + Entity::tableName() + " e ON e.id = l.entity_id ");
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, true, "l.");

		model->setQuery(generateQuery());
		addColumn(ViewId, model->addColumn("l.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewAddress, model->addColumn("l.address"), tr("Address"), AddressColumnWidth);
		addColumn(ViewCountryName, model->addColumn("cnt.name"), tr("Country"), CountryColumnWidth);
		addColumn(ViewCityName, model->addColumn("city.name"), tr("City"), CityColumnWidth);
		addColumn(ViewEntityName, model->addColumn("e.name"), tr("Entity"), EntityColumnWidth);

		//_proxyModel = std::make_shared<LocationListProxyModel>(_model);
	}

}
