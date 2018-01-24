#ifndef GS_ACCOUNTSDATABASE_H
#define GS_ACCOUNTSDATABASE_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Dbo/EntitiesDatabase.h"

namespace ERP
{
	class AccountsDatabase
	{
	public:
		AccountsDatabase(Dbo::Session &session) : dboSession(session), _entitiesDatabase(dboSession) { }
		void createEntityAccountsIfNotFound(Dbo::ptr<Entity> entityPtr);

		Dbo::ptr<AccountEntry> createAccountEntry(const Money &amount, Dbo::ptr<Account> debitAccountPtr, Dbo::ptr<Account> creditAccountPtr);
		void createPendingCycleEntry(Dbo::ptr<IncomeCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration = nullptr);
		void createPendingCycleEntry(Dbo::ptr<ExpenseCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration = nullptr);
		
		Dbo::ptr<Account> findOrCreateCashAccount(bool loadLazy = false);

// 		Dbo::ptr<Account> findOrCreateSelfAccount();
// 		Dbo::ptr<Account> findOrCreateEntityAccount(Dbo::ptr<Entity> entityPtr);
// 		Dbo::ptr<Account> findOrCreateIncomeExpenseAccount(Dbo::ptr<IncomeCycle> cyclePtr);
// 		Dbo::ptr<Account> findOrCreateIncomeExpenseAccount(Dbo::ptr<ExpenseCycle> cyclePtr);
// 		Dbo::ptr<Account> findOrCreateGeneralIncomeAccount();
// 		Dbo::ptr<Account> findOrCreateGeneralExpenseAccount();

	protected:
		//void initEntityAccountValues(Dbo::ptr<Account> accountPtr, Dbo::ptr<Entity> entityPtr);

		Dbo::Session &dboSession;
		EntitiesDatabase _entitiesDatabase;

	private:
		Dbo::ptr<AccountEntry> _createPendingCycleEntry(
			const EntryCycle &cycle,
			Dbo::ptr<AccountEntry> lastEntryPtr,
			const Wt::WDateTime &currentDt,
			steady_clock::duration *nextEntryDuration
		);

		void updateAccountBalances(Dbo::ptr<AccountEntry> accountEntryPtr);
	};

}

#endif