#ifndef GS_HR_MVC_H
#define GS_HR_MVC_H

#include "Dbo/Dbos.h"
#include "Utilities/RecordFormView.h"
#include "Utilities/FilteredList.h"

namespace GS
{
	class ExpenseCycleView;
	class EmployeeAssignmentView;

	class EmployeeAssignmentFormModel : public RecordFormModel<EmployeeAssignment>
	{
	public:
		static const Field descriptionField;
		static const Field startDateField;
		static const Field endDateField;
		static const Field entityField;
		static const Field locationField;
		static const Field expenseCycleField;

		EmployeeAssignmentFormModel(EmployeeAssignmentView *view, Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr = Wt::Dbo::ptr<EmployeeAssignment>());
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

	protected:
		void updateEndDateValidator(bool update);
		EmployeeAssignmentView *_view = nullptr;

	private:
		friend class EmployeeAssignmentView;
	};

	class EmployeeAssignmentView : public RecordFormView
	{
	public:
		EmployeeAssignmentView(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr = Wt::Dbo::ptr<EmployeeAssignment>());
		virtual void init() override;

		using RecordFormView::updateView;
		using RecordFormView::updateModel;

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return employeeAssignmentPtr() && employeeAssignmentPtr()->entityPtr ? EmployeeAssignment::viewInternalPath(employeeAssignmentPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new EmployeeAssignmentView(); }

		Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr() const { return _model->recordPtr(); }
		EmployeeAssignmentFormModel *model() const { return _model; }

	protected:
		virtual void updateView(Wt::WFormModel *model) override;
		virtual void updateModel(Wt::WFormModel *model) override;

		EmployeeAssignmentFormModel *_model = nullptr;
		Wt::Dbo::ptr<EmployeeAssignment> _tempPtr;

	private:
		friend class EmployeeAssignmentFormModel;
	};

	class EmployeeExpenseView : public Wt::WTemplate, public SubmittableRecordWidget
	{
	public:
		EmployeeExpenseView(bool isEmployeeAssignment);
		EmployeeExpenseView(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr);
		EmployeeExpenseView(Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr);
		void init();
		virtual void load() override;

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override;
		virtual SubmittableRecordWidget *createFormView() override { return new EmployeeExpenseView(_isEmployeeAssignment); }

		EmployeeAssignmentView *assignmentView(int index) const;
		ExpenseCycleView *cycleView(int index) const;

	protected:
		void handleMainViewSubmitted();
		EmployeeAssignmentView *addAssignment(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr);
		ExpenseCycleView *addCycle(Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr);
		void handleAddCycle();
		void handleAddAssignment();

		bool canAddCycle() const { return _cycles->count() == 0; }
		bool canAddAssignment() const { return _cycles->count() > 0; }
		bool canSubmitAdditionalViews() const { return _mainView->model()->isRecordPersisted(); }

		bool _isEmployeeAssignment;
		Wt::Dbo::ptr<EmployeeAssignment> _tempAssignment;
		Wt::Dbo::ptr<ExpenseCycle> _tempCycle;
		Wt::WContainerWidget *_assignments = nullptr;
		Wt::WContainerWidget *_cycles = nullptr;
		RecordFormView *_mainView = nullptr;
	};

	class EmployeeAssignmentListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		EmployeeAssignmentListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class EmployeeAssignmentList : public QueryModelFilteredList<boost::tuple<long long, Wt::WDateTime, std::string, Wt::WDate, Wt::WDate, std::string, std::string, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResTimestamp, ResEntityName, ResStartDate, ResEndDate, ResCountryName, ResCityName, ResAddress };
		enum ViewColumns { ViewId, ViewCreatedOn, ViewEntity, ViewStartDate, ViewEndDate, ViewCountry, ViewCity, ViewAddress };
		EmployeeAssignmentList();
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

}

#endif