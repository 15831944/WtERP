#ifndef ERP_ACCOUNTSDATABASE_H
#define ERP_ACCOUNTSDATABASE_H

#include "Common.h"
#include "Dbo/Dbos.h"

namespace ERP
{
	enum DefaultAccount
	{
		CashAcc,
		DiscountsRecievedAcc,
		DiscountsAllowedAcc,
		SalesTaxPayableAcc,
		BadDebtsAcc,
		
		DefaultAccountsCount
	};
	enum DefaultControlAcc
	{ //ENSURE THE LIST IS TOPOLOGICALLY SORTED
		
		//Top level
		CurrentAssetsControlAcc,
		CurrentLiabilitiesControlAcc,
		OperatingIncomesControlAcc,
		OperatingExpensesControlAcc,
		RevenuesControlAcc,
		COGSControlAcc,
		//Lower level
		AccountsReceivableControlAcc,
		AccountsPayableControlAcc,
		DoubtfulDebtsControlAcc,
		RecurringIncomesControlAcc,
		RecurringExpensesControlAcc,
		InventoryControlAcc,
		
		DefaultControlAccsCount
	};
	
	class AccountsDatabase
	{
	private:
		struct AccountInfo
		{
			std::string prefix;
			DefaultControlAcc controlAcc = DefaultControlAccsCount;
		};
		AccountInfo _accInfo[DefaultAccountsCount];
		AccountInfo _ctrlInfo[DefaultControlAccsCount];
		void _initAccountsInfo()
		{
			//Top level control acounts
			_ctrlInfo[CurrentAssetsControlAcc] = { "CurrentAssets" };
			_ctrlInfo[CurrentLiabilitiesControlAcc] = { "CurrentLiabilities" };
			_ctrlInfo[OperatingIncomesControlAcc] = { "OperatingIncomes" };
			_ctrlInfo[OperatingExpensesControlAcc] = { "OperatingExpenses" };
			_ctrlInfo[RevenuesControlAcc] = { "Revenues" };
			_ctrlInfo[COGSControlAcc] = { "COGS" };
			
			//Lower level control accounts
			_ctrlInfo[AccountsReceivableControlAcc] = { "Receivables", CurrentAssetsControlAcc };
			_ctrlInfo[AccountsPayableControlAcc] = { "Payables", CurrentLiabilitiesControlAcc };
			_ctrlInfo[DoubtfulDebtsControlAcc] = { "DoubtfulDebts", CurrentAssetsControlAcc };
			_ctrlInfo[RecurringIncomesControlAcc] = { "RecurringIncomes", OperatingIncomesControlAcc };
			_ctrlInfo[RecurringExpensesControlAcc] = { "RecurringExpenses", OperatingExpensesControlAcc };
			_ctrlInfo[InventoryControlAcc] = { "Inventory", CurrentAssetsControlAcc };
			
			//Simple accounts
			_accInfo[CashAcc] = { "Cash", CurrentAssetsControlAcc };
			_accInfo[DiscountsRecievedAcc] = { "DiscReceived", OperatingIncomesControlAcc };
			_accInfo[DiscountsAllowedAcc] = { "DiscAllowed", OperatingExpensesControlAcc };
			_accInfo[SalesTaxPayableAcc] = { "SalesTaxPayable", CurrentLiabilitiesControlAcc };
			_accInfo[BadDebtsAcc] = { "BadDebts", OperatingExpensesControlAcc };
		}
		
	public:
		AccountsDatabase(DboSession &serverDboSession);
		static AccountsDatabase &instance();
		
		void createDefaultAccounts();
		void createEntityBalanceAccIfNotFound(Dbo::ptr<Entity> entityPtr);
		void createEntityRecurringIncomesAccIfNotFound(Dbo::ptr<Entity> entityPtr);
		void createEntityRecurringExpensesAccIfNotFound(Dbo::ptr<Entity> entityPtr);
		void createEntityDoubtfulDebtsAccIfNotFound(Dbo::ptr<Entity> entityPtr);
		void createInventoryAssetAccIfNotFound(Dbo::ptr<InventoryItem> inventoryItemPtr);
		void createInventoryCOGSAccIfNotFound(Dbo::ptr<InventoryItem> inventoryItemPtr);

		Dbo::ptr<AccountEntry> createAccountEntry(const Money &amount, Dbo::ptr<Account> debitAccountPtr, Dbo::ptr<Account> creditAccountPtr);
		
		steady_clock::duration createAllPendingCycleEntries(steady_clock::duration maxEntryDuration);
		void createPendingCycleEntry(Dbo::ptr<IncomeCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration = nullptr);
		void createPendingCycleEntry(Dbo::ptr<ExpenseCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration = nullptr);
		
		long long getControlAccId(DefaultControlAcc account);
		long long getAccountId(DefaultAccount account);
		
		Dbo::ptr<ControlAccount> loadControlAcc(DefaultControlAcc account, bool loadLazy = false);
		Dbo::ptr<Account> loadAccount(DefaultAccount account, bool loadLazy = false);
		
		AccountEntryCollection abnormalAccountEntries() { return _accountEntryCheckAbnormal; }
		IncomeCycleCollection abnormalIncomeCycles() { return _incomeCycleCheckAbnormal; }
		ExpenseCycleCollection abnormalExpenseCycles() { return _expenseCycleCheckAbnormal; }

	private:
		Dbo::ptr<AccountEntry> _createPendingCycleEntry(
			const EntryCycle &cycle,
			Dbo::ptr<AccountEntry> lastEntryPtr,
			const Wt::WDateTime &currentDt,
			steady_clock::duration *nextEntryDuration
		);

		void _updateAccountBalances(Dbo::ptr<AccountEntry> accountEntryPtr, bool secondAttempt, bool disableFlushing);
		void _updateControlAccountBalances(Dbo::ptr<ControlAccount> controlAccPtr, long long valueInCents, bool disableFlushing);
		void _recalculateAccountBalances();
		
		DboSession &dboSession();
		
		//Pending cycle queries
		typedef tuple<Dbo::ptr<IncomeCycle>, Dbo::ptr<AccountEntry>> IncomeCycleTuple;
		typedef Dbo::collection<IncomeCycleTuple> IncomeTupleCollection;
		typedef tuple<Dbo::ptr<ExpenseCycle>, Dbo::ptr<AccountEntry>> ExpenseCycleTuple;
		typedef Dbo::collection<ExpenseCycleTuple> ExpenseTupleCollection;
		Dbo::Query<IncomeCycleTuple> _incomeCycleQuery;
		Dbo::Query<ExpenseCycleTuple> _expenseCycleQuery;
		
		//Abnormal entry check queries
		Dbo::Query<Dbo::ptr<AccountEntry>> _accountEntryCheckAbnormal;
		Dbo::Query<Dbo::ptr<IncomeCycle>> _incomeCycleCheckAbnormal;
		Dbo::Query<Dbo::ptr<ExpenseCycle>> _expenseCycleCheckAbnormal;
		
		DboSession &_serverDboSession;
		
		friend class TaskScheduler;
	};

}

#endif