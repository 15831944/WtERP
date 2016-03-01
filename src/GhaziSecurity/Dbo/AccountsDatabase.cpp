#include "Dbo/AccountsDatabase.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Application/WServer.h"
#include <Wt/WLogger>

namespace GS
{

	void AccountsDatabase::createEntityAccountsIfNotFound(Wt::Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return;

		Wt::Dbo::Transaction t(dboSession);

		if(!entityPtr->balAccountPtr)
			entityPtr.modify()->balAccountPtr = dboSession.add(new Account(Account::EntityBalanceAccount));
		if(!entityPtr->pnlAccountPtr)
			entityPtr.modify()->pnlAccountPtr = dboSession.add(new Account(Account::EntityPnlAccount));

		t.commit();
	}

	Wt::Dbo::ptr<AccountEntry> AccountsDatabase::createAccountEntry(double amount, Wt::Dbo::ptr<Account> debitAccountPtr, Wt::Dbo::ptr<Account> creditAccountPtr)
	{
		if(!debitAccountPtr || !creditAccountPtr)
			return Wt::Dbo::ptr<AccountEntry>();

		Wt::Dbo::Transaction t(dboSession);
		auto result = dboSession.add(new AccountEntry(amount, debitAccountPtr, creditAccountPtr));

		updateAccountBalances(result);
		t.commit();

		return result;
	}

	void AccountsDatabase::updateAccountBalances(Wt::Dbo::ptr<AccountEntry> accountEntryPtr)
	{
		accountEntryPtr->_debitAccountPtr.modify()->_balance += accountEntryPtr->amount();
		accountEntryPtr->_creditAccountPtr.modify()->_balance -= accountEntryPtr->amount();
	}

