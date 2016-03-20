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

	protected:
		void updateFromCycle(const EntryCycle &cycle);
	};

	//EntryCycleView
	class EntryCycleView : public MyTemplateFormView
	{
	public:
		void initEntryCycleView();
		virtual Wt::WFormModel *model() const = 0;
		virtual void submit() = 0;

	protected:
		EntryCycleView(const Wt::WString &text, Wt::WContainerWidget *parent = nullptr) : MyTemplateFormView(text, parent) { }
		void handleIntervalChanged();
		void updateEndDateValidator(bool update);
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
		virtual Wt::WFormModel *model() const override { return _model; }
		Wt::Dbo::ptr<ExpenseCycle> cyclePtr() const { return _model->cyclePtr(); }

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
		virtual Wt::WFormModel *model() const override { return _model; }
		Wt::Dbo::ptr<IncomeCycle> cyclePtr() const { return _model->cyclePtr(); }

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
		BaseEntryCycleListProxyModel(const std::string &pathPrefix, Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr)
			: Wt::WBatchEditProxyModel(parent), _pathPrefix(pathPrefix)
		{
			setSourceModel(model);
			addAdditionalColumns();
		}
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
		std::string _pathPrefix;
	};

	template<class FilteredList>
	class EntryCycleListProxyModel : public BaseEntryCycleListProxyModel<FilteredList>
	{
	public:
		EntryCycleListProxyModel(const std::string &pathPrefix, Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr) : BaseEntryCycleListProxyModel(pathPrefix, model, parent) { }
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

}

#endif