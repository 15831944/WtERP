#ifndef GS_ACCOUNT_MVC_H
#define GS_ACCOUNT_MVC_H

#include "Dbo/Dbos.h"
#include "Utilities/FilteredList.h"
#include "Utilities/RecordFormView.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/Dbo/QueryModel>
#include <Wt/WTemplateFormView>
#include <Wt/WBatchEditProxyModel>
#include <Wt/WLengthValidator>
#include <Wt/WCompositeWidget>

namespace GS
{
	class AccountView;
	class AccountEntryView;

	//ACCOUNT List
	class AccountListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AccountListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AccountList : public QueryModelFilteredList<boost::tuple<long long, std::string, Account::Type, boost::optional<long long>, long long, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResType, ResEntityId, ResBalance, ResEntityName };
		enum ViewColumns { ViewId, ViewName, ViewType, ViewEntity, ViewBalance };
		AccountList();

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	//ACCOUNT ENTRY list
	class AccountChildrenEntryListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AccountChildrenEntryListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AccountEntryListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AccountEntryListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AccountChildrenEntryList : public QueryModelFilteredList<boost::tuple<Wt::WDateTime, std::string, long long, long long, long long, long long, std::string, long long>>
	{
	public:
		enum ResultColumns { ResTimestamp, ResDescription, ResAmount, ResDebitAccountId, ResCreditAccountId, ResOppositeAccountId, ResOppositeAccountName, ResId };
		enum ViewColumns { ViewTimestamp, ViewDescription, ViewOppositeAccount, ViewDebitAmount, ViewCreditAmount };

		AccountChildrenEntryList(Wt::Dbo::ptr<Account> accountPtr) : _accountPtr(accountPtr) { }
		AccountChildrenEntryList(Wt::Dbo::ptr<IncomeCycle> cyclePtr) : _incomeCyclePtr(cyclePtr) { }
		AccountChildrenEntryList(Wt::Dbo::ptr<ExpenseCycle> cyclePtr) : _expenseCyclePtr(cyclePtr) { }
		virtual void load() override;
		Wt::Dbo::ptr<Account> accountPtr() const { return _accountPtr; }

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;

		Wt::Dbo::ptr<Account> _accountPtr;
		Wt::Dbo::ptr<IncomeCycle> _incomeCyclePtr;
		Wt::Dbo::ptr<ExpenseCycle> _expenseCyclePtr;
	};

	class AccountEntryList : public QueryModelFilteredList<boost::tuple<Wt::WDateTime, std::string, long long, long long, long long, std::string, std::string, long long>>
	{
	public:
		enum ResultColumns { ResTimestamp, ResDescription, ResAmount, ResDebitAccountId, ResCreditAccountId, ResDebitAccountName, ResCreditAccountName, ResId };
		enum ViewColumns { ViewTimestamp, ViewDescription, ViewAmount, ViewDebitAccount, ViewCreditAccount };

		AccountEntryList() = default;
		AccountEntryList(Wt::Dbo::ptr<IncomeCycle> cyclePtr) : _incomeCyclePtr(cyclePtr) { }
		AccountEntryList(Wt::Dbo::ptr<ExpenseCycle> cyclePtr) : _expenseCyclePtr(cyclePtr) { }
		virtual void load() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;

		Wt::Dbo::ptr<IncomeCycle> _incomeCyclePtr;
		Wt::Dbo::ptr<ExpenseCycle> _expenseCyclePtr;
	};

	//AccountNameValidator
	class AccountNameValidator : public Wt::WLengthValidator
	{
	public:
		AccountNameValidator(bool mandatory = false, const Wt::WString &allowedName = "", Wt::WObject *parent = nullptr)
			: Wt::WLengthValidator(0, 70, parent), _allowedName(allowedName)
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

		AccountFormModel(AccountView *view, Wt::Dbo::ptr<Account> accountPtr = Wt::Dbo::ptr<Account>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		virtual void persistedHandler() override;
		virtual AuthLogin::PermissionResult checkModifyPermission() const override;
		AccountView *_view = nullptr;
	};

	class AccountView : public RecordFormView
	{
	public:
		AccountView(Wt::Dbo::ptr<Account> accountPtr = Wt::Dbo::ptr<Account>());
		virtual void initView() override;

		Wt::Dbo::ptr<Account> accountPtr() const { return _model->recordPtr(); }
		AccountFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return accountPtr() ? Account::viewInternalPath(accountPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new AccountView(); }

	protected:
		AccountChildrenEntryList *_entryList = nullptr;
		AccountFormModel *_model = nullptr;
		Wt::Dbo::ptr<Account> _tempPtr;
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

		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

		void handleAccountChanged(bool update = true);

	protected:
		virtual void persistedHandler() override;
		BaseAccountEntryFormModel(AccountEntryView *view, Wt::Dbo::ptr<AccountEntry> accountEntryPtr = Wt::Dbo::ptr<AccountEntry>());
		AccountEntryView *_view = nullptr;
	};

	class AccountEntryFormModel : public BaseAccountEntryFormModel
	{
	public:
		AccountEntryFormModel(AccountEntryView *view, Wt::Dbo::ptr<AccountEntry> accountEntryPtr = Wt::Dbo::ptr<AccountEntry>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
	};

	class TransactionFormModel : public BaseAccountEntryFormModel
	{
	public:
		TransactionFormModel(AccountEntryView *view, Wt::Dbo::ptr<AccountEntry> accountEntryPtr = Wt::Dbo::ptr<AccountEntry>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;
		
	protected:
		void setAccountsFromEntity();
		boost::optional<bool> _isReceipt;

	private:
		friend class TransactionView;
	};

	class AccountEntryView : public RecordFormView
	{
	public:
		AccountEntryView(Wt::Dbo::ptr<AccountEntry> accountEntryPtr = Wt::Dbo::ptr<AccountEntry>(), bool isTransactionView = false);
		virtual void initView() override;

		Wt::Dbo::ptr<AccountEntry> accountEntryPtr() const { return _model->recordPtr(); }
		BaseAccountEntryFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return accountEntryPtr() ? AccountEntry::viewInternalPath(accountEntryPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new AccountEntryView(); }

	protected:
		BaseAccountEntryFormModel *_model = nullptr;
		Wt::Dbo::ptr<AccountEntry> _tempPtr;
	};

	//Transaction view
	class TransactionView : public AccountEntryView
	{
	public:
		TransactionView();
		virtual void initView() override;

		TransactionFormModel *model() const { return dynamic_cast<TransactionFormModel*>(_model); }
		void selectDirection(bool isReceipt);
		virtual RecordFormView *createFormView() override { return new TransactionView(); }

	protected:
		virtual void afterSubmitHandler() override;
		virtual void submit() override;

		Wt::WPushButton *_selectReceipt = nullptr;
		Wt::WPushButton *_selectPayment = nullptr;
	};

}

#endif