#ifndef GS_ACCOUNTLIST_WIDGET_H
#define GS_ACCOUNTLIST_WIDGET_H

#include "Dbo/Dbos.h"
#include "Utilities/FilteredList.h"
#include "Utilities/MyFormView.h"

#include <Wt/Dbo/QueryModel>
#include <Wt/WTemplateFormView>
#include <Wt/WBatchEditProxyModel>
#include <Wt/WLengthValidator>

namespace GS
{
	class AccountView;

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

	class AccountList : public QueryModelFilteredList<boost::tuple<long long, std::string, Account::Type, boost::optional<long long>, double, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResType, ResEntityId, ResBalance, ResEntityName };
		enum ViewColumns { ViewId, ViewName, ViewType, ViewEntity, ViewBalance };
		AccountList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	//ACCOUNT ENTRY list
	class AccountEntryListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AccountEntryListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
	};

	class AccountEntryList : public QueryModelFilteredList<boost::tuple<Wt::WDateTime, std::string, double, long long, long long, long long, std::string>>
	{
	public:
		enum ResultColumns { ResTimestamp, ResDescription, ResAmount, ResDebitAccountId, ResCreditAccountId, ResOppositeAccountId, ResOppositeAccountName };
		enum ViewColumns { ViewTimestamp, ViewDescription, ViewOppositeAccount, ViewDebitAmount, ViewCreditAmount };
		AccountEntryList(Wt::Dbo::ptr<Account> accountPtr, Wt::WContainerWidget *parent = nullptr);
		Wt::Dbo::ptr<Account> accountPtr() const { return _accountPtr; }

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;

		Wt::Dbo::ptr<Account> _accountPtr;
	};

	//FindAccountModel
	typedef boost::tuple<std::string, long long, Account::Type, std::string> _FAMTuple;
	class FindAccountModel : public Wt::Dbo::QueryModel<_FAMTuple>
	{
	public:
		enum ResultColumns { Name, Id, Type, EntityName };

		FindAccountModel(Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
	};

	//FindAccountEdit
	class FindAccountEdit : public Wt::WLineEdit
	{
	public:
		FindAccountEdit(Wt::WContainerWidget *parent = nullptr);
		void showNewAccountDialog();
		void showAccountListDialog();

		Wt::Dbo::ptr<Account> accountPtr() const { return _accountPtr; }
		void setAccountPtr(Wt::Dbo::ptr<Account> ptr);
		Wt::WPushButton *newAccountBtn() const { return _newAccount; }
		Wt::WPushButton *showListBtn() const { return _showList; }
		Wt::Signal<void> &accountChanged() { return _accountChanged; }

	protected:
		void handleAccountViewSubmitted(AccountView *view);
		void handleListSelectionChanged(AbstractFilteredList *listWidget);
		void handleActivated(int index, Wt::WFormWidget *lineEdit);

		Account::Type _accountType;
		Wt::WDialog *_listDialog = nullptr;
		Wt::WDialog *_newDialog = nullptr;
		Wt::WSuggestionPopup *_suggestionPopup = nullptr;
		Wt::WPushButton *_newAccount = nullptr;
		Wt::WPushButton *_showList = nullptr;
		Wt::Dbo::ptr<Account> _accountPtr;
		Wt::Signal<void> _accountChanged;

	private:
		friend class FindAccountValidator;
	};

	//FindAccountValidator
	class FindAccountValidator : public Wt::WValidator
	{
	public:
		FindAccountValidator(FindAccountEdit *findEdit, bool mandatory = false)
			: Wt::WValidator(mandatory, findEdit), _findEdit(findEdit)
		{ }
		virtual Result validate(const Wt::WString &input) const override;

	protected:
		FindAccountEdit *_findEdit;
	};

	//AccountNameValidator
	class AccountNameValidator : public Wt::WLengthValidator
	{
	public:
		AccountNameValidator(bool mandatory = false, Wt::WObject *parent = nullptr)
			: Wt::WLengthValidator(0, 70, parent)
		{
			setMandatory(mandatory);
		}
		virtual Result validate(const Wt::WString &input) const override;
	};

	//ACCOUNT view
	class AccountView : public MyTemplateFormView
	{
	public:
		static const Wt::WFormModel::Field typeField;
		static const Wt::WFormModel::Field nameField;

		AccountView(Wt::WContainerWidget *parent = nullptr);
		Wt::Dbo::ptr<Account> accountPtr() const { return _accountPtr; }
		Wt::WFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return accountPtr() ? Account::viewInternalPath(accountPtr().id()) : ""; }
		virtual MyTemplateFormView *createFormView() override { return new AccountView(); }

	protected:
		void submit();

		Wt::WFormModel *_model = nullptr;
		Wt::Dbo::ptr<Account> _accountPtr;
	};

	//ACCOUNT ENTRY view
	class AccountEntryView : public MyTemplateFormView
	{
	public:
		//static const Wt::WFormModel::Field typeField;
		static const Wt::WFormModel::Field descriptionField;
		static const Wt::WFormModel::Field debitAccountField;
		static const Wt::WFormModel::Field creditAccountField;
		static const Wt::WFormModel::Field amountField;
		static const Wt::WFormModel::Field entityField;

		AccountEntryView(Wt::WContainerWidget *parent = nullptr, bool isTransactionView = false);
		Wt::Dbo::ptr<AccountEntry> accountEntryPtr() const { return _accountEntryPtr; }
		Wt::WFormModel *model() const { return _model; }

		void handleAccountChanged();

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return accountEntryPtr() ? AccountEntry::viewInternalPath(accountEntryPtr().id()) : ""; }
		virtual MyTemplateFormView *createFormView() override { return new AccountEntryView(); }

	protected:
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;
		virtual void submit();

		Wt::WFormModel *_model = nullptr;
		Wt::Dbo::ptr<AccountEntry> _accountEntryPtr;
	};

	//Transaction view
	class TransactionView : public AccountEntryView
	{
	public:
		TransactionView(Wt::WContainerWidget *parent = nullptr);
		void selectDirection(bool isReceipt);
		void setAccountsFromEntity();

	protected:
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;
		virtual MyTemplateFormView *createFormView() override { return new TransactionView(); }
		virtual void submit() override;

		boost::optional<bool> _isReceipt;
		Wt::WPushButton *_selectReceipt = nullptr;
		Wt::WPushButton *_selectPayment = nullptr;
	};

}

#endif