	void AccountsDatabase::createPendingCycleEntry(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime, boost::posix_time::time_duration *nextEntryDuration)
	{
		if(!cyclePtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr was null");
		if(!cyclePtr->entityPtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr->entityPtr was null");
		if(currentPTime.is_special())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): currentPTime was invalid");

		Wt::Dbo::Transaction t(dboSession);

		if(lastEntryPtr && lastEntryPtr->incomeCyclePtr && lastEntryPtr->incomeCyclePtr.id() != cyclePtr.id())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): lastEntryPtr->incomeCyclePtr != cyclePtr");

		createEntityAccountsIfNotFound(cyclePtr->entityPtr);

		while(Wt::Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentPTime, nextEntryDuration))
		{
			newEntry.modify()->incomeCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->balAccountPtr;
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->pnlAccountPtr;
			newEntry.modify()->type = AccountEntry::UnspecifiedType;
			updateAccountBalances(newEntry);

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	void AccountsDatabase::createPendingCycleEntry(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime, boost::posix_time::time_duration *nextEntryDuration)
	{
		if(!cyclePtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr was null");
		if(!cyclePtr->entityPtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr->entityPtr was null");
		if(currentPTime.is_special())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): currentPTime was invalid");
		
		Wt::Dbo::Transaction t(dboSession);

		if(lastEntryPtr && lastEntryPtr->expenseCyclePtr && lastEntryPtr->expenseCyclePtr.id() != cyclePtr.id())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): lastEntryPtr->expenseCyclePtr != cyclePtr");

		createEntityAccountsIfNotFound(cyclePtr->entityPtr);

		while(Wt::Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentPTime, nextEntryDuration))
		{
			newEntry.modify()->expenseCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->pnlAccountPtr;
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->balAccountPtr;
			newEntry.modify()->type = AccountEntry::UnspecifiedType;
			updateAccountBalances(newEntry);

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	Wt::Dbo::ptr<AccountEntry> AccountsDatabase::_createPendingCycleEntry(
		const EntryCycle &cycle,
		Wt::Dbo::ptr<AccountEntry> lastEntryPtr,
		boost::posix_time::ptime currentPTime,
		boost::posix_time::time_duration *nextEntryDuration)
	{
		Wt::WDateTime currentDt(currentPTime);

		//DO NOT ALLOW THESE NULL DATES(in scan query)
		if(cycle.startDate.isNull() || cycle.creationDt.isNull())
			return Wt::Dbo::ptr<AccountEntry>();

		//Do not allow cycles with start dates later than today and creation dt later than now(already filtered, yet asserted)
		if(cycle.startDate > currentDt.date() || cycle.creationDt > currentDt)
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
			//Do not allow cycles which have ended and the final entry had already been made(already filtered, yet asserted)
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

		boost::posix_time::ptime previousCyclePeriodPTime; //either startDate or lastEntry timestamp
		boost::posix_time::ptime nextCyclePeriodPTime;
		boost::posix_time::time_duration cycleDuration;
		boost::posix_time::time_duration elapsedDuration;

		if(checkElapsedDuration)
		{
			if(lastEntryPtr)
			{
				previousCyclePeriodPTime = lastEntryPtr->timestamp.toPosixTime();
				elapsedDuration = currentPTime - previousCyclePeriodPTime;
			}
			else if(cycle.firstEntryAfterCycle)
			{
				previousCyclePeriodPTime = boost::posix_time::ptime(cycle.startDate.toGregorianDate());
				elapsedDuration = currentPTime - previousCyclePeriodPTime;
			}
			else
			{
				Wt::log("error") << "AccountsDatabase::_createPendingCycleEntry() logic error: lastEntryPtr AND cycle.firstEntryAfterCycle were both false!";
				return Wt::Dbo::ptr<AccountEntry>();
			}

			nextCyclePeriodPTime = addInterval(previousCyclePeriodPTime, cycle.interval, cycle.nIntervals);

			cycleDuration = nextCyclePeriodPTime - previousCyclePeriodPTime;
			if(elapsedDuration >= cycleDuration) //complete cycle
			{
				createEntry = true;

				//Next entry duration
				if(nextEntryDuration && (nextEntryDuration->is_special() || *nextEntryDuration > cycleDuration))
					*nextEntryDuration = cycleDuration;
			}
			else
			{ //incomplete duration
				//Create incomplete duration entry ONLY IF the cycle has ended
				//no entry has been made after the cycle.endDt: IS ASSERTED previously
				if(cycle.endDate.isValid() && currentDt.date() >= cycle.endDate)
				{
					createEntry = true;
					incompleteDurationEntry = true;
				}
				else
				{
					createEntry = false;

					//Next entry duration
					auto remainingDuration = cycleDuration - elapsedDuration;
					if(nextEntryDuration && (nextEntryDuration->is_special() || *nextEntryDuration > remainingDuration))
						*nextEntryDuration = remainingDuration;
				}
			}
		}
		else
		{
			nextCyclePeriodPTime = boost::posix_time::ptime(cycle.startDate.toGregorianDate());
			createEntry = true;
		}

		if(!createEntry)
			return Wt::Dbo::ptr<AccountEntry>();

		AccountEntry newEntry;
		if(incompleteDurationEntry)
		{
			boost::gregorian::days cycleDays = nextCyclePeriodPTime.date() - previousCyclePeriodPTime.date();
			boost::gregorian::days incompleteDays = cycle.endDate.toGregorianDate() - previousCyclePeriodPTime.date();
			double ratio = static_cast<double>(incompleteDays.days()) / cycleDays.days();
			newEntry._amount = cycle.amount * ratio;
			newEntry.timestamp = Wt::WDateTime(cycle.endDate, Wt::WTime(0, 0));
		}
		else
		{
			newEntry._amount = cycle.amount;
			newEntry.timestamp = Wt::WDateTime(nextCyclePeriodPTime);
		}

		return dboSession.add(new AccountEntry(std::move(newEntry)));
	}

	boost::posix_time::ptime AccountsDatabase::addInterval(boost::posix_time::ptime pTime, CycleInterval interval, int nIntervals) const
	{
		if(interval == DailyInterval)
			pTime += boost::gregorian::days(nIntervals);
		else if(interval == WeeklyInterval)
			pTime += boost::gregorian::weeks(nIntervals);
		else if(interval == MonthlyInterval)
			pTime += boost::gregorian::months(nIntervals);
		else if(interval == YearlyInterval)
			pTime += boost::gregorian::years(nIntervals);

		return pTime;
	}

// 	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateIncomeExpenseAccount(Wt::Dbo::ptr<IncomeCycle> cyclePtr)
// 	{
// 		Wt::Dbo::Transaction t(dboSession);
// 
// 		if(cyclePtr->servicePtr)
// 		{
// 			if(!cyclePtr->servicePtr->accountPtr)
// 				cyclePtr.modify()->servicePtr.modify()->accountPtr = dboSession.add(new Account());
// 
// 			return cyclePtr->servicePtr->accountPtr;
// 		}
// 		return findOrCreateGeneralIncomeAccount();
// 	}
// 
// 	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateIncomeExpenseAccount(Wt::Dbo::ptr<ExpenseCycle> cyclePtr)
// 	{
// 		Wt::Dbo::Transaction t(dboSession);
// 
// 		if(cyclePtr->positionPtr)
// 		{
// 			if(!cyclePtr->positionPtr->accountPtr)
// 				cyclePtr.modify()->positionPtr.modify()->accountPtr = dboSession.add(new Account());
// 
// 			return cyclePtr->positionPtr->accountPtr;
// 		}
// 		return findOrCreateGeneralExpenseAccount();
// 	}
// 
// 	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateGeneralIncomeAccount()
// 	{
// 		WServer *server = SERVER;
// 
// 		Wt::Dbo::Transaction t(dboSession);
// 		Wt::Dbo::ptr<Account> accountPtr;
// 
// 		//Config found
// 		if(server->configs()->getLongIntPtr("GeneralIncomeAccountId"))
// 		{
// 			long long entityId = server->configs()->getLongInt("GeneralIncomeAccountId", -1);
// 			if(entityId != -1)
// 				accountPtr = dboSession.find<Account>().where("id = ?").bind(entityId);
// 		}
// 
// 		//Create account if not found
// 		if(!accountPtr)
// 		{
// 			accountPtr = dboSession.add(new Account());
// 
// 			if(!server->configs()->addLongInt("GeneralIncomeAccountId", accountPtr.id(), &dboSession))
// 				throw std::runtime_error("Error creating GeneralIncomeAccountId config, ConfigurationsDatabase::addLongInt() returned null");
// 		}
// 
// 		t.commit();
// 		return accountPtr;
// 	}
// 
// 	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateGeneralExpenseAccount()
// 	{
// 		WServer *server = SERVER;
// 
// 		Wt::Dbo::Transaction t(dboSession);
// 		Wt::Dbo::ptr<Account> accountPtr;
// 
// 		//Config found
// 		if(server->configs()->getLongIntPtr("GeneralExpenseAccountId"))
// 		{
// 			long long entityId = server->configs()->getLongInt("GeneralExpenseAccountId", -1);
// 			if(entityId != -1)
// 				accountPtr = dboSession.find<Account>().where("id = ?").bind(entityId);
// 		}
// 
// 		//Create account if not found
// 		if(!accountPtr)
// 		{
// 			accountPtr = dboSession.add(new Account());
// 
// 			if(!server->configs()->addLongInt("GeneralExpenseAccountId", accountPtr.id(), &dboSession))
// 				throw std::runtime_error("Error creating GeneralExpenseAccountId config, ConfigurationsDatabase::addLongInt() returned null");
// 		}
// 
// 		t.commit();
// 		return accountPtr;
// 	}
// 
// 	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateSelfAccount()
// 	{
// 		Wt::Dbo::Transaction t(dboSession);
// 		auto selfEntity = _entitiesDatabase.findOrCreateSelfEntity();
// 		if(!selfEntity)
// 			return Wt::Dbo::ptr<Account>();
// 
// 		Wt::Dbo::ptr<Account> selfAccount = selfEntity->accountWPtr;
// 		if(!selfAccount)
// 		{
// 			Wt::log("warn") << "Self account was not found, creating self account";
// 			selfAccount = dboSession.add(new Account());
// 			initEntityAccountValues(selfAccount, selfEntity);
// 		}
// 
// 		t.commit();
// 		return selfAccount;
// 	}
// 
// 	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateEntityAccount(Wt::Dbo::ptr<Entity> entityPtr)
// 	{
// 		if(!entityPtr)
// 			return Wt::Dbo::ptr<Account>();
// 
// 		Wt::Dbo::Transaction t(dboSession);
// 		Wt::Dbo::ptr<Account> accountPtr = entityPtr->accountWPtr.query();
// 		if(!accountPtr)
// 		{
// 			accountPtr = dboSession.add(new Account());
// 			initEntityAccountValues(accountPtr, entityPtr);
// 		}
// 		t.commit();
// 		return accountPtr;
// 	}
// 
// 	void AccountsDatabase::initEntityAccountValues(Wt::Dbo::ptr<Account> accountPtr, Wt::Dbo::ptr<Entity> entityPtr)
// 	{
// 		accountPtr.modify()->entityPtr = entityPtr;
// 	}

}