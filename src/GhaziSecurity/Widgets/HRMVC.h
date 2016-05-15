#ifndef GS_HR_MVC_H
#define GS_HR_MVC_H

#include "Dbo/Dbos.h"
#include "Utilities/RecordFormView.h"
#include "Utilities/FilteredList.h"

namespace GS
{
	class ExpenseCycleView;
	class IncomeCycleView;
	class EmployeeAssignmentView;
	class ClientAssignmentView;
	class PositionView;
	class ServiceView;
	class EntryCycleView;

	//Position/Service....
	//PositionProxyModel
	class PositionProxyModel : public QueryProxyModel<Wt::Dbo::ptr<EmployeePosition>>
	{
	public:
		PositionProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<EmployeePosition>> *sourceModel, Wt::WObject *parent = nullptr);

	protected:
		void addAdditionalRows();
	};

	//PositionView
	class PositionFormModel : public RecordFormModel<EmployeePosition>
	{
	public:
		static const Wt::WFormModel::Field titleField;
		static const Wt::WFormModel::Field typeField;

		PositionFormModel(PositionView *view, Wt::Dbo::ptr<EmployeePosition> positionPtr = Wt::Dbo::ptr<EmployeePosition>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		PositionView *_view = nullptr;
	};

	class PositionView : public RecordFormView
	{
	public:
		PositionView(Wt::Dbo::ptr<EmployeePosition> positionPtr);
		PositionView();
		virtual void initView() override;

		Wt::Dbo::ptr<EmployeePosition> positionPtr() const { return _model->recordPtr(); }
		Wt::WFormModel *model() const { return _model; }

	protected:
		PositionFormModel *_model = nullptr;
		Wt::Dbo::ptr<EmployeePosition> _tempPtr;
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
	class ServiceFormModel : public RecordFormModel<ClientService>
	{
	public:
		static const Wt::WFormModel::Field titleField;

		ServiceFormModel(ServiceView *view, Wt::Dbo::ptr<ClientService> servicePtr = Wt::Dbo::ptr<ClientService>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		ServiceView *_view = nullptr;
	};
	class ServiceView : public RecordFormView
	{
	public:
		ServiceView(Wt::Dbo::ptr<ClientService> servicePtr);
		ServiceView();
		virtual void initView() override;

		Wt::Dbo::ptr<ClientService> servicePtr() const { return _model->recordPtr(); }
		ServiceFormModel *model() const { return _model; }

	protected:
		ServiceFormModel *_model = nullptr;
		Wt::Dbo::ptr<ClientService> _tempPtr;
	};

	//EmployeeAssignmentView
	class EmployeeAssignmentFormModel : public RecordFormModel<EmployeeAssignment>
	{
	public:
		static const Field descriptionField;
		static const Field startDateField;
		static const Field endDateField;
		static const Field entityField;
		static const Field locationField;
		static const Field cycleField;
		static const Field positionField;

		EmployeeAssignmentFormModel(EmployeeAssignmentView *view, Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr = Wt::Dbo::ptr<EmployeeAssignment>());
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

	protected:
		virtual void persistedHandler() override;
		void updateEndDateValidator(bool update);
		EmployeeAssignmentView *_view = nullptr;

	private:
		friend class EmployeeAssignmentView;
	};

	class EmployeeAssignmentView : public RecordFormView
	{
	public:
		EmployeeAssignmentView(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr = Wt::Dbo::ptr<EmployeeAssignment>());
		virtual void initView() override;

		using RecordFormView::updateView;
		using RecordFormView::updateModel;

		void handlePositionChanged();
		Wt::WDialog *createAddPositionDialog();
		QueryProxyModelCB<PositionProxyModel> *positionCombo() const { return _positionCombo; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return employeeAssignmentPtr() && employeeAssignmentPtr()->entityPtr ? EmployeeAssignment::viewInternalPath(employeeAssignmentPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new EmployeeAssignmentView(); }

		Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr() const { return _model->recordPtr(); }
		EmployeeAssignmentFormModel *model() const { return _model; }

	protected:
		virtual void updateView(Wt::WFormModel *model) override;
		virtual void updateModel(Wt::WFormModel *model) override;

		QueryProxyModelCB<PositionProxyModel> *_positionCombo = nullptr;
		EmployeeAssignmentFormModel *_model = nullptr;
		Wt::Dbo::ptr<EmployeeAssignment> _tempPtr;

	private:
		friend class EmployeeAssignmentFormModel;
	};

	//ClientAssignmentView
	class ClientAssignmentFormModel : public RecordFormModel<ClientAssignment>
	{
	public:
		static const Field descriptionField;
		static const Field startDateField;
		static const Field endDateField;
		static const Field entityField;
		static const Field cycleField;
		static const Field serviceField;

		ClientAssignmentFormModel(ClientAssignmentView *view, Wt::Dbo::ptr<ClientAssignment> clientAssignmentPtr = Wt::Dbo::ptr<ClientAssignment>());
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

	protected:
		void updateEndDateValidator(bool update);
		ClientAssignmentView *_view = nullptr;

	private:
		friend class ClientAssignmentView;
	};

	class ClientAssignmentView : public RecordFormView
	{
	public:
		ClientAssignmentView(Wt::Dbo::ptr<ClientAssignment> clientAssignmentPtr = Wt::Dbo::ptr<ClientAssignment>());
		virtual void initView() override;

		using RecordFormView::updateView;
		using RecordFormView::updateModel;

		void handleServiceChanged();
		Wt::WDialog *createAddServiceDialog();
		QueryProxyModelCB<ServiceProxyModel> *serviceCombo() const { return _serviceCombo; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return clientAssignmentPtr() && clientAssignmentPtr()->entityPtr ? ClientAssignment::viewInternalPath(clientAssignmentPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new ClientAssignmentView(); }

		Wt::Dbo::ptr<ClientAssignment> clientAssignmentPtr() const { return _model->recordPtr(); }
		ClientAssignmentFormModel *model() const { return _model; }

	protected:
		virtual void updateView(Wt::WFormModel *model) override;
		virtual void updateModel(Wt::WFormModel *model) override;

		QueryProxyModelCB<ServiceProxyModel> *_serviceCombo = nullptr;
		ClientAssignmentFormModel *_model = nullptr;
		Wt::Dbo::ptr<ClientAssignment> _tempPtr;

	private:
		friend class ClientAssignmentFormModel;
	};

	//EmployeeAssignmentList
	class EmployeeAssignmentList : public QueryModelFilteredList<boost::tuple<long long, Wt::WDateTime, std::string, Wt::WDate, Wt::WDate, std::string, std::string, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResTimestamp, ResEntityName, ResStartDate, ResEndDate, ResCountryName, ResCityName, ResAddress };
		enum ViewColumns { ViewId, ViewCreatedOn, ViewEntity, ViewStartDate, ViewEndDate, ViewCountry, ViewCity, ViewAddress };
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
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

	//ClientAssignmentList
	class ClientAssignmentList : public QueryModelFilteredList<boost::tuple<long long, Wt::WDateTime, std::string, Wt::WDate, Wt::WDate, long long>>
	{
	public:
		enum ResultColumns { ResId, ResTimestamp, ResEntityName, ResStartDate, ResEndDate, ResEmployeesAssigned };
		enum ViewColumns { ViewId, ViewCreatedOn, ViewEntity, ViewStartDate, ViewEndDate, ViewEmployeesAssigned };
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class ClientAssignmentListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		ClientAssignmentListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

// 	class MultipleViewTemplate : public Wt::WTemplate, public SubmittableRecordWidget
// 	{
// 	public:
// 		MultipleViewTemplate(Wt::WContainerWidget *parent = nullptr) : Wt::WTemplate(parent), SubmittableRecordWidget(this) { init(); }
// 		MultipleViewTemplate(const Wt::WString &text, Wt::WContainerWidget *parent = nullptr) : Wt::WTemplate(text, parent), SubmittableRecordWidget(this) { init(); }
// 
// 		virtual Wt::WString viewName() const override;
// 		virtual std::string viewInternalPath() const override;
// 
// 	protected:
// 		bool canSubmitAdditionalViews() const { return _mainView->model()->isRecordPersisted(); }
// 		RecordFormView *_mainView = nullptr;
// 
// 	private:
// 		void init();
// 	};
// 
// 	class EmployeeExpenseView : public MultipleViewTemplate
// 	{
// 	public:
// 		EmployeeExpenseView(bool isEmployeeAssignment);
// 		EmployeeExpenseView(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr);
// 		EmployeeExpenseView(Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr);
// 		virtual void load() override;
// 		void updateFromViewCounts();
// 		virtual SubmittableRecordWidget *createFormView() override { return new EmployeeExpenseView(_isEmployeeAssignment); }
// 
// 	protected:
// 		void handleMainViewSubmitted();
// 		EmployeeAssignmentView *addAssignment(Wt::Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr);
// 		ExpenseCycleView *addCycle(Wt::Dbo::ptr<ExpenseCycle> expenseCyclePtr);
// 		void handleAddCycle();
// 		void handleAddAssignment();
// 
// 		bool canAddCycle() const { return !_cycleView; }
// 		bool canAddAssignment() const;
// 
// 		bool _isEmployeeAssignment;
// 		Wt::Dbo::ptr<EmployeeAssignment> _tempAssignment;
// 		Wt::Dbo::ptr<ExpenseCycle> _tempCycle;
// 		Wt::WContainerWidget *_assignments = nullptr;
// 		Wt::WContainerWidget *_cycles = nullptr;
// 		EntryCycleView *_cycleView = nullptr;
// 	};
// 
// 	class ClientIncomeView : public MultipleViewTemplate
// 	{
// 	public:
// 		ClientIncomeView(bool isClientAssignment);
// 		ClientIncomeView(Wt::Dbo::ptr<ClientAssignment> employeeAssignmentPtr);
// 		ClientIncomeView(Wt::Dbo::ptr<IncomeCycle> incomeCyclePtr);
// 		virtual void load() override;
// 		void updateFromViewCounts();
// 		virtual SubmittableRecordWidget *createFormView() override { return new ClientIncomeView(_isClientAssignment); }
// 
// 	protected:
// 		void handleMainViewSubmitted();
// 		ClientAssignmentView *addAssignment(Wt::Dbo::ptr<ClientAssignment> employeeAssignmentPtr);
// 		IncomeCycleView *addCycle(Wt::Dbo::ptr<IncomeCycle> incomeCyclePtr);
// 		void handleAddCycle();
// 		void handleAddAssignment();
// 
// 		bool canAddCycle() const { return !_cycleView; }
// 		bool canAddAssignment() const;
// 
// 		bool _isClientAssignment;
// 		Wt::Dbo::ptr<ClientAssignment> _tempAssignment;
// 		Wt::Dbo::ptr<IncomeCycle> _tempCycle;
// 		Wt::WContainerWidget *_assignments = nullptr;
// 		Wt::WContainerWidget *_cycles = nullptr;
// 		EntryCycleView *_cycleView = nullptr;
// 	};

}

#endif