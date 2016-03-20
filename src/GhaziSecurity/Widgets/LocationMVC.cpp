#include "Widgets/LocationMVC.h"
#include "Widgets/EntityView.h"
#include "Application/WApplication.h"

#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WComboBox>
#include <Wt/WDialog>

namespace GS
{

	const Wt::WFormModel::Field CountryView::codeField = "code";
	const Wt::WFormModel::Field CountryView::nameField = "name";

	CountryView::CountryView(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplateFormView(tr("GS.Admin.CountryView"), parent)
	{
		_model = new Wt::WFormModel(this);
		model()->addField(codeField);
		model()->addField(nameField);

		Wt::WLineEdit *code = new Wt::WLineEdit();
		code->setMaxLength(3);
		setFormWidget(codeField, code);
		model()->setValidator(codeField, new CountryCodeValidator(true));
		code->blurred().connect(std::bind([this]() {
			updateModel(model());
			model()->validateField(codeField);
			updateView(model());
		}));

		Wt::WLineEdit *name = new Wt::WLineEdit();
		name->setMaxLength(70);
		setFormWidget(nameField, name);
		auto nameValidator = new Wt::WLengthValidator(0, 70);
		nameValidator->setMandatory(true);
		model()->setValidator(nameField, nameValidator);

		Wt::WPushButton *submit = new Wt::WPushButton(tr("Submit"));
		submit->clicked().connect(this, &CountryView::submit);
		bindWidget("submit", submit);

		updateView(model());
	}

	void CountryView::submit()
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
			if(!_countryPtr)
				_countryPtr = app->session().add(new Country());

			_countryPtr.modify()->code = model()->valueText(codeField).toUTF8();
			_countryPtr.modify()->name = model()->valueText(nameField).toUTF8();

			if(app->countryQueryModel())
				app->countryQueryModel()->reload();

			t.commit();

			updateView(model());
			submitted().emit();
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "CountryView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	Wt::WValidator::Result CountryCodeValidator::validate(const Wt::WString &input) const
	{
		Result baseResult = Wt::WValidator::validate(input);
		if(baseResult.state() != Valid)
			return baseResult;

		if(input.empty())
			return baseResult;

		WApplication *app = WApplication::instance();
		try
		{
			Wt::Dbo::Transaction t(app->session());
			int rows = app->session().query<int>("SELECT COUNT(1) FROM " + std::string(Country::tableName())).where("code = ?").bind(input);
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

	const Wt::WFormModel::Field CityView::countryField = "country";
	const Wt::WFormModel::Field CityView::nameField = "name";

	CityView::CityView(Wt::WContainerWidget *parent /*= nullptr*/)
		: MyTemplateFormView(tr("GS.Admin.CityView"), parent)
	{
		_model = new Wt::WFormModel(this);
		model()->addField(countryField);
		model()->addField(nameField);

		WApplication *app = WApplication::instance();
		auto country = new ProxyModelComboBox<CountryProxyModel>(app->countryProxyModel());
		setFormWidget(countryField, country);
		auto countryValidator = new ProxyModelCBValidator<CountryProxyModel>(country);
		countryValidator->setErrorString(tr("MustSelectCountry"));
		model()->setValidator(countryField, countryValidator);
		country->blurred().connect(boost::bind(&Wt::WComboBox::validate, country));

		Wt::WLineEdit *name = new Wt::WLineEdit();
		name->setMaxLength(70);
		setFormWidget(nameField, name);
		auto nameValidator = new Wt::WLengthValidator(0, 70);
		nameValidator->setMandatory(true);
		model()->setValidator(nameField, nameValidator);

		Wt::WPushButton *submit = new Wt::WPushButton(tr("Submit"));
		submit->clicked().connect(this, &CityView::submit);
		bindWidget("submit", submit);

		updateView(model());
	}

	void CityView::submit()
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
			if(!_cityPtr)
				_cityPtr = app->session().add(new City());

			_cityPtr.modify()->countryPtr = boost::any_cast<Wt::Dbo::ptr<Country>>(model()->value(countryField));
			_cityPtr.modify()->name = model()->valueText(nameField).toUTF8();

			if(app->cityQueryModel())
				app->cityQueryModel()->reload();

			t.commit();

			updateView(model());
			submitted().emit();
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "CityView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
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
	const Wt::WFormModel::Field LocationFormModel::countryField = "country";
	const Wt::WFormModel::Field LocationFormModel::cityField = "city";
	const Wt::WFormModel::Field LocationFormModel::addressField = "address";

	LocationFormModel::LocationFormModel(LocationView *view, Wt::Dbo::ptr<Location> locationPtr /*= Wt::Dbo::ptr<Location>()*/)
		: Wt::WFormModel(view), _view(view), _locationPtr(locationPtr)
	{
		addField(countryField);
		addField(cityField);
		addField(addressField);

		if(locationPtr)
		{
			setValue(countryField, locationPtr->countryPtr);
			setValue(cityField, locationPtr->cityPtr);
			setValue(addressField, Wt::WString::fromUTF8(locationPtr->address));
		}
	}

	void LocationFormModel::saveChanges(Wt::Dbo::ptr<Location> &inputLocationPtr, Wt::Dbo::ptr<Entity> entityPtr)
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		auto address = valueText(addressField).toUTF8();
		auto countryPtr = boost::any_cast<Wt::Dbo::ptr<Country>>(value(countryField));
		auto cityPtr = boost::any_cast<Wt::Dbo::ptr<City>>(value(cityField));

		if(inputLocationPtr)
			_locationPtr = inputLocationPtr;
		else
		{
			if(!_locationPtr)
			{
				if(address.empty() && !countryPtr && !cityPtr)
					return;

				_locationPtr = app->session().add(new Location()); //TODO: modification restriction instead
			}
		}

		_locationPtr.modify()->address = address;
		_locationPtr.modify()->countryPtr = countryPtr;
		_locationPtr.modify()->cityPtr = cityPtr;
		_locationPtr.modify()->entityPtr = entityPtr;
		t.commit();

		inputLocationPtr = _locationPtr;
	}

	//LocationView
	LocationView::LocationView(Wt::Dbo::ptr<Location> locationPtr, Wt::WContainerWidget *parent)
		: MyTemplateFormView(tr("GS.Admin.LocationView"), parent)
	{
		WApplication *app = WApplication::instance();
		app->initCountryQueryModel();
		app->initCityQueryModel();
		_model = new LocationFormModel(this, locationPtr);
		_cityProxyModel = new CityProxyModel(app->cityQueryModel(), this);

		_countryCombo = new ProxyModelComboBox<CountryProxyModel>(app->countryProxyModel());
		_countryCombo->changed().connect(this, &LocationView::handleCountryChanged);
		setFormWidget(LocationFormModel::countryField, _countryCombo);

		_cityCombo = new ProxyModelComboBox<CityProxyModel>(_cityProxyModel);
		_cityCombo->changed().connect(this, &LocationView::handleCityChanged);
		setFormWidget(LocationFormModel::cityField, _cityCombo);

		_addressEdit = new Wt::WLineEdit();
		setFormWidget(LocationFormModel::addressField, _addressEdit);

		bindEmpty("index");

		updateView(model());
		handleCountryChanged();
	}

	void LocationView::handleCountryChanged()
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
		updateModel(model());
		Wt::WDialog *dialog = new Wt::WDialog(tr("AddCountry"), this);
		dialog->setClosable(true);
		dialog->setTransient(true);
		dialog->rejectWhenEscapePressed(true);
		dialog->setWidth(Wt::WLength(500));
		CountryView *countryView = new CountryView(dialog->contents());

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			delete dialog;
			if(code == Wt::WDialog::Rejected)
			{
				updateModel(model());
				model()->setValue(LocationFormModel::countryField, Wt::Dbo::ptr<Country>());
				updateView(model());
			}
		}, std::placeholders::_1));

		countryView->submitted().connect(std::bind([=]() {
			updateModel(model());
			model()->setValue(LocationFormModel::countryField, countryView->countryPtr());
			updateView(model());
			dialog->accept();
		}));

		dialog->show();
		return dialog;
	}

	Wt::WDialog *LocationView::createAddCityDialog()
	{
		updateModel(model());
		Wt::WDialog *dialog = new Wt::WDialog(tr("AddCity"), this);
		dialog->setClosable(true);
		dialog->setTransient(true);
		dialog->rejectWhenEscapePressed(true);
		dialog->setWidth(Wt::WLength(500));
		CityView *cityView = new CityView(dialog->contents());

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			delete dialog;
			if(code == Wt::WDialog::Rejected)
			{
				updateModel(model());
				model()->setValue(LocationFormModel::cityField, Wt::Dbo::ptr<City>());
				updateView(model());
			}
		}, std::placeholders::_1));

		cityView->model()->setValue(CityView::countryField, model()->value(LocationFormModel::countryField));
		cityView->updateView(cityView->model());
		cityView->submitted().connect(std::bind([=]() {
			updateModel(model());
			model()->setValue(LocationFormModel::countryField, cityView->cityPtr()->countryPtr);
			model()->setValue(LocationFormModel::cityField, cityView->cityPtr());
			updateView(model());
			dialog->accept();
		}));

		dialog->show();
		return dialog;
	}

	LocationsContainer::LocationsContainer(LocationsManagerModel *model, Wt::WContainerWidget *parent)
		: TemplateViewsContainer<LocationView, LocationFormModel>(parent), _model(model)
	{
		for(size_t i = 0; i < _model->ptrVector().size() + 1; ++i)
			addFieldWidget(i < _model->ptrVector().size() ? _model->ptrVector()[i] : Wt::Dbo::ptr<Location>());
	}

	void LocationsContainer::addFieldWidget(Wt::Dbo::ptr<Location> locationPtr /*= Wt::Dbo::ptr<Location>()*/)
	{
		auto w = new LocationView(locationPtr);
		addWidget(w);
		w->bindInt("index", count());
	}

}
