#ifndef GS_COUNTRYCITYVIEWS_WIDGET_H
#define GS_COUNTRYCITYVIEWS_WIDGET_H

#include "Dbo/Dbos.h"
#include "Utilities/QueryProxyMVC.h"
#include "Utilities/RecordFormView.h"
#include "Utilities/FilteredList.h"

#include <Wt/Dbo/QueryModel>
#include <Wt/WSortFilterProxyModel>
#include <Wt/WTemplateFormView>
#include <Wt/WLengthValidator>

namespace GS
{
	class LocationView;
	class LocationsManagerModel;
	class FindEntityEdit;
	class CountryView;
	class CityView;

	class CountryProxyModel : public QueryProxyModel<Wt::Dbo::ptr<Country>>
	{
	public:
		CountryProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<Country>> *sourceModel, Wt::WObject *parent = nullptr);

	protected:
		void addAdditionalRows();
	};

	//LocationFilterModel
	class CityFilterModel : public Wt::WSortFilterProxyModel
	{
	public:
		CityFilterModel(Wt::WObject *parent = nullptr);
		virtual bool filterAcceptRow(int sourceRow, const Wt::WModelIndex &sourceParent) const override;

		void setCountryCode(const std::string code);
		std::string countryCode() const { return _countryCode; }

	protected:
		std::string _countryCode;
	};

	class CityProxyModel : public QueryProxyModel<Wt::Dbo::ptr<City>>
	{
	public:
		CityProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<City>> *sourceModel, Wt::WObject *parent = nullptr);
		CityFilterModel *filterModel() const { return _filterModel; }

	protected:
		void addAdditionalRows();
		CityFilterModel *_filterModel = nullptr;
	};

	class CountryFormModel : public RecordFormModel<Country>
	{
	public:
		static const Wt::WFormModel::Field codeField;
		static const Wt::WFormModel::Field nameField;

		CountryFormModel(CountryView *view, Wt::Dbo::ptr<Country> countryPtr = Wt::Dbo::ptr<Country>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		CountryView *_view = nullptr;
	};

	class CountryView : public RecordFormView
	{
	public:
		CountryView(Wt::Dbo::ptr<Country> countryPtr);
		CountryView();
		virtual void init() override;

		Wt::Dbo::ptr<Country> countryPtr() const { return _model->recordPtr(); }
		CountryFormModel *model() const { return _model; }

	protected:
		CountryFormModel *_model = nullptr;
		Wt::Dbo::ptr<Country> _tempPtr;
	};

	class CountryCodeValidator : public Wt::WLengthValidator
	{
	public:
		CountryCodeValidator(bool mandatory = false, const Wt::WString &allowedName = "", Wt::WObject *parent = nullptr)
			: Wt::WLengthValidator(0, 3, parent), _allowedCode(allowedName)
		{
			setMandatory(mandatory);
		}
		virtual Result validate(const Wt::WString &input) const override;
		void setAllowedCode(const Wt::WString &name) { _allowedCode = name; }

	protected:
		Wt::WString _allowedCode;
	};

	class CityFormModel : public RecordFormModel<City>
	{
	public:
		static const Wt::WFormModel::Field countryField;
		static const Wt::WFormModel::Field nameField;

		CityFormModel(CityView *view, Wt::Dbo::ptr<City> cityPtr = Wt::Dbo::ptr<City>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		CityView *_view = nullptr;
	};

	class CityView : public RecordFormView
	{
	public:
		CityView();
		CityView(Wt::Dbo::ptr<City> cityPtr);
		virtual void init() override;

		Wt::Dbo::ptr<City> cityPtr() const { return _model->recordPtr(); }
		CityFormModel *model() const { return _model; }

	protected:
		CityFormModel *_model = nullptr;
		Wt::Dbo::ptr<City> _tempPtr;
	};

	class LocationFormModel : public RecordFormModel<Location>
	{
	public:
		static const Field entityField;
		static const Field countryField;
		static const Field cityField;
		static const Field addressField;

		LocationFormModel(LocationView *view, Wt::Dbo::ptr<Location> locationPtr = Wt::Dbo::ptr<Location>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		LocationView *_view = nullptr;
	};

	class LocationView : public RecordFormView
	{
	public:
		LocationView(Wt::Dbo::ptr<Location> locationPtr = Wt::Dbo::ptr<Location>());
		virtual void init() override;

		void handleCountryChanged();
		void handleCityChanged();
		Wt::WDialog *createAddCountryDialog();
		Wt::WDialog *createAddCityDialog();

		QueryProxyModelCB<CountryProxyModel> *countryCombo() const { return _countryCombo; }
		QueryProxyModelCB<CityProxyModel> *cityCombo() const { return _cityCombo; }
		LocationFormModel *model() const { return _model; }
		Wt::Dbo::ptr<Location> locationPtr() const { return _model->recordPtr(); }
		using RecordFormView::updateView;

	protected:
		virtual void updateView(Wt::WFormModel *model) override;

		QueryProxyModelCB<CountryProxyModel> *_countryCombo = nullptr;
		QueryProxyModelCB<CityProxyModel> *_cityCombo = nullptr;
		CityProxyModel *_cityProxyModel = nullptr;
		LocationFormModel *_model = nullptr;
		Wt::Dbo::ptr<Location> _tempPtr;
	};

	//LocationList
	class LocationList : public QueryModelFilteredList<boost::tuple<long long, std::string, std::string, std::string, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResAddress, ResCountryName, ResCityName, ResEntityName };
		enum ViewColumns { ViewId, ViewAddress, ViewCountryName, ViewCityName, ViewEntityName };
		LocationList();

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

}

#endif