#ifndef ERP_ACCOUNT_MVC_H
#define ERP_ACCOUNT_MVC_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Widgets/FilteredList.h"
#include "Widgets/RecordFormView.h"
#include "ModelView/FindRecordEdit.h"

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

	class AccountList : public QueryModelFilteredList<tuple<long long, std::string, Account::Nature, optional<long long>, long long, std::string>>
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

		AccountChildrenEntryList(Dbo::ptr<Account> accountPtr) : _accountPtr(move(accountPtr)) { }
		AccountChildrenEntryList(Dbo::ptr<IncomeCycle> cyclePtr) : _incomeCyclePtr(move(cyclePtr)) { }
		AccountChildrenEntryList(Dbo::ptr<ExpenseCycle> cyclePtr) : _expenseCyclePtr(move(cyclePtr)) { }
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
		AccountEntryList(Dbo::ptr<IncomeCycle> cyclePtr) : _incomeCyclePtr(move(cyclePtr)) { }
		AccountEntryList(Dbo::ptr<ExpenseCycle> cyclePtr) : _expenseCyclePtr(move(cyclePtr)) { }
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
		AccountNameValidator(bool mandatory = false, Wt::WString allowedName = "")
			: Wt::WLengthValidator(0, 70), _allowedName(move(allowedName))
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
		static const Wt::WFormModel::Field natureField;
		static const Wt::WFormModel::Field nameField;

		AccountFormModel(AccountView *view, Dbo::ptr<Account> accountPtr = nullptr);
		virtual void updateFromDb() override;
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

		Dbo::ptr<Account> accountPtr() const { return _model->recordPtr(); }
		AccountFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return accountPtr() ? Account::viewInternalPath(accountPtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<AccountView>(); }

	protected:
		AccountChildrenEntryList *_entryList = nullptr;
		AccountFormModel *_model = nullptr;
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

		virtual void updateFromDb() override;
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

		void handleAccountChanged(bool update = true);

	protected:
		BaseAccountEntryFormModel(AccountEntryView *view, Dbo::ptr<AccountEntry> accountEntryPtr = nullptr);
		virtual void persistedHandler() override;
		AccountEntryView *_view = nullptr;
	};

	class AccountEntryFormModel : public BaseAccountEntryFormModel
	{
	public:
		AccountEntryFormModel(AccountEntryView *view, Dbo::ptr<AccountEntry> accountEntryPtr = nullptr);
		virtual void updateFromDb() override;
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
		void handleEntityChanged();
		optional<bool> _isReceipt;

	private:
		friend class TransactionView;
	};

	class AccountEntryView : public RecordFormView
	{
	public:
		AccountEntryView(Dbo::ptr<AccountEntry> accountEntryPtr = nullptr);

		Dbo::ptr<AccountEntry> accountEntryPtr() const { return _model->recordPtr(); }
		BaseAccountEntryFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return accountEntryPtr() ? AccountEntry::viewInternalPath(accountEntryPtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<AccountEntryView>(); }

	protected:
		BaseAccountEntryFormModel *_model = nullptr;
	};

	//Transaction view
	class TransactionView : public AccountEntryView
	{
	public:
		TransactionView();

		void selectDirection(bool isReceipt);
		TransactionFormModel *model() const { return dynamic_cast<TransactionFormModel*>(_model); }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<TransactionView>(); }

	protected:
		virtual void initView() override;
		virtual void afterSubmitHandler() override;
		virtual void submit() override;

		Wt::WPushButton *_selectReceipt = nullptr;
		Wt::WPushButton *_selectPayment = nullptr;
	};

}

#endif