#ifndef ERP_ACCOUNT_MVC_H
#define ERP_ACCOUNT_MVC_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Utilities/FilteredList.h"
#include "Utilities/RecordFormView.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/Dbo/QueryModel.h>
#include <Wt/WTemplateFormView.h>
#include <Wt/WBatchEditProxyModel.h>
#include <Wt/WLengthValidator.h>
#include <Wt/WCompositeWidget.h>

namespace ERP
{
	class AccountView;
	class AccountEntryView;

	//ACCOUNT List
	class AccountListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AccountListProxyModel(shared_ptr<Wt::WAbstractItemModel> model);
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AccountList : public QueryModelFilteredList<tuple<long long, std::string, Account::Type, optional<long long>, long long, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResType, ResEntityId, ResBalance, ResEntityName };
		enum ViewColumns { ViewId, ViewName, ViewType, ViewEntity, ViewBalance };

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	//ACCOUNT ENTRY list
	class AccountChildrenEntryListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AccountChildrenEntryListProxyModel(shared_ptr<Wt::WAbstractItemModel> model);
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AccountEntryListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AccountEntryListProxyModel(shared_ptr<Wt::WAbstractItemModel> model);
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AccountChildrenEntryList : public QueryModelFilteredList<tuple<Wt::WDateTime, std::string, long long, long long, long long, long long, std::string, long long>>
	{
	public:
		enum ResultColumns { ResTimestamp, ResDescription, ResAmount, ResDebitAccountId, ResCreditAccountId, ResOppositeAccountId, ResOppositeAccountName, ResId };
		enum ViewColumns { ViewTimestamp, ViewDescription, ViewOppositeAccount, ViewDebitAmount, ViewCreditAmount };

		AccountChildrenEntryList(Dbo::ptr<Account> accountPtr) : _accountPtr(accountPtr) { }
		AccountChildrenEntryList(Dbo::ptr<IncomeCycle> cyclePtr) : _incomeCyclePtr(cyclePtr) { }
		AccountChildrenEntryList(Dbo::ptr<ExpenseCycle> cyclePtr) : _expenseCyclePtr(cyclePtr) { }
		virtual void load() override;
		Dbo::ptr<Account> accountPtr() const { return _accountPtr; }

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;

		Dbo::ptr<Account> _accountPtr;
		Dbo::ptr<IncomeCycle> _incomeCyclePtr;
		Dbo::ptr<ExpenseCycle> _expenseCyclePtr;
	};

	class AccountEntryList : public QueryModelFilteredList<tuple<Wt::WDateTime, std::string, long long, long long, long long, std::string, std::string, long long>>
	{
	public:
		enum ResultColumns { ResTimestamp, ResDescription, ResAmount, ResDebitAccountId, ResCreditAccountId, ResDebitAccountName, ResCreditAccountName, ResId };
		enum ViewColumns { ViewTimestamp, ViewDescription, ViewAmount, ViewDebitAccount, ViewCreditAccount };

		AccountEntryList() = default;
		AccountEntryList(Dbo::ptr<IncomeCycle> cyclePtr) : _incomeCyclePtr(cyclePtr) { }
		AccountEntryList(Dbo::ptr<ExpenseCycle> cyclePtr) : _expenseCyclePtr(cyclePtr) { }
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;

		Dbo::ptr<IncomeCycle> _incomeCyclePtr;
		Dbo::ptr<ExpenseCycle> _expenseCyclePtr;
	};

	//AccountNameValidator
	class AccountNameValidator : public Wt::WLengthValidator
	{
	public:
		AccountNameValidator(bool mandatory = false, const Wt::WString &allowedName = "")
			: Wt::WLengthValidator(0, 70), _allowedName(allowedName)
		{
			setMandatory(mandatory);
		}
		virtual Result validate(const Wt::WString &input) const override;
		void setAllowedName(const Wt::WString &name) { _allowedName = name; }

	protected:
		Wt::WString _allowedName;
	};

	//ACCOUNT view
	class AccountFormModel : public RecordFormModel<Account>
	{
	public:
		static const Wt::WFormModel::Field typeField;
		static const Wt::WFormModel::Field nameField;

		AccountFormModel(AccountView *view, Dbo::ptr<Account> accountPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		virtual void persistedHandler() override;
		virtual AuthLogin::PermissionResult checkModifyPermission() const override;
		AccountView *_view = nullptr;
	};

	class AccountView : public RecordFormView
	{
	public:
		AccountView(Dbo::ptr<Account> accountPtr = nullptr);
		virtual void initView() override;

		Dbo::ptr<Account> accountPtr() const { return _model->recordPtr(); }
		shared_ptr<AccountFormModel> model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return accountPtr() ? Account::viewInternalPath(accountPtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<AccountView>(); }

	protected:
		AccountChildrenEntryList *_entryList = nullptr;
		shared_ptr<AccountFormModel> _model;
		Dbo::ptr<Account> _tempPtr;
	};

	//ACCOUNT ENTRY view
	class BaseAccountEntryFormModel : public RecordFormModel<AccountEntry>
	{
	public:
		//static const Wt::WFormModel::Field typeField;
		static const Wt::WFormModel::Field descriptionField;
		static const Wt::WFormModel::Field debitAccountField;
		static const Wt::WFormModel::Field creditAccountField;
		static const Wt::WFormModel::Field amountField;
		static const Wt::WFormModel::Field entityField;

		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

		void handleAccountChanged(bool update = true);

	protected:
		virtual void persistedHandler() override;
		BaseAccountEntryFormModel(AccountEntryView *view, Dbo::ptr<AccountEntry> accountEntryPtr = nullptr);
		AccountEntryView *_view = nullptr;
	};

	class AccountEntryFormModel : public BaseAccountEntryFormModel
	{
	public:
		AccountEntryFormModel(AccountEntryView *view, Dbo::ptr<AccountEntry> accountEntryPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
	};

	class TransactionFormModel : public BaseAccountEntryFormModel
	{
	public:
		TransactionFormModel(AccountEntryView *view, Dbo::ptr<AccountEntry> accountEntryPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;
		
	protected:
		void setAccountsFromEntity();
		optional<bool> _isReceipt;

	private:
		friend class TransactionView;
	};

	class AccountEntryView : public RecordFormView
	{
	public:
		AccountEntryView(Dbo::ptr<AccountEntry> accountEntryPtr = nullptr);
		virtual void initView() override;

		Dbo::ptr<AccountEntry> accountEntryPtr() const { return _model->recordPtr(); }
		shared_ptr<BaseAccountEntryFormModel> model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return accountEntryPtr() ? AccountEntry::viewInternalPath(accountEntryPtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<AccountEntryView>(); }

	protected:
		shared_ptr<BaseAccountEntryFormModel> _model = nullptr;
		Dbo::ptr<AccountEntry> _tempPtr;
	};

	//Transaction view
	class TransactionView : public AccountEntryView
	{
	public:
		TransactionView();
		virtual void initView() override;

		void selectDirection(bool isReceipt);
		shared_ptr<TransactionFormModel> model() const { return dynamic_pointer_cast<TransactionFormModel>(_model); }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<TransactionView>(); }

	protected:
		virtual void afterSubmitHandler() override;
		virtual void submit() override;

		Wt::WPushButton *_selectReceipt = nullptr;
		Wt::WPushButton *_selectPayment = nullptr;
	};

}

#endif