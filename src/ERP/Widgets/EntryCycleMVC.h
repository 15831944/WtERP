#ifndef GS_ENTRYCYCLE_MVC_H
#define GS_ENTRYCYCLE_MVC_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Utilities/QueryProxyMVC.h"
#include "Utilities/RecordFormView.h"
#include "Utilities/FilteredList.h"

#include <Wt/WTemplateFormView.h>
#include <Wt/WBatchEditProxyModel.h>
#include <Wt/WDateValidator.h>

namespace ERP
{
	class EntryCycleView;
	class ExpenseCycleView;
	class IncomeCycleView;
	class EmployeeAssignmentFormModel;

	class CycleEndDateValidator : public Wt::WDateValidator
	{
	public:
		CycleEndDateValidator(Wt::WFormModel *model) : _model(model) { }
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
		static unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel *model, EntryCycleView *view, Wt::WFormModel::Field field);
		static void updateEndDateValidator(Wt::WFormModel *model, EntryCycleView *view, bool update);
		static void updateModelFromCycle(Wt::WFormModel *model, const EntryCycle &cycle);

		template<class CycleDbo>
		static void updateCycleFromModel(Wt::WFormModel *model, Dbo::ptr<CycleDbo> ptr, bool newCycle);
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
		ExpenseCycleFormModel(ExpenseCycleView *view, Dbo::ptr<ExpenseCycle> cyclePtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

	protected:
		virtual void persistedHandler() override;
		ExpenseCycleView *_view = nullptr;
	};

	//ExpenseCycleView
	class ExpenseCycleView : public EntryCycleView
	{
	public:
		ExpenseCycleView(Dbo::ptr<ExpenseCycle> cyclePtr = nullptr);
		virtual void initView() override;
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return cyclePtr() ? ExpenseCycle::viewInternalPath(cyclePtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<ExpenseCycleView>(); }

		shared_ptr<ExpenseCycleFormModel> model() const { return _model; }
		Dbo::ptr<ExpenseCycle> cyclePtr() const { return _model->recordPtr(); }

	protected:
		shared_ptr<ExpenseCycleFormModel> _model = nullptr;
		Dbo::ptr<ExpenseCycle> _tempPtr;
	};

	//IncomeCycleFormModel
	class IncomeCycleFormModel : public RecordFormModel<IncomeCycle>
	{
	public:
		IncomeCycleFormModel(IncomeCycleView *view, Dbo::ptr<IncomeCycle> cyclePtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

	protected:
		virtual void persistedHandler() override;
		IncomeCycleView *_view = nullptr;
	};

	//IncomeCycleView
	class IncomeCycleView : public EntryCycleView
	{
	public:
		IncomeCycleView(Dbo::ptr<IncomeCycle> cyclePtr = nullptr);
		virtual void initView() override;
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return cyclePtr() ? IncomeCycle::viewInternalPath(cyclePtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<IncomeCycleView>(); }

		shared_ptr<IncomeCycleFormModel> model() const { return _model; }
		Dbo::ptr<IncomeCycle> cyclePtr() const { return _model->recordPtr(); }

	protected:
		shared_ptr<IncomeCycleFormModel> _model = nullptr;
		Dbo::ptr<IncomeCycle> _tempPtr;
	};

	//LISTS
	class EntryCycleList : public QueryModelFilteredList<tuple<long long, Wt::WDateTime, std::string, Wt::WDate, Wt::WDate, long long, long long, CycleInterval, int, long long>>
	{
	public:
		enum ResultColumns { ResId, ResTimestamp, ResEntityName, ResStartDate, ResEndDate, ResAmount, ResExtra, ResInterval, ResNIntervals, ResEntityId };
		enum ViewColumns { ViewId, ViewCreatedOn, ViewEntity, ViewStartDate, ViewEndDate, ViewAmount, ViewExtra };

		EntryCycleList(Dbo::ptr<Entity> entityPtr) : _entityPtr(entityPtr) { }
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		Dbo::ptr<Entity> _entityPtr;
	};

	class IncomeCycleList : public EntryCycleList
	{
	public:
		IncomeCycleList(Dbo::ptr<Entity> entityPtr = nullptr) : EntryCycleList(entityPtr) { }

	protected:
		virtual void initModel() override;
	};

	class ExpenseCycleList : public EntryCycleList
	{
	public:
		ExpenseCycleList(Dbo::ptr<Entity> entityPtr = nullptr) : EntryCycleList(entityPtr) { }

	protected:
		virtual void initModel() override;
	};

	template<class FilteredList>
	class EntryCycleListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		EntryCycleListProxyModel(const std::string &pathPrefix, shared_ptr<Wt::WAbstractItemModel> model)
			: _pathPrefix(pathPrefix)
		{
			setSourceModel(model);
			addAdditionalColumns();
		}
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
		std::string _pathPrefix;
	};
}

#endif