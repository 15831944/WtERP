#ifndef GS_POSITION_MVC_H
#define GS_POSITION_MVC_H

#include "Dbo/Dbos.h"
#include "Utilities/QueryProxyModel.h"
#include "Utilities/MyFormView.h"
#include "Utilities/TemplateViewsContainer.h"
#include "Utilities/FilteredList.h"

#include <Wt/WTemplateFormView>
#include <Wt/WBatchEditProxyModel>


namespace GS
{
	class ExpenseCycleView;
	class IncomeCycleView;

	//PositionProxyModel
	class PositionProxyModel : public QueryProxyModel<Wt::Dbo::ptr<EmployeePosition>>
	{
	public:
		PositionProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<EmployeePosition>> *sourceModel, Wt::WObject *parent = nullptr);

	protected:
		void addAdditionalRows();
	};

	//PositionView
	class PositionView : public MyTemplateFormView
	{
	public:
		static const Wt::WFormModel::Field titleField;

		PositionView(Wt::WContainerWidget *parent = nullptr);
		Wt::Dbo::ptr<EmployeePosition> positionPtr() const { return _positionPtr; }
		Wt::WFormModel *model() const { return _model; }

	protected:
		void submit();

		Wt::WFormModel *_model = nullptr;
		Wt::Dbo::ptr<EmployeePosition> _positionPtr;
	};

	//ServiceProxyModel
	class ServiceProxyModel : public QueryProxyModel<Wt::Dbo::ptr<ClientService>>
	{
	public:
		ServiceProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<ClientService>> *sourceModel, Wt::WObject *parent = nullptr);

	protected:
		void addAdditionalRows();
	};

	//ServiceView
	class ServiceView : public MyTemplateFormView
	{
	public:
		static const Wt::WFormModel::Field titleField;

		ServiceView(Wt::WContainerWidget *parent = nullptr);
		Wt::Dbo::ptr<ClientService> servicePtr() const { return _servicePtr; }
		Wt::WFormModel *model() const { return _model; }

	protected:
		void submit();

		Wt::WFormModel *_model = nullptr;
		Wt::Dbo::ptr<ClientService> _servicePtr;
	};

	//EntryCycleFormModel
	class EntryCycleFormModel : public Wt::WFormModel
	{
	public:
		static const Field entityField;
		static const Field startDateField;
		static const Field endDateField;
		static const Field intervalField;
		static const Field nIntervalsField;
		static const Field amountField;
		static const Field firstEntryAfterCycleField;

		EntryCycleFormModel(Wt::WObject *parent);
	};

	//EntryCycleView
	class EntryCycleView : public MyTemplateFormView
	{
	public:
		EntryCycleView(const Wt::WString &text, Wt::WContainerWidget *parent = nullptr) : MyTemplateFormView(text, parent) { }
		void initEntryCycleView(Wt::WFormModel *model);
		virtual void submit() = 0;

	protected:
		void handleIntervalChanged();
	};

	//ExpenseCycleFormModel
	class ExpenseCycleFormModel : public EntryCycleFormModel
	{
	public:
		static const Field purposeField;
		static const Field positionField;

		ExpenseCycleFormModel(ExpenseCycleView *view, Wt::Dbo::ptr<ExpenseCycle> cyclePtr = Wt::Dbo::ptr<ExpenseCycle>());
		Wt::Dbo::ptr<ExpenseCycle> cyclePtr() const { return _cyclePtr; }
		void saveChanges();

	protected:
		ExpenseCycleView *_view = nullptr;
		Wt::Dbo::ptr<ExpenseCycle> _cyclePtr;
	};

	//ExpenseCycleView
	class ExpenseCycleView : public EntryCycleView
	{
	public:
		ExpenseCycleView(Wt::Dbo::ptr<ExpenseCycle> cyclePtr = Wt::Dbo::ptr<ExpenseCycle>(), Wt::WContainerWidget *parent = nullptr);

		virtual void submit() override;
		void handlePurposeChanged();
		void handlePositionChanged();
		Wt::WDialog *createAddPositionDialog();

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return cyclePtr() ? ExpenseCycle::viewInternalPath(cyclePtr().id()) : ""; }
		virtual MyTemplateFormView *createFormView() override { return new ExpenseCycleView(); }

		Wt::WComboBox *purposeCombo() const { return _purposeCombo; }
		ProxyModelComboBox<PositionProxyModel> *positionCombo() const { return _positionCombo; }
		ExpenseCycleFormModel *model() const { return _model; }
		Wt::Dbo::ptr<ExpenseCycle> cyclePtr() const { return model()->cyclePtr(); }

	protected:
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;

		Wt::WComboBox *_purposeCombo = nullptr;
		ProxyModelComboBox<PositionProxyModel> *_positionCombo = nullptr;
		ExpenseCycleFormModel *_model = nullptr;
	};

	//IncomeCycleFormModel
	class IncomeCycleFormModel : public EntryCycleFormModel
	{
	public:
		static const Field purposeField;
		static const Field serviceField;

		IncomeCycleFormModel(IncomeCycleView *view, Wt::Dbo::ptr<IncomeCycle> cyclePtr = Wt::Dbo::ptr<IncomeCycle>());
		Wt::Dbo::ptr<IncomeCycle> cyclePtr() const { return _cyclePtr; }
		void saveChanges();

	protected:
		IncomeCycleView *_view = nullptr;
		Wt::Dbo::ptr<IncomeCycle> _cyclePtr;
	};

	//IncomeCycleView
	class IncomeCycleView : public EntryCycleView
	{
	public:
		IncomeCycleView(Wt::Dbo::ptr<IncomeCycle> cyclePtr = Wt::Dbo::ptr<IncomeCycle>(), Wt::WContainerWidget *parent = nullptr);

		virtual void submit() override;
		void handlePurposeChanged();
		void handleServiceChanged();
		Wt::WDialog *createAddServiceDialog();

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return cyclePtr() ? IncomeCycle::viewInternalPath(cyclePtr().id()) : ""; }
		virtual MyTemplateFormView *createFormView() override { return new IncomeCycleView(); }

		Wt::WComboBox *purposeCombo() const { return _purposeCombo; }
		ProxyModelComboBox<ServiceProxyModel> *serviceCombo() const { return _serviceCombo; }
		IncomeCycleFormModel *model() const { return _model; }
		Wt::Dbo::ptr<IncomeCycle> cyclePtr() const { return model()->cyclePtr(); }

	protected:
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;

		Wt::WComboBox *_purposeCombo = nullptr;
		ProxyModelComboBox<ServiceProxyModel> *_serviceCombo = nullptr;
		IncomeCycleFormModel *_model = nullptr;
	};

	//LISTS
	class EntryCycleList : public QueryModelFilteredList<boost::tuple<long long, Wt::WDateTime, std::string, Wt::WDate, Wt::WDate, double, std::string, CycleInterval, int, long long>>
	{
	public:
		enum ResultColumns { ResId, ResCreationTimestamp, ResEntityName, ResStartDate, ResEndDate, ResAmount, ResExtraName, ResInterval, ResNIntervals, ResEntityId };
		enum ViewColumns { ViewId, ViewCreatedOn, ViewEntity, ViewStartDate, ViewEndDate, ViewAmount, ViewExtra };

		EntryCycleList(Wt::WContainerWidget *parent = nullptr);
		void initEntryCycleList();

	protected:
		virtual void initFilters() override;
	};
	class EntityEntryCycleList : public QueryModelFilteredList<boost::tuple<long long, Wt::WDateTime, Wt::WDate, Wt::WDate, double, std::string, CycleInterval, int>>
	{
	public:
		enum ResultColumns { ResId, ResCreationTimestamp, ResStartDate, ResEndDate, ResAmount, ResExtraName, ResInterval, ResNIntervals };

		EntityEntryCycleList(Wt::WContainerWidget *parent = nullptr);
		void initEntityEntryCycleList();

	protected:
		virtual void initFilters() override;
	};

	class IncomeCycleList : public EntryCycleList
	{
	public:
		IncomeCycleList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};
	class EntityIncomeCycleList : public EntityEntryCycleList
	{
	public:
		EntityIncomeCycleList(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
		Wt::Dbo::ptr<Entity> _entityPtr;
	};

	class ExpenseCycleList : public EntryCycleList
	{
	public:
		ExpenseCycleList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};
	class EntityExpenseCycleList : public EntityEntryCycleList
	{
	public:
		EntityExpenseCycleList(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
		Wt::Dbo::ptr<Entity> _entityPtr;
	};

	template<class FilteredList>
	class BaseEntryCycleListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		BaseEntryCycleListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr)
			: Wt::WBatchEditProxyModel(parent)
		{
			setSourceModel(model);
		}
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
	};

	template<class FilteredList>
	class EntryCycleListProxyModel : public BaseEntryCycleListProxyModel<FilteredList>
	{
	public:
		EntryCycleListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr) : BaseEntryCycleListProxyModel(model, parent) { }
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
	};

