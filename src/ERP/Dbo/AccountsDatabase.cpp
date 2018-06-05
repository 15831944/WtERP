#include "Dbo/AccountsDatabase.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Application/WServer.h"

namespace ERP
{
	Dbo::ptr<Account> AccountsDatabase::findOrCreateCashAccount(bool loadLazy)
	{
		Dbo::Transaction t(dboSession);
		Dbo::ptr<Account> accountPtr;
		WServer *server = SERVER;

		//Config found
		if(server->configs().getLongIntPtr("CashAccountId"))
		{
			long long accountId = server->configs().getLongInt("CashAccountId", -1);
			if(accountId != -1)
			{
				if(loadLazy)
					accountPtr = dboSession.loadLazy<Account>(accountId);
				else
					accountPtr = dboSession.load<Account>(accountId, true);
			}
		}

		//Create account if not found
		if(!accountPtr)
		{
			Wt::log("erp-info") << "AccountsDatabase: Cash account was not found in database, creating cash account";
			accountPtr = dboSession.addNew<Account>(Account::Asset);
			accountPtr.modify()->name = tr("CashAccountName").toUTF8();
			accountPtr.flush();

			if(!server->configs().addLongInt("CashAccountId", accountPtr.id(), &dboSession))
				throw std::runtime_error("Error creating CashAccountId config, ConfigurationsDatabase::addLongInt() returned null");
		}

		t.commit();
		return accountPtr;
	}

	void AccountsDatabase::createEntityAccountsIfNotFound(Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return;

		Dbo::Transaction t(dboSession);

		if(!entityPtr->balAccountPtr)
		{
			auto accountPtr = dboSession.addNew<Account>(Account::EntityBalanceAccount);
			accountPtr.modify()->_creatorUserPtr = entityPtr->creatorUserPtr();
			accountPtr.modify()->_regionPtr = entityPtr->regionPtr();
			accountPtr.modify()->name = tr("EntityIdBalanceAccount").arg(entityPtr.id()).toUTF8();

			entityPtr.modify()->balAccountPtr = accountPtr;
		}
		if(!entityPtr->pnlAccountPtr)
		{
			auto accountPtr = dboSession.addNew<Account>(Account::EntityPnlAccount);
			accountPtr.modify()->_creatorUserPtr = entityPtr->creatorUserPtr();
			accountPtr.modify()->_regionPtr = entityPtr->regionPtr();
			accountPtr.modify()->name = tr("EntityIdPnlAccount").arg(entityPtr.id()).toUTF8();

			entityPtr.modify()->pnlAccountPtr = accountPtr;
		}

		t.commit();
	}

	Dbo::ptr<AccountEntry> AccountsDatabase::createAccountEntry(const Money &amount, Dbo::ptr<Account> debitAccountPtr, Dbo::ptr<Account> creditAccountPtr)
	{
		if(!debitAccountPtr || !creditAccountPtr)
			return nullptr;

		Dbo::Transaction t(dboSession);
		auto result = dboSession.add(unique_ptr<AccountEntry>(new AccountEntry(amount, debitAccountPtr, creditAccountPtr)));
		result.modify()->setCreatedByValues();

		updateAccountBalances(result);
		t.commit();

		return result;
	}

	void AccountsDatabase::updateAccountBalances(Dbo::ptr<AccountEntry> accountEntryPtr)
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

	void AccountsDatabase::createPendingCycleEntry(Dbo::ptr<IncomeCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration)
	{
		Dbo::Transaction t(dboSession);

		if(!cyclePtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr was null");
		if(!cyclePtr->entityPtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr->entityPtr was null");
		if(!currentDt.isValid())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): currentDt was invalid");

		if(lastEntryPtr && lastEntryPtr->incomeCyclePtr && lastEntryPtr->incomeCyclePtr.id() != cyclePtr.id())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): lastEntryPtr->incomeCyclePtr != cyclePtr");

		createEntityAccountsIfNotFound(cyclePtr->entityPtr);

		auto assignmentCount = cyclePtr->clientAssignmentCollection.size();

		while(Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentDt, nextEntryDuration))
		{
			newEntry.modify()->incomeCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->balAccountPtr;
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->pnlAccountPtr;
			newEntry.modify()->description.arg(tr("income"));
			if(assignmentCount > 0)
				newEntry.modify()->description += trn("forNClientAssignments", assignmentCount).arg(assignmentCount).toUTF8();

			updateAccountBalances(newEntry);

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	void AccountsDatabase::createPendingCycleEntry(Dbo::ptr<ExpenseCycle> cyclePtr, Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration)
	{
		Dbo::Transaction t(dboSession);

		if(!cyclePtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr was null");
		if(!cyclePtr->entityPtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr->entityPtr was null");
		if(!currentDt.isValid())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): currentDt was invalid");
		
		if(lastEntryPtr && lastEntryPtr->expenseCyclePtr && lastEntryPtr->expenseCyclePtr.id() != cyclePtr.id())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): lastEntryPtr->expenseCyclePtr != cyclePtr");

		createEntityAccountsIfNotFound(cyclePtr->entityPtr);

		auto assignmentCount = cyclePtr->employeeAssignmentCollection.size();

		while(Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentDt, nextEntryDuration))
		{
			newEntry.modify()->expenseCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->pnlAccountPtr;
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->balAccountPtr;
			newEntry.modify()->description.arg(tr("expense"));
			if(assignmentCount > 0)
				newEntry.modify()->description += trn("forNEmployeeAssignments", assignmentCount).arg(assignmentCount).toUTF8();

			updateAccountBalances(newEntry);

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

		return dboSession.addNew<AccountEntry>(move(newEntry));
	}
}