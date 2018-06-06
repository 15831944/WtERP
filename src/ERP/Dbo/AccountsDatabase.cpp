#include "Dbo/AccountsDatabase.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Application/WServer.h"
#include "Application/WApplication.h"

#include <boost/format.hpp>

namespace ERP
{
	AccountsDatabase::AccountsDatabase(DboSession &serverDboSession)
		: _serverDboSession(serverDboSession)
	{
		//Entry cycle queries
		{
			std::string query = std::string("SELECT cycle, lastEntry FROM %1% cycle ") +
				"LEFT JOIN " + AccountEntry::tStr() + " lastEntry ON (cycle.id = lastEntry.%2%) "
				"LEFT JOIN " + AccountEntry::tStr() + " e2 ON (cycle.id = e2.%2% AND (lastEntry.timestamp < e2.timestamp OR lastEntry.timestamp = e2.timestamp AND lastEntry.id < e2.id)) "
				"WHERE (e2.id IS null) "
				"AND (cycle.startDate <= ? AND cycle.timestamp <= ?) "
				"AND (lastEntry.id IS null OR cycle.endDate IS null OR (lastEntry.timestamp <= ? AND cycle.endDate > lastEntry.timestamp AND cycle.endDate > cycle.startDate))";
			boost::format fString(query);

			fString % IncomeCycle::tStr() % "incomecycle_id";
			_incomeCycleQuery = dboSession().query<IncomeCycleTuple>(fString.str());

			fString % ExpenseCycle::tStr() % "expensecycle_id";
			_expenseCycleQuery = dboSession().query<ExpenseCycleTuple>(fString.str());
		}

		//Abnormal account entry check query
		{
			_accountEntryCheckAbnormal = dboSession().find<AccountEntry>().where("timestamp IS null OR amount < 0");
		}

		//Abnormal entry cycle check queries
		{
			std::string condition = "timestamp IS null OR startDate IS null OR \"interval\" < 0 OR \"interval\" > ? OR "
				"nIntervals < 1 OR (endDate IS NOT null AND endDate <= startDate)";
			_incomeCycleCheckAbnormal = dboSession().find<IncomeCycle>().where(condition).bind(YearlyInterval);
			_expenseCycleCheckAbnormal = dboSession().find<ExpenseCycle>().where(condition).bind(YearlyInterval);
		}
		
		//Recalculate balance update query
		{
			//TODO: Rewrite after updating account system
			Dbo::Transaction t(dboSession());
			_recalculateBalanceCall = make_unique<Dbo::Call>(dboSession().execute(
				"UPDATE " + Account::tStr() + " SET balance = "
				"COALESCE((SELECT SUM(dE.amount) FROM " + AccountEntry::tStr() + " dE WHERE dE.debit_account_id = " + Account::tStr() + ".id), 0)"
				"-COALESCE((SELECT SUM(cE.amount) FROM " + AccountEntry::tStr() + " cE WHERE cE.credit_account_id = " + Account::tStr() + ".id), 0)"
				", \"version\" = \"version\" + 1"));
			t.rollback();
		}
	}
	
	void AccountsDatabase::createDefaultAccountsIfNotFound()
	{
		Dbo::Transaction t(dboSession());
		acquireCashAcc();
		acquireRecurringIncomesAcc();
		acquireRecurringExpensesAcc();
	}
	
	Dbo::ptr<Account> AccountsDatabase::_findOrCreateAccount(const std::string &configName, const Wt::WString &accountName, Account::Nature accountNature, bool loadLazy)
	{
		Dbo::Transaction t(dboSession());
		Dbo::ptr<Account> accountPtr;
		WServer *server = SERVER;
		
		//Config found
		if(server->configs().getLongIntPtr(configName))
		{
			long long accountId = server->configs().getLongInt(configName, -1);
			if(accountId != -1)
			{
				if(loadLazy)
					accountPtr = dboSession().loadLazy<Account>(accountId);
				else
					accountPtr = dboSession().load<Account>(accountId, true);
			}
		}
		
		//Create account if not found
		if(!accountPtr)
		{
			Wt::log("erp-info") << "AccountsDatabase: Config account " << configName << " was not found in database, creating account and config";
			accountPtr = dboSession().addNew<Account>(accountNature);
			accountPtr.modify()->name = accountName.toUTF8();
			accountPtr.flush();
			
			if(!server->configs().addLongInt(configName, accountPtr.id(), &dboSession()))
				throw std::runtime_error("Error creating config account " + configName + ", ConfigurationsDatabase::addLongInt() returned null");
		}
		
		t.commit();
		return accountPtr;
	}

