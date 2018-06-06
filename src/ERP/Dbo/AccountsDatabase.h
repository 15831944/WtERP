#ifndef ERP_ACCOUNTSDATABASE_H
#define ERP_ACCOUNTSDATABASE_H

#include "Common.h"
#include "Dbo/Dbos.h"

namespace ERP
{
	class AccountsDatabase
	{
	public:
		AccountsDatabase(DboSession &serverDboSession);
		static AccountsDatabase &instance();
		
		void createDefaultAccountsIfNotFound();
		void createEntityBalanceAccIfNotFound(Dbo::ptr<Entity> entityPtr);

		Dbo::ptr<AccountEntry> createAccountEntry(const Money &amount, Dbo::ptr<Account> debitAccountPtr, Dbo::ptr<Account> creditAccountPtr);
		
		steady_clock::duration createAllPendingCycleEntries(steady_clock::duration maxEntryDuration);
		void createPendingCycleEntry(Dbo::ptr<IncomeCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration = nullptr);
		void createPendingCycleEntry(Dbo::ptr<ExpenseCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration = nullptr);
		
		AccountEntryCollection abnormalAccountEntries() { return _accountEntryCheckAbnormal; }
		IncomeCycleCollection abnormalIncomeCycles() { return _incomeCycleCheckAbnormal; }
		ExpenseCycleCollection abnormalExpenseCycles() { return _expenseCycleCheckAbnormal; }
		
		Dbo::ptr<Account> acquireCashAcc(bool loadLazy = false)
		{
			return _findOrCreateAccount("CashAccId", tr("CashAccName"), Account::AssetNature, loadLazy);
		}
		Dbo::ptr<Account> acquireRecurringIncomesAcc(bool loadLazy = false)
		{
			return _findOrCreateAccount("RecurringIncomesAccId", tr("RecurringIncomesAccName"), Account::IncomeNature, loadLazy);
		}
		Dbo::ptr<Account> acquireRecurringExpensesAcc(bool loadLazy = false)
		{
			return _findOrCreateAccount("RecurringExpensesAccId", tr("RecurringExpensesAccName"), Account::ExpenseNature, loadLazy);
		}

	private:
		Dbo::ptr<AccountEntry> _createPendingCycleEntry(
			const EntryCycle &cycle,
			Dbo::ptr<AccountEntry> lastEntryPtr,
			const Wt::WDateTime &currentDt,
			steady_clock::duration *nextEntryDuration
		);
		
		Dbo::ptr<Account> _findOrCreateAccount(
			const std::string &configName,
			const Wt::WString &accountName,
			Account::Nature accountNature,
			bool loadLazy
		);

		void _updateAccountBalances(Dbo::ptr<AccountEntry> accountEntryPtr);
		void _recalculateAccountBalances() { _recalculateBalanceCall->run(); }
		
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
		
		//Recalculate balances query
		unique_ptr<Dbo::Call> _recalculateBalanceCall;
		
		DboSession &_serverDboSession;
		
		friend class TaskScheduler;
	};

}

#endif