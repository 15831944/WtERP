#ifndef GS_ENTRYCYCLE_MVC_H
#define GS_ENTRYCYCLE_MVC_H

#include "Dbo/Dbos.h"
#include "Utilities/QueryProxyMVC.h"
#include "Utilities/RecordFormView.h"
#include "Utilities/FilteredList.h"

#include <Wt/WTemplateFormView>
#include <Wt/WBatchEditProxyModel>
#include <Wt/WDateValidator>

namespace GS
{
	class EntryCycleView;
	class ExpenseCycleView;
	class IncomeCycleView;
	class EmployeeAssignmentFormModel;

	class CycleEndDateValidator : public Wt::WDateValidator
	{
	public:
		CycleEndDateValidator(Wt::WFormModel *model) : Wt::WDateValidator(model), _model(model) { }
		virtual Result validate(const Wt::WString &input) const override;

	protected:
		Wt::WFormModel *_model = nullptr;
	};

	//EntryCycleFormModel
	class EntryCycleFormModel
	{
	public:
		static const Wt::WFormModel::Field entityField;
		static const Wt::WFormModel::Field startDateField;
		static const Wt::WFormModel::Field endDateField;
		static const Wt::WFormModel::Field intervalField;
		static const Wt::WFormModel::Field nIntervalsField;
		static const Wt::WFormModel::Field amountField;
		static const Wt::WFormModel::Field firstEntryAfterCycleField;

		static void addFields(Wt::WFormModel *model);
		static Wt::WWidget *createFormWidget(Wt::WFormModel *model, EntryCycleView *view, Wt::WFormModel::Field field);
		static void updateEndDateValidator(Wt::WFormModel *model, EntryCycleView *view, bool update);
		static void updateModelFromCycle(Wt::WFormModel *model, const EntryCycle &cycle);

		template<class Dbo>
		static void updateCycleFromModel(Wt::WFormModel *model, Wt::Dbo::ptr<Dbo> ptr, bool newCycle);
	};

	//EntryCycleView
	class EntryCycleView : public RecordFormView
	{
	public:
		using RecordFormView::updateView;
		using RecordFormView::updateModel;

	protected:
		EntryCycleView(const Wt::WString &text) : RecordFormView(text) { }
		virtual void updateView(Wt::WFormModel *model) override;
		virtual void updateModel(Wt::WFormModel *model) override;
		void handleIntervalChanged();

	private:
		friend class EntryCycleFormModel;
	};

	//ExpenseCycleFormModel
	class ExpenseCycleFormModel : public RecordFormModel<ExpenseCycle>
	{
	public:
		ExpenseCycleFormModel(ExpenseCycleView *view, Wt::Dbo::ptr<ExpenseCycle> cyclePtr = Wt::Dbo::ptr<ExpenseCycle>());
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

	protected:
		virtual void persistedHandler() override;
		ExpenseCycleView *_view = nullptr;
	};

	//ExpenseCycleView
	class ExpenseCycleView : public EntryCycleView
	{
	public:
		ExpenseCycleView(Wt::Dbo::ptr<ExpenseCycle> cyclePtr = Wt::Dbo::ptr<ExpenseCycle>());
		virtual void initView() override;
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return cyclePtr() ? ExpenseCycle::viewInternalPath(cyclePtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new ExpenseCycleView(); }

		ExpenseCycleFormModel *model() const { return _model; }
		Wt::Dbo::ptr<ExpenseCycle> cyclePtr() const { return _model->recordPtr(); }

	protected:
		ExpenseCycleFormModel *_model = nullptr;
		Wt::Dbo::ptr<ExpenseCycle> _tempPtr;
	};

	//IncomeCycleFormModel
	class IncomeCycleFormModel : public RecordFormModel<IncomeCycle>
	{
	public:
		IncomeCycleFormModel(IncomeCycleView *view, Wt::Dbo::ptr<IncomeCycle> cyclePtr = Wt::Dbo::ptr<IncomeCycle>());
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

	protected:
		virtual void persistedHandler() override;
		IncomeCycleView *_view = nullptr;
	};

	//IncomeCycleView
	class IncomeCycleView : public EntryCycleView
	{
	public:
		IncomeCycleView(Wt::Dbo::ptr<IncomeCycle> cyclePtr = Wt::Dbo::ptr<IncomeCycle>());
		virtual void initView() override;
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return cyclePtr() ? IncomeCycle::viewInternalPath(cyclePtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new IncomeCycleView(); }

		IncomeCycleFormModel *model() const { return _model; }
		Wt::Dbo::ptr<IncomeCycle> cyclePtr() const { return _model->recordPtr(); }

	protected:
		IncomeCycleFormModel *_model = nullptr;
		Wt::Dbo::ptr<IncomeCycle> _tempPtr;
	};

	//LISTS
	class EntryCycleList : public QueryModelFilteredList<boost::tuple<long long, Wt::WDateTime, std::string, Wt::WDate, Wt::WDate, long long, long long, CycleInterval, int, long long>>
	{
	public:
		enum ResultColumns { ResId, ResTimestamp, ResEntityName, ResStartDate, ResEndDate, ResAmount, ResExtra, ResInterval, ResNIntervals, ResEntityId };
		enum ViewColumns { ViewId, ViewCreatedOn, ViewEntity, ViewStartDate, ViewEndDate, ViewAmount, ViewExtra };

		EntryCycleList(Wt::Dbo::ptr<Entity> entityPtr) : _entityPtr(entityPtr) { }
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		Wt::Dbo::ptr<Entity> _entityPtr;
	};

	class IncomeCycleList : public EntryCycleList
	{
	public:
		IncomeCycleList(Wt::Dbo::ptr<Entity> entityPtr = Wt::Dbo::ptr<Entity>()) : EntryCycleList(entityPtr) { }

	protected:
		virtual void initModel() override;
	};

	class ExpenseCycleList : public EntryCycleList
	{
	public:
		ExpenseCycleList(Wt::Dbo::ptr<Entity> entityPtr = Wt::Dbo::ptr<Entity>()) : EntryCycleList(entityPtr) { }

	protected:
		virtual void initModel() override;
	};

	template<class FilteredList>
	class EntryCycleListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		EntryCycleListProxyModel(const std::string &pathPrefix, Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr)
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

	//TEMPLATE CLASS DEFITIONS

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