	void AccountsDatabase::createEntityBalanceAccIfNotFound(Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return;

		Dbo::Transaction t(dboSession());

		if(!entityPtr->balAccountPtr)
		{
			auto accountPtr = dboSession().addNew<Account>(Account::BalanceNature);
			accountPtr.modify()->_creatorUserPtr = entityPtr->creatorUserPtr();
			accountPtr.modify()->_regionPtr = entityPtr->regionPtr();
			accountPtr.modify()->name = tr("EntityBalanceAccName").arg(entityPtr->name).arg(entityPtr.id()).toUTF8();

			entityPtr.modify()->balAccountPtr = accountPtr;
		}

		t.commit();
	}

	Dbo::ptr<AccountEntry> AccountsDatabase::createAccountEntry(const Money &amount, Dbo::ptr<Account> debitAccountPtr, Dbo::ptr<Account> creditAccountPtr)
	{
		if(!debitAccountPtr || !creditAccountPtr)
			return nullptr;

		Dbo::Transaction t(dboSession());
		auto result = dboSession().add(unique_ptr<AccountEntry>(new AccountEntry(amount, debitAccountPtr, creditAccountPtr)));
		result.modify()->setCreatedByValues();
		
		_updateAccountBalances(result);
		t.commit();

		return result;
	}

	void AccountsDatabase::_updateAccountBalances(Dbo::ptr<AccountEntry> accountEntryPtr)
	{
		try
		{
			accountEntryPtr->_debitAccountPtr.modify()->_balanceInCents += accountEntryPtr->amount().valueInCents();
			accountEntryPtr->_creditAccountPtr.modify()->_balanceInCents -= accountEntryPtr->amount().valueInCents();
			accountEntryPtr->_debitAccountPtr.flush();
			accountEntryPtr->_creditAccountPtr.flush();
		}
		catch(const Dbo::StaleObjectException &)
		{
			Account debitAccount = *accountEntryPtr->_debitAccountPtr;
			Account creditAccount = *accountEntryPtr->_creditAccountPtr;

			try
			{
				accountEntryPtr.modify()->_debitAccountPtr.reread();
				accountEntryPtr.modify()->_creditAccountPtr.reread();
				accountEntryPtr->_debitAccountPtr.modify()->_balanceInCents += accountEntryPtr->amount().valueInCents();
				accountEntryPtr->_creditAccountPtr.modify()->_balanceInCents -= accountEntryPtr->amount().valueInCents();
				accountEntryPtr->_debitAccountPtr.flush();
				accountEntryPtr->_creditAccountPtr.flush();
			}
			catch(const Dbo::StaleObjectException &)
			{
				Wt::log("warn") << "AccountsDatabase::updateAccountBalances(): StaleObjectException caught twice";
				accountEntryPtr->_debitAccountPtr.modify()->operator=(debitAccount);
				accountEntryPtr->_creditAccountPtr.modify()->operator=(creditAccount);
				throw;
			}
		}
	}
	
	steady_clock::duration AccountsDatabase::createAllPendingCycleEntries(steady_clock::duration maxEntryDuration)
	{
		steady_clock::duration nextEntryDuration = maxEntryDuration;
		Wt::WDateTime currentDt = Wt::WDateTime::currentDateTime();
		
		//Income cycle
		_incomeCycleQuery.reset();
		_incomeCycleQuery.bind(currentDt).bind(currentDt).bind(currentDt);
		
		IncomeTupleCollection incomeCollection = _incomeCycleQuery;
		for(auto &tuple : incomeCollection)
		{
			Dbo::ptr<IncomeCycle> cyclePtr;
			Dbo::ptr<AccountEntry> lastEntryPtr;
			std::tie(cyclePtr, lastEntryPtr) = tuple;
			
			createPendingCycleEntry(cyclePtr, lastEntryPtr, currentDt, &nextEntryDuration);
		}
		
		//Expense cycle
		_expenseCycleQuery.reset();
		_expenseCycleQuery.bind(currentDt).bind(currentDt).bind(currentDt);
		
		ExpenseTupleCollection expenseCollection = _expenseCycleQuery;
		for(auto &tuple : expenseCollection)
		{
			Dbo::ptr<ExpenseCycle> cyclePtr;
			Dbo::ptr<AccountEntry> lastEntryPtr;
			std::tie(cyclePtr, lastEntryPtr) = tuple;
			
			createPendingCycleEntry(cyclePtr, lastEntryPtr, currentDt, &nextEntryDuration);
		}
		
		return nextEntryDuration;
	}