// 	class IncomeCycleListProxyModel : public EntryCycleListProxyModel
// 	{
// 	public:
// 		IncomeCycleListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr) : EntryCycleListProxyModel(model, parent) { }
// 		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
// 	};
// 	class ExpenseCycleListProxyModel : public EntryCycleListProxyModel
// 	{
// 	public:
// 		ExpenseCycleListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr) : EntryCycleListProxyModel(model, parent) { }
// 		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
// 	};

	//TEMPLATE CLASS DEFINITIONS

	template<class FilteredList>
	boost::any BaseEntryCycleListProxyModel<FilteredList>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		boost::any viewIndexData = headerData(idx.column(), Wt::Horizontal, Wt::ViewIndexRole);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = boost::any_cast<int>(viewIndexData);

		const FilteredList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<FilteredList::ResultType>*>(sourceModel())->resultRow(idx.row());

		if(viewIndex == EntryCycleList::ViewAmount && role == Wt::DisplayRole)
		{
			Wt::WString str;
			switch(boost::get<FilteredList::ResInterval>(res))
			{
			case DailyInterval: str = Wt::WString::trn("RsEveryNDays", boost::get<FilteredList::ResNIntervals>(res)); break;
			case WeeklyInterval: str = Wt::WString::trn("RsEveryNWeeks", boost::get<FilteredList::ResNIntervals>(res)); break;
			case MonthlyInterval: str = Wt::WString::trn("RsEveryNMonths", boost::get<FilteredList::ResNIntervals>(res)); break;
			case YearlyInterval: str = Wt::WString::trn("RsEveryNYears", boost::get<FilteredList::ResNIntervals>(res)); break;
			default: return boost::any();
			}

			str.arg(APP->locale().toFixedString(boost::get<FilteredList::ResAmount>(res), 2));
			str.arg(boost::get<FilteredList::ResNIntervals>(res));
			return str;
		}

		if(viewIndex == EntryCycleList::ViewStartDate && role == Wt::DisplayRole)
		{
			const Wt::WDate &date = boost::get<FilteredList::ResStartDate>(res);
			if(date.isValid() && date > Wt::WDate(boost::gregorian::day_clock::local_day()))
				return Wt::WString::tr("XNotStarted").arg(boost::get<FilteredList::ResStartDate>(res).toString(APP->locale().dateFormat()));
		}

		if(viewIndex == EntryCycleList::ViewEndDate && role == Wt::DisplayRole)
		{
			const Wt::WDate &date = boost::get<FilteredList::ResEndDate>(res);
			if(date.isValid() && Wt::WDate(boost::gregorian::day_clock::local_day()) >= date)
				return Wt::WString::tr("XEnded").arg(date.toString(APP->locale().dateFormat()));
		}

		if(role == Wt::StyleClassRole)
		{
			const Wt::WDate &startDate = boost::get<FilteredList::ResStartDate>(res);
			if(startDate.isValid() && startDate > Wt::WDate(boost::gregorian::day_clock::local_day()))
				return "text-info";

			const Wt::WDate &endDate = boost::get<FilteredList::ResEndDate>(res);
			if(endDate.isValid() && Wt::WDate(boost::gregorian::day_clock::local_day()) >= endDate)
				return "text-muted";
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	template<class FilteredList>
	boost::any EntryCycleListProxyModel<FilteredList>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		boost::any viewIndexData = headerData(idx.column(), Wt::Horizontal, Wt::ViewIndexRole);
		if(viewIndexData.empty())
			return BaseEntryCycleListProxyModel<FilteredList>::data(idx, role);
		int viewIndex = boost::any_cast<int>(viewIndexData);

		if(viewIndex == EntryCycleList::ViewEntity && role == Wt::LinkRole)
		{
			const FilteredList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<FilteredList::ResultType>*>(sourceModel())->resultRow(idx.row());
			return Wt::WLink(Wt::WLink::InternalPath, Entity::viewInternalPath(boost::get<FilteredList::ResEntityId>(res)));
		}

		return BaseEntryCycleListProxyModel<FilteredList>::data(idx, role);
	}

}

#endif