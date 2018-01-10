#include "Dbo/AccountsDatabase.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Application/WServer.h"
#include <Wt/WLogger.h>

namespace GS
{
	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateCashAccount(bool loadLazy /*= false*/)
	{
		Wt::Dbo::Transaction t(dboSession);
		Wt::Dbo::ptr<Account> accountPtr;
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
			Wt::log("gs-info") << "AccountsDatabase: Cash account was not found in database, creating cash account";
			accountPtr = dboSession.addNew<Account>(Account::Asset);
			accountPtr.modify()->name = tr("CashAccountName").toUTF8();
			accountPtr.flush();

			if(!server->configs().addLongInt("CashAccountId", accountPtr.id(), &dboSession))
				throw std::runtime_error("Error creating CashAccountId config, ConfigurationsDatabase::addLongInt() returned null");
		}

		t.commit();
		return accountPtr;
	}

	void AccountsDatabase::createEntityAccountsIfNotFound(Wt::Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return;

		Wt::Dbo::Transaction t(dboSession);

		if(!entityPtr->balAccountPtr)
		{
			auto accountPtr = dboSession.addNew<Account>(Account::EntityBalanceAccount);
			accountPtr.modify()->creatorUserPtr = entityPtr->creatorUserPtr;
			accountPtr.modify()->regionPtr = entityPtr->regionPtr;
			accountPtr.modify()->name = tr("EntityIdBalanceAccount").arg(entityPtr.id()).toUTF8();

			entityPtr.modify()->balAccountPtr = accountPtr;
		}
		if(!entityPtr->pnlAccountPtr)
		{
			auto accountPtr = dboSession.addNew<Account>(Account::EntityPnlAccount);
			accountPtr.modify()->creatorUserPtr = entityPtr->creatorUserPtr;
			accountPtr.modify()->regionPtr = entityPtr->regionPtr;
			accountPtr.modify()->name = tr("EntityIdPnlAccount").arg(entityPtr.id()).toUTF8();

			entityPtr.modify()->pnlAccountPtr = accountPtr;
		}

		t.commit();
	}

	Wt::Dbo::ptr<AccountEntry> AccountsDatabase::createAccountEntry(const Money &amount, Wt::Dbo::ptr<Account> debitAccountPtr, Wt::Dbo::ptr<Account> creditAccountPtr)
	{
		if(!debitAccountPtr || !creditAccountPtr)
			return Wt::Dbo::ptr<AccountEntry>();

		Wt::Dbo::Transaction t(dboSession);
		auto result = dboSession.add(std::unique_ptr<AccountEntry>(new AccountEntry(amount, debitAccountPtr, creditAccountPtr)));
		result.modify()->setCreatedByValues();

		updateAccountBalances(result);
		t.commit();

		return result;
	}

	void AccountsDatabase::updateAccountBalances(Wt::Dbo::ptr<AccountEntry> accountEntryPtr)
	{
		try
		{
			accountEntryPtr->_debitAccountPtr.modify()->_balanceInCents += accountEntryPtr->amount().valueInCents();
			accountEntryPtr->_creditAccountPtr.modify()->_balanceInCents -= accountEntryPtr->amount().valueInCents();
			accountEntryPtr->_debitAccountPtr.flush();
			accountEntryPtr->_creditAccountPtr.flush();
		}
		catch(const Wt::Dbo::StaleObjectException &)
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
			catch(const Wt::Dbo::StaleObjectException &)
			{
				Wt::log("warn") << "AccountsDatabase::updateAccountBalances(): StaleObjectException caught twice";
				accountEntryPtr->_debitAccountPtr.modify()->operator=(debitAccount);
				accountEntryPtr->_creditAccountPtr.modify()->operator=(creditAccount);
				throw;
			}
		}
	}

	void AccountsDatabase::createPendingCycleEntry(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration)
	{
		Wt::Dbo::Transaction t(dboSession);

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

		while(Wt::Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentDt, nextEntryDuration))
		{
			newEntry.modify()->incomeCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->balAccountPtr;
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->pnlAccountPtr;
			newEntry.modify()->type = AccountEntry::UnspecifiedType;
			newEntry.modify()->description.arg(tr("income"));
			if(assignmentCount > 0)
				newEntry.modify()->description += trn("forNClientAssignments", assignmentCount).arg(assignmentCount).toUTF8();

			updateAccountBalances(newEntry);

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	void AccountsDatabase::createPendingCycleEntry(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, const Wt::WDateTime &currentDt, steady_clock::duration *nextEntryDuration)
	{
		Wt::Dbo::Transaction t(dboSession);

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

		while(Wt::Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentDt, nextEntryDuration))
		{
			newEntry.modify()->expenseCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->pnlAccountPtr;
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->balAccountPtr;
			newEntry.modify()->type = AccountEntry::UnspecifiedType;
			newEntry.modify()->description.arg(tr("expense"));
			if(assignmentCount > 0)
				newEntry.modify()->description += trn("forNEmployeeAssignments", assignmentCount).arg(assignmentCount).toUTF8();

			updateAccountBalances(newEntry);

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	Wt::Dbo::ptr<AccountEntry> AccountsDatabase::_createPendingCycleEntry(
		const EntryCycle &cycle,
		Wt::Dbo::ptr<AccountEntry> lastEntryPtr,
		const Wt::WDateTime &currentDt,
		steady_clock::duration *nextEntryDuration)
	{
		//DO NOT ALLOW THESE NULL DATES(in scan query)
		if(cycle.startDate.isNull() || cycle.timestamp.isNull())
			return Wt::Dbo::ptr<AccountEntry>();

		//Do not allow cycles with start dates later than today and creation dt later than now(already filtered, reasserted)
		if(cycle.startDate > currentDt.date() || cycle.timestamp > currentDt)
			return Wt::Dbo::ptr<AccountEntry>();

		//Do not allow cycles with end dates earlier or equal to start date
		if(cycle.endDate.isValid() && cycle.endDate <= cycle.startDate)
			return Wt::Dbo::ptr<AccountEntry>();

		//Do not allow invalid interval or nIntervals
		if(cycle.interval < DailyInterval || cycle.interval > YearlyInterval)
			return Wt::Dbo::ptr<AccountEntry>();
		if(cycle.nIntervals < 1)
			return Wt::Dbo::ptr<AccountEntry>();

		if(lastEntryPtr)
		{
			//Do not allow cycles which have ended and the final entry had already been made(already filtered, reasserted)
			if(cycle.endDate.isValid() && cycle.endDate <= lastEntryPtr->timestamp.date())
				return Wt::Dbo::ptr<AccountEntry>();

			//Do not allow entries with invalid date(in scan query)
			if(lastEntryPtr->timestamp.isNull() || lastEntryPtr->timestamp > currentDt)
				return Wt::Dbo::ptr<AccountEntry>();
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
			steady_clock::duration cycleDuration;
			steady_clock::duration elapsedDuration;

			if(lastEntryPtr)
				previousCyclePeriodDt = lastEntryPtr->timestamp;
			else if(cycle.firstEntryAfterCycle)
				previousCyclePeriodDt = addCycleInterval(Wt::WDateTime(cycle.startDate), cycle.interval, cycle.nIntervals);
			else
			{
				Wt::log("error") << "AccountsDatabase::_createPendingCycleEntry() logic error: lastEntryPtr AND cycle.firstEntryAfterCycle were both false!";
				return Wt::Dbo::ptr<AccountEntry>();
			}

			//Its possible because of cycle.firstEntryAfterCycle
			if(previousCyclePeriodDt > currentDt)
				return Wt::Dbo::ptr<AccountEntry>();

			elapsedDuration = currentDt.toTimePoint() - previousCyclePeriodDt.toTimePoint();
			nextCyclePeriodDt = addCycleInterval(previousCyclePeriodDt, cycle.interval, cycle.nIntervals);
			cycleDuration = nextCyclePeriodDt.toTimePoint() - previousCyclePeriodDt.toTimePoint();

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
					auto remainingDuration = cycleDuration - elapsedDuration;
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
			return Wt::Dbo::ptr<AccountEntry>();

		AccountEntry newEntry;
		if(incompleteDurationEntry)
		{
			int cycleDays = previousCyclePeriodDt.date().daysTo(nextCyclePeriodDt.date());
			int incompleteDays = previousCyclePeriodDt.date().daysTo(cycle.endDate);
			double ratio = static_cast<double>(incompleteDays) / cycleDays;
			newEntry._amountInCents = (cycle.amount() * ratio).valueInCents();

			//Timestamp
			if(!lastEntryPtr && cycle.endDate == currentDt.date())
				newEntry.timestamp = currentDt;
			else if(lastEntryPtr)
				newEntry.timestamp = Wt::WDateTime(cycle.endDate, lastEntryPtr->timestamp.time());
			else
				newEntry.timestamp = Wt::WDateTime(cycle.endDate, Wt::WTime(0, 0));

			//Description
			newEntry.description = tr("IncompleteEntry");
		}
		else
		{
			newEntry._amountInCents = cycle._amountInCents;

			//Timestamp
			if(!lastEntryPtr && nextCyclePeriodDt.date() == currentDt.date())
				newEntry.timestamp = currentDt;
			else
				newEntry.timestamp = nextCyclePeriodDt;

			//Description
			if(finalCompleteEntry)
				newEntry.description = tr("FinalEntry");
			else if(!lastEntryPtr)
				newEntry.description = tr("InitialEntry");
			else
				newEntry.description = tr("RecurringEntry");
		}

		return dboSession.addNew<AccountEntry>(std::move(newEntry));
	}
}