	void AccountsDatabase::createPendingCycleEntry(Dbo::ptr<IncomeCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration)
	{
		Dbo::Transaction t(dboSession());

		if(!cyclePtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr was null");
		if(!cyclePtr->entityPtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr->entityPtr was null");
		if(!currentDt.isValid())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): currentDt was invalid");

		if(lastEntryPtr && lastEntryPtr->incomeCyclePtr && lastEntryPtr->incomeCyclePtr.id() != cyclePtr.id())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): lastEntryPtr->incomeCyclePtr != cyclePtr");
		
		createEntityBalanceAccIfNotFound(cyclePtr->entityPtr);

		auto assignmentCount = cyclePtr->clientAssignmentCollection.size();

		while(Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentDt, nextEntryDuration))
		{
			newEntry.modify()->incomeCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->balAccountPtr;
			newEntry.modify()->_creditAccountPtr = acquireRecurringIncomesAcc();
			newEntry.modify()->description.arg(tr("income"));
			if(assignmentCount > 0)
				newEntry.modify()->description += trn("forNClientAssignments", assignmentCount).arg(assignmentCount).toUTF8();
			
			_updateAccountBalances(newEntry);

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	void AccountsDatabase::createPendingCycleEntry(Dbo::ptr<ExpenseCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration)
	{
		Dbo::Transaction t(dboSession());

		if(!cyclePtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr was null");
		if(!cyclePtr->entityPtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr->entityPtr was null");
		if(!currentDt.isValid())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): currentDt was invalid");
		
		if(lastEntryPtr && lastEntryPtr->expenseCyclePtr && lastEntryPtr->expenseCyclePtr.id() != cyclePtr.id())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): lastEntryPtr->expenseCyclePtr != cyclePtr");
		
		createEntityBalanceAccIfNotFound(cyclePtr->entityPtr);

		auto assignmentCount = cyclePtr->employeeAssignmentCollection.size();

		while(Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentDt, nextEntryDuration))
		{
			newEntry.modify()->expenseCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = acquireRecurringExpensesAcc();
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->balAccountPtr;
			newEntry.modify()->description.arg(tr("expense"));
			if(assignmentCount > 0)
				newEntry.modify()->description += trn("forNEmployeeAssignments", assignmentCount).arg(assignmentCount).toUTF8();
			
			_updateAccountBalances(newEntry);

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	Dbo::ptr<AccountEntry> AccountsDatabase::_createPendingCycleEntry(
		const EntryCycle &cycle,
		Dbo::ptr<AccountEntry> lastEntryPtr,
		const Wt::WDateTime &currentDt,
		steady_clock::duration *nextEntryDuration)
	{
		//DO NOT ALLOW THESE NULL DATES(in scan query)
		if(cycle.startDate.isNull() || cycle.timestamp().isNull())
			return nullptr;

		//Do not allow cycles with start dates later than today and creation dt later than now(already filtered, reasserted)
		if(cycle.startDate > currentDt.date() || cycle.timestamp() > currentDt)
			return nullptr;

		//Do not allow cycles with end dates earlier or equal to start date
		if(cycle.endDate.isValid() && cycle.endDate <= cycle.startDate)
			return nullptr;

		//Do not allow invalid interval or nIntervals
		if(cycle.interval < DailyInterval || cycle.interval > YearlyInterval)
			return nullptr;
		if(cycle.nIntervals < 1)
			return nullptr;

		if(lastEntryPtr)
		{
			//Do not allow cycles which have ended and the final entry had already been made(already filtered, reasserted)
			if(cycle.endDate.isValid() && cycle.endDate <= lastEntryPtr->timestamp().date())
				return nullptr;

			//Do not allow entries with invalid date(in scan query)
			if(lastEntryPtr->timestamp().isNull() || lastEntryPtr->timestamp() > currentDt)
				return nullptr;
		}

		//BEGIN
		bool checkElapsedDuration = false;
		if(lastEntryPtr || cycle.firstEntryAfterCycle)
			checkElapsedDuration = true;

		bool createEntry = false;
		bool incompleteDurationEntry = false;
		bool finalCompleteEntry = false;

		Wt::WDateTime previousCyclePeriodDt; //either startDate or lastEntry timestamp
		Wt::WDateTime nextCyclePeriodDt;

		if(checkElapsedDuration)
		{
			if(lastEntryPtr)
				previousCyclePeriodDt = lastEntryPtr->timestamp();
			else if(cycle.firstEntryAfterCycle)
				previousCyclePeriodDt = addCycleInterval(Wt::WDateTime(cycle.startDate), cycle.interval, cycle.nIntervals);
			else
			{
				Wt::log("error") << "AccountsDatabase::_createPendingCycleEntry() logic error: lastEntryPtr AND cycle.firstEntryAfterCycle were both false!";
				return nullptr;
			}

			//Its possible because of cycle.firstEntryAfterCycle
			if(previousCyclePeriodDt > currentDt)
				return nullptr;

			steady_clock::duration elapsedDuration = currentDt.toTimePoint() - previousCyclePeriodDt.toTimePoint();
			steady_clock::duration cycleDuration = nextCyclePeriodDt.toTimePoint() - previousCyclePeriodDt.toTimePoint();
			nextCyclePeriodDt = addCycleInterval(previousCyclePeriodDt, cycle.interval, cycle.nIntervals);

			if(elapsedDuration >= cycleDuration) //complete cycle
			{
				if(cycle.endDate.isValid() && nextCyclePeriodDt.date() >= cycle.endDate)
					finalCompleteEntry = true;

				createEntry = true;

				//Next entry duration
				if(nextEntryDuration && *nextEntryDuration > cycleDuration)
					*nextEntryDuration = cycleDuration;
			}
			else
			{ //incomplete duration
				//Create incomplete duration entry ONLY IF the cycle has ended
				//no entry has been made after the cycle.endDt: IS ASSERTED previously
				if(cycle.endDate.isValid() && nextCyclePeriodDt.date() >= cycle.endDate)
				{
					createEntry = true;
					incompleteDurationEntry = true;
				}
				else
				{
					createEntry = false;

					//Next entry duration
					steady_clock::duration remainingDuration = cycleDuration - elapsedDuration;
					if(nextEntryDuration && *nextEntryDuration > remainingDuration)
						*nextEntryDuration = remainingDuration;
				}
			}
		}
		else
		{
			nextCyclePeriodDt = Wt::WDateTime(cycle.startDate);
			createEntry = true;
		}

		if(!createEntry)
			return nullptr;

		AccountEntry newEntry;
		if(incompleteDurationEntry)
		{
			int cycleDays = previousCyclePeriodDt.date().daysTo(nextCyclePeriodDt.date());
			int incompleteDays = previousCyclePeriodDt.date().daysTo(cycle.endDate);
			double ratio = static_cast<double>(incompleteDays) / cycleDays;
			newEntry._amountInCents = (cycle.amount() * ratio).valueInCents();

			//Timestamp
			if(!lastEntryPtr && cycle.endDate == currentDt.date())
				newEntry._timestamp = currentDt;
			else if(lastEntryPtr)
				newEntry._timestamp = Wt::WDateTime(cycle.endDate, lastEntryPtr->timestamp().time());
			else
				newEntry._timestamp = Wt::WDateTime(cycle.endDate, Wt::WTime(0, 0));

			//Description
			newEntry.description = tr("IncompleteEntry");
		}
		else
		{
			newEntry._amountInCents = cycle._amountInCents;

			//Timestamp
			if(!lastEntryPtr && nextCyclePeriodDt.date() == currentDt.date())
				newEntry._timestamp = currentDt;
			else
				newEntry._timestamp = nextCyclePeriodDt;

			//Description
			if(finalCompleteEntry)
				newEntry.description = tr("FinalEntry");
			else if(!lastEntryPtr)
				newEntry.description = tr("InitialEntry");
			else
				newEntry.description = tr("RecurringEntry");
		}

		return dboSession().addNew<AccountEntry>(move(newEntry));
	}
	
	DboSession &AccountsDatabase::dboSession()
	{
		WApplication *app = APP;
		if(app)
			return app->dboSession();
		return _serverDboSession;
	}
	
	AccountsDatabase &AccountsDatabase::instance()
	{
		return SERVER->accountsDatabase();
	}
}