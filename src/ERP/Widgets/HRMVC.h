#ifndef GS_HR_MVC_H
#define GS_HR_MVC_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Utilities/RecordFormView.h"
#include "Utilities/FilteredList.h"

namespace ERP
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
	class PositionProxyModel : public QueryProxyModel<Dbo::ptr<EmployeePosition>>
	{
	public:
		typedef Dbo::QueryModel<Dbo::ptr<EmployeePosition>> QueryModel;
		PositionProxyModel(shared_ptr<QueryModel> sourceModel);

	protected:
		void addAdditionalRows();
	};

	//PositionView
	class PositionFormModel : public RecordFormModel<EmployeePosition>
	{
	public:
		static const Wt::WFormModel::Field titleField;
		static const Wt::WFormModel::Field typeField;

		PositionFormModel(PositionView *view, Dbo::ptr<EmployeePosition> positionPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		PositionView *_view = nullptr;
	};

	class PositionView : public RecordFormView
	{
	public:
		PositionView(Dbo::ptr<EmployeePosition> positionPtr);
		PositionView();
		virtual void initView() override;

		Dbo::ptr<EmployeePosition> positionPtr() const { return _model->recordPtr(); }
		shared_ptr<Wt::WFormModel> model() const { return _model; }

	protected:
		shared_ptr<PositionFormModel> _model;
		Dbo::ptr<EmployeePosition> _tempPtr;
	};

	//ServiceProxyModel
	class ServiceProxyModel : public QueryProxyModel<Dbo::ptr<ClientService>>
	{
	public:
		typedef Dbo::QueryModel<Dbo::ptr<ClientService>> QueryModel;
		ServiceProxyModel(shared_ptr<QueryModel> sourceModel);

	protected:
		void addAdditionalRows();
	};

	//ServiceView
	class ServiceFormModel : public RecordFormModel<ClientService>
	{
	public:
		static const Wt::WFormModel::Field titleField;

		ServiceFormModel(ServiceView *view, Dbo::ptr<ClientService> servicePtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		ServiceView *_view = nullptr;
	};
	class ServiceView : public RecordFormView
	{
	public:
		ServiceView(Dbo::ptr<ClientService> servicePtr);
		ServiceView();
		virtual void initView() override;

		Dbo::ptr<ClientService> servicePtr() const { return _model->recordPtr(); }
		shared_ptr<ServiceFormModel> model() const { return _model; }

	protected:
		shared_ptr<ServiceFormModel> _model;
		Dbo::ptr<ClientService> _tempPtr;
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

		EmployeeAssignmentFormModel(EmployeeAssignmentView *view, Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

		void showExpenseCycleDialog();
		void showClientAssignmentDialog();

	protected:
		virtual void persistedHandler() override;
		void updateEndDateValidator(bool update);

		void handleExpenseCycleSelected(long long id);
		void handleClientAssignmentSelected(long long id);
		void handleDialogFinished();

		EmployeeAssignmentView *_view = nullptr;
		Wt::WDialog *_dialog = nullptr;

	private:
		friend class EmployeeAssignmentView;
	};

	class EmployeeAssignmentView : public ReloadOnVisibleWidget<RecordFormView>
	{
	public:
		EmployeeAssignmentView(Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr = nullptr);
		virtual void reload() override;

		using RecordFormView::updateView;
		using RecordFormView::updateModel;

		void handlePositionChanged();
		void showAddPositionDialog();
		QueryProxyModelCB<PositionProxyModel> *positionCombo() const { return _positionCombo; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return employeeAssignmentPtr() && employeeAssignmentPtr()->entityPtr ? EmployeeAssignment::viewInternalPath(employeeAssignmentPtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<EmployeeAssignmentView>(); }

		Dbo::ptr<EmployeeAssignment> employeeAssignmentPtr() const { return _model->recordPtr(); }
		shared_ptr<EmployeeAssignmentFormModel> model() const { return _model; }

	protected:
		virtual void initView() override;
		virtual void updateView(Wt::WFormModel *model) override;
		virtual void updateModel(Wt::WFormModel *model) override;

		QueryProxyModelCB<PositionProxyModel> *_positionCombo = nullptr;
		Wt::WDialog *_dialog = nullptr;
		shared_ptr<EmployeeAssignmentFormModel> _model;
		Dbo::ptr<EmployeeAssignment> _tempPtr;

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

		ClientAssignmentFormModel(ClientAssignmentView *view, Dbo::ptr<ClientAssignment> clientAssignmentPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool saveChanges() override;

		void showIncomeCycleDialog();

	protected:
		virtual void persistedHandler() override;
		void updateEndDateValidator(bool update);

		void handleIncomeCycleSelected(long long id);
		void handleDialogFinished();

		ClientAssignmentView *_view = nullptr;
		Wt::WDialog *_dialog = nullptr;

	private:
		friend class ClientAssignmentView;
	};

	class ClientAssignmentView : public ReloadOnVisibleWidget<RecordFormView>
	{
	public:
		ClientAssignmentView(Dbo::ptr<ClientAssignment> clientAssignmentPtr = nullptr);
		virtual void reload() override;
		virtual void initView() override;

		using RecordFormView::updateView;
		using RecordFormView::updateModel;

		void handleServiceChanged();
		void showAddServiceDialog();
		QueryProxyModelCB<ServiceProxyModel> *serviceCombo() const { return _serviceCombo; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return clientAssignmentPtr() && clientAssignmentPtr()->entityPtr ? ClientAssignment::viewInternalPath(clientAssignmentPtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<ClientAssignmentView>(); }

		Dbo::ptr<ClientAssignment> clientAssignmentPtr() const { return _model->recordPtr(); }
		shared_ptr<ClientAssignmentFormModel> model() const { return _model; }

	protected:
		virtual void updateView(Wt::WFormModel *model) override;
		virtual void updateModel(Wt::WFormModel *model) override;

		QueryProxyModelCB<ServiceProxyModel> *_serviceCombo = nullptr;
		Wt::WDialog *_dialog = nullptr;
		shared_ptr<ClientAssignmentFormModel> _model;
		Dbo::ptr<ClientAssignment> _tempPtr;

	private:
		friend class ClientAssignmentFormModel;
	};

	//EmployeeAssignmentList
	class EmployeeAssignmentList : public QueryModelFilteredList<tuple<long long, Wt::WDateTime, std::string, Wt::WDate, Wt::WDate, std::string, std::string, std::string>>
	{
	public:
		EmployeeAssignmentList(Dbo::ptr<Entity> entityPtr = nullptr) : _entityPtr(entityPtr) { }
		EmployeeAssignmentList(Dbo::ptr<ClientAssignment> clientAssignmentPtr) : _clientAssignmentPtr(clientAssignmentPtr) { }
		EmployeeAssignmentList(Dbo::ptr<ExpenseCycle> cyclePtr) : _cyclePtr(cyclePtr) { }
		enum ResultColumns { ResId, ResTimestamp, ResEntityName, ResStartDate, ResEndDate, ResCountryName, ResCityName, ResAddress };
		enum ViewColumns { ViewId, ViewCreatedOn, ViewEntity, ViewStartDate, ViewEndDate, ViewCountry, ViewCity, ViewAddress };
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;

		Dbo::ptr<ClientAssignment> _clientAssignmentPtr;
		Dbo::ptr<Entity> _entityPtr;
		Dbo::ptr<ExpenseCycle> _cyclePtr;
	};

	class EmployeeAssignmentListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		EmployeeAssignmentListProxyModel(shared_ptr<Wt::WAbstractItemModel> model);
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	//ClientAssignmentList
	class ClientAssignmentList : public QueryModelFilteredList<tuple<long long, Wt::WDateTime, std::string, Wt::WDate, Wt::WDate, long long>>
	{
	public:
		enum ResultColumns { ResId, ResTimestamp, ResEntityName, ResStartDate, ResEndDate, ResEmployeesAssigned };
		enum ViewColumns { ViewId, ViewCreatedOn, ViewEntity, ViewStartDate, ViewEndDate, ViewEmployeesAssigned };
		ClientAssignmentList(Dbo::ptr<Entity> entityPtr = nullptr) : _entityPtr(entityPtr) { }
		ClientAssignmentList(Dbo::ptr<IncomeCycle> cyclePtr) : _cyclePtr(cyclePtr) { }
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
		Dbo::ptr<Entity> _entityPtr;
		Dbo::ptr<IncomeCycle> _cyclePtr;
	};

	class ClientAssignmentListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		ClientAssignmentListProxyModel(shared_ptr<Wt::WAbstractItemModel> model);
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};
}

#endif