#ifndef GS_HRMVC_WIDGET_H
#define GS_HRMVC_WIDGET_H

#include "Dbo/Dbos.h"
#include "Utilities/RecordFormView.h"

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
		virtual std::string viewInternalPath() const override { return employeeAssignmentPtr() && employeeAssignmentPtr()->entityPtr ? EmployeeAssignment::viewInternalPath(employeeAssignmentPtr()->entityPtr.id()) : ""; }
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

		bool _isEmployeeAssignment;
		Wt::Dbo::ptr<EmployeeAssignment> _tempAssignment;
		Wt::Dbo::ptr<ExpenseCycle> _tempCycle;
		Wt::WContainerWidget *_assignments = nullptr;
		Wt::WContainerWidget *_cycles = nullptr;
		RecordFormView *_mainView = nullptr;
	};

}

#endif