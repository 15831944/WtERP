#ifndef GS_LOCATION_MVC_H
#define GS_LOCATION_MVC_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Utilities/QueryProxyMVC.h"
#include "Utilities/RecordFormView.h"
#include "Utilities/FilteredList.h"

#include <Wt/Dbo/QueryModel.h>
#include <Wt/WSortFilterProxyModel.h>
#include <Wt/WTemplateFormView.h>
#include <Wt/WLengthValidator.h>

namespace ERP
{
	class LocationView;
	class LocationsManagerModel;
	class FindEntityEdit;
	class CountryView;
	class CityView;

	class CountryProxyModel : public QueryProxyModel<Dbo::ptr<Country>>
	{
	public:
		typedef Dbo::QueryModel<Dbo::ptr<Country>> QueryModel;
		CountryProxyModel(shared_ptr<QueryModel> sourceModel);

	protected:
		void addAdditionalRows();
	};

	//LocationFilterModel
	class CityFilterModel : public Wt::WSortFilterProxyModel
	{
	public:
		CityFilterModel();
		virtual bool filterAcceptRow(int sourceRow, const Wt::WModelIndex &sourceParent) const override;

		void setCountryCode(const std::string code);
		std::string countryCode() const { return _countryCode; }

	protected:
		std::string _countryCode;
	};

	class CityProxyModel : public QueryProxyModel<Dbo::ptr<City>>
	{
	public:
		typedef Dbo::QueryModel<Dbo::ptr<City>> QueryModel;
		CityProxyModel(shared_ptr<QueryModel> sourceModel);
		shared_ptr<CityFilterModel> filterModel() const { return _filterModel; }

	protected:
		void addAdditionalRows();
		shared_ptr<CityFilterModel> _filterModel = nullptr;
	};

	class CountryFormModel : public RecordFormModel<Country>
	{
	public:
		static const Wt::WFormModel::Field codeField;
		static const Wt::WFormModel::Field nameField;

		CountryFormModel(CountryView *view, Dbo::ptr<Country> countryPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		CountryView *_view = nullptr;
	};

	class CountryView : public RecordFormView
	{
	public:
		CountryView(Dbo::ptr<Country> countryPtr);
		CountryView();
		virtual void initView() override;

		Dbo::ptr<Country> countryPtr() const { return _model->recordPtr(); }
		shared_ptr<CountryFormModel> model() const { return _model; }

	protected:
		shared_ptr<CountryFormModel> _model;
		Dbo::ptr<Country> _tempPtr;
	};

	class CountryCodeValidator : public Wt::WLengthValidator
	{
	public:
		CountryCodeValidator(bool mandatory = false, const Wt::WString &allowedName = "")
			: Wt::WLengthValidator(0, 3), _allowedCode(allowedName)
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

		CityFormModel(CityView *view, Dbo::ptr<City> cityPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		CityView *_view = nullptr;
	};

	class CityView : public RecordFormView
	{
	public:
		CityView();
		CityView(Dbo::ptr<City> cityPtr);
		virtual void initView() override;

		Dbo::ptr<City> cityPtr() const { return _model->recordPtr(); }
		shared_ptr<CityFormModel> model() const { return _model; }

	protected:
		shared_ptr<CityFormModel> _model = nullptr;
		Dbo::ptr<City> _tempPtr;
	};

	class LocationFormModel : public RecordFormModel<Location>
	{
	public:
		static const Field entityField;
		static const Field countryField;
		static const Field cityField;
		static const Field addressField;

		LocationFormModel(LocationView *view, Dbo::ptr<Location> locationPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		LocationView *_view = nullptr;
	};

	class LocationView : public RecordFormView
	{
	public:
		LocationView(Dbo::ptr<Location> locationPtr = nullptr);
		virtual void initView() override;

		void handleCountryChanged(bool resetCity);
		void handleCityChanged();
		void showAddCountryDialog();
		void showAddCityDialog();

		QueryProxyModelCB<CountryProxyModel> *countryCombo() const { return _countryCombo; }
		QueryProxyModelCB<CityProxyModel> *cityCombo() const { return _cityCombo; }
		shared_ptr<LocationFormModel> model() const { return _model; }
		Dbo::ptr<Location> locationPtr() const { return _model->recordPtr(); }
		using RecordFormView::updateView;

	protected:
		virtual void updateView(Wt::WFormModel *model) override;

		QueryProxyModelCB<CountryProxyModel> *_countryCombo = nullptr;
		QueryProxyModelCB<CityProxyModel> *_cityCombo = nullptr;
		Wt::WDialog *_dialog = nullptr;
		shared_ptr<CityProxyModel> _cityProxyModel;
		shared_ptr<LocationFormModel> _model;
		Dbo::ptr<Location> _tempPtr;
	};

	//LocationList
	class LocationList : public QueryModelFilteredList<tuple<long long, std::string, std::string, std::string, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResAddress, ResCountryName, ResCityName, ResEntityName };
		enum ViewColumns { ViewId, ViewAddress, ViewCountryName, ViewCityName, ViewEntityName };

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

}

#endif