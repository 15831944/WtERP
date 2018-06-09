#include "Dbo/AccountsDatabase.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Application/WServer.h"
#include "Application/WApplication.h"

#include <boost/format.hpp>
#include <stack>
#include <tuple>

namespace ERP
{
	AccountsDatabase::AccountsDatabase(DboSession &serverDboSession)
		: _serverDboSession(serverDboSession)
	{
		_initAccountsInfo();
		
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
	}
	
	void AccountsDatabase::createDefaultAccounts()
	{
		Dbo::Transaction t(dboSession());
		
		Dbo::ptr<ControlAccount> controlAccounts[DefaultControlAccsCount+1];
		for(int i = 0; i < DefaultControlAccsCount; ++i)
		{
			controlAccounts[i] = dboSession().addNew<ControlAccount>(controlAccounts[_ctrlInfo[i].controlAcc]);
			controlAccounts[i].modify()->name = tr(_ctrlInfo[i].prefix + "CAccName").toUTF8();
		}
		
		Dbo::ptr<Account> accounts[DefaultAccountsCount];
		for(int i = 0; i < DefaultAccountsCount; ++i)
		{
			accounts[i] = dboSession().addNew<Account>(controlAccounts[_accInfo[i].controlAcc]);
			accounts[i].modify()->name = tr(_accInfo[i].prefix + "AccName").toUTF8();
		}
		dboSession().flush();
		
		WServer *server = SERVER;
		for(int i = 0; i < DefaultControlAccsCount; ++i)
		{
			std::string configName = _ctrlInfo[i].prefix + "CAccId";
			if(!server->configs().addLongInt(configName, controlAccounts[i].id(), &dboSession()))
				throw std::runtime_error("Error creating config account " + configName + ", ConfigurationsDatabase::addLongInt() returned null");
		}
		for(int i = 0; i < DefaultAccountsCount; ++i)
		{
			std::string configName = _accInfo[i].prefix + "AccId";
			if(!server->configs().addLongInt(configName, accounts[i].id(), &dboSession()))
				throw std::runtime_error("Error creating config account " + configName + ", ConfigurationsDatabase::addLongInt() returned null");
		}
	}
	
	long long AccountsDatabase::getControlAccId(DefaultControlAcc account)
	{
		long long id = SERVER->configs().getLongInt(_ctrlInfo[account].prefix + "CAccId", -1);
		if(id == -1)
			throw std::runtime_error("Could not find control account id config for " + _ctrlInfo[account].prefix + ". Is your data corrupted?");
		return id;
	}
	
	long long AccountsDatabase::getAccountId(DefaultAccount account)
	{
		long long id = SERVER->configs().getLongInt(_accInfo[account].prefix + "AccId", -1);
		if(id == -1)
			throw std::runtime_error("Could not find account id config for " + _accInfo[account].prefix + ". Is your data corrupted?");
		return id;
	}
	
	Dbo::ptr<ControlAccount> AccountsDatabase::loadControlAcc(DefaultControlAcc account, bool loadLazy)
	{
		WServer *server = SERVER;
		AccountInfo info = _ctrlInfo[account];
		
		long long accountId = server->configs().getLongInt(info.prefix + "CAccId", -1);
		if(accountId == -1)
			throw std::runtime_error("Could not find control account id config for " + info.prefix + ". Is your data corrupted?");
		
		Dbo::Transaction t(dboSession());
		if(loadLazy)
			return dboSession().loadLazy<ControlAccount>(accountId);
		else
			return dboSession().load<ControlAccount>(accountId, true);
	}
	
	Dbo::ptr<Account> AccountsDatabase::loadAccount(DefaultAccount account, bool loadLazy)
	{
		WServer *server = SERVER;
		AccountInfo info = _accInfo[account];
		
		long long accountId = server->configs().getLongInt(info.prefix + "AccId", -1);
		if(accountId == -1)
			throw std::runtime_error("Could not find account id config for " + info.prefix + ". Is your data corrupted?");
		
		Dbo::Transaction t(dboSession());
		if(loadLazy)
			return dboSession().loadLazy<Account>(accountId);
		else
			return dboSession().load<Account>(accountId, true);
	}

	void AccountsDatabase::createEntityBalanceAccIfNotFound(Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return;

		Dbo::Transaction t(dboSession());

		if(!entityPtr->balanceAccPtr)
		{
			auto accountPtr = dboSession().addNew<Account>(
				loadControlAcc(AccountsReceivableControlAcc, true),
				loadControlAcc(AccountsPayableControlAcc, true)
			);
			accountPtr.modify()->_creatorUserPtr = entityPtr->creatorUserPtr();
			accountPtr.modify()->_regionPtr = entityPtr->regionPtr();
			accountPtr.modify()->name = tr("EntityBalanceAccName").arg(entityPtr->name).toUTF8();

			entityPtr.modify()->balanceAccPtr = accountPtr;
		}

		t.commit();
	}
	
	void AccountsDatabase::createEntityRecurringIncomesAccIfNotFound(Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return;
		
		Dbo::Transaction t(dboSession());
		
		if(!entityPtr->recurringIncomesAccPtr)
		{
			auto accountPtr = dboSession().addNew<Account>(loadControlAcc(RecurringIncomesControlAcc, true));
			accountPtr.modify()->_creatorUserPtr = entityPtr->creatorUserPtr();
			accountPtr.modify()->_regionPtr = entityPtr->regionPtr();
			accountPtr.modify()->name = tr("EntityRecurringIncomesAccName").arg(entityPtr->name).toUTF8();
			
			entityPtr.modify()->recurringIncomesAccPtr = accountPtr;
		}
		
		t.commit();
	}
	
	void AccountsDatabase::createEntityRecurringExpensesAccIfNotFound(Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return;
		
		Dbo::Transaction t(dboSession());
		
		if(!entityPtr->recurringExpensesAccPtr)
		{
			auto accountPtr = dboSession().addNew<Account>(loadControlAcc(RecurringExpensesControlAcc, true));
			accountPtr.modify()->_creatorUserPtr = entityPtr->creatorUserPtr();
			accountPtr.modify()->_regionPtr = entityPtr->regionPtr();
			accountPtr.modify()->name = tr("EntityRecurringExpensesAccName").arg(entityPtr->name).toUTF8();
			
			entityPtr.modify()->recurringExpensesAccPtr = accountPtr;
		}
		
		t.commit();
	}
	
	void AccountsDatabase::createEntityDoubtfulDebtsAccIfNotFound(Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return;
		
		Dbo::Transaction t(dboSession());
		
		if(!entityPtr->doubtfulDebtsAccPtr)
		{
			auto accountPtr = dboSession().addNew<Account>(loadControlAcc(DoubtfulDebtsControlAcc, true));
			accountPtr.modify()->_creatorUserPtr = entityPtr->creatorUserPtr();
			accountPtr.modify()->_regionPtr = entityPtr->regionPtr();
			accountPtr.modify()->name = tr("EntityDoubtfulDebtsAccName").arg(entityPtr->name).toUTF8();
			
			entityPtr.modify()->doubtfulDebtsAccPtr = accountPtr;
		}
		
		t.commit();
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
		createEntityRecurringIncomesAccIfNotFound(cyclePtr->entityPtr);

		auto assignmentCount = cyclePtr->clientAssignmentCollection.size();

		while(Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentDt, nextEntryDuration))
		{
			newEntry.modify()->incomeCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->balanceAccPtr;
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->recurringIncomesAccPtr;
			newEntry.modify()->description.arg(tr("income"));
			if(assignmentCount > 0)
				newEntry.modify()->description += trn("forNClientAssignments", assignmentCount).arg(assignmentCount).toUTF8();
			
			_updateAccountBalances(newEntry, false, false);

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
		createEntityRecurringExpensesAccIfNotFound(cyclePtr->entityPtr);

		auto assignmentCount = cyclePtr->employeeAssignmentCollection.size();

		while(Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentDt, nextEntryDuration))
		{
			newEntry.modify()->expenseCyclePtr = cyclePtr;
			newEntry.modify()->_debitAccountPtr = cyclePtr->entityPtr->recurringExpensesAccPtr;
			newEntry.modify()->_creditAccountPtr = cyclePtr->entityPtr->balanceAccPtr;
			newEntry.modify()->description.arg(tr("expense"));
			if(assignmentCount > 0)
				newEntry.modify()->description += trn("forNEmployeeAssignments", assignmentCount).arg(assignmentCount).toUTF8();
			
			_updateAccountBalances(newEntry, false, false);

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
	
	Dbo::ptr<AccountEntry> AccountsDatabase::createAccountEntry(const Money &amount, Dbo::ptr<Account> debitAccountPtr, Dbo::ptr<Account> creditAccountPtr)
	{
		if(!debitAccountPtr || !creditAccountPtr)
			return nullptr;
		
		if(amount.valueInCents() <= 0)
			return nullptr;
		
		Dbo::Transaction t(dboSession());
		auto result = dboSession().add(unique_ptr<AccountEntry>(new AccountEntry(amount, debitAccountPtr, creditAccountPtr)));
		result.modify()->setCreatedByValues();
		
		_updateAccountBalances(result, false, false);
		t.commit();
		
		return result;
	}
	
	void AccountsDatabase::_updateAccountBalances(Dbo::ptr<AccountEntry> accountEntryPtr, bool secondAttempt, bool disableFlushing)
	{
		long long value = accountEntryPtr->amount().valueInCents();
		std::vector<pair<Dbo::ptr<ControlAccount>, long long>> controlAccsToUpdate;
		
		try
		{
			//Debit account
			long long prevDebitAccBalance = accountEntryPtr->debitAccountPtr()->balanceInCents();
			accountEntryPtr->_debitAccountPtr.modify()->_balanceInCents += value;
			long long newDebitAccBalance = accountEntryPtr->debitAccountPtr()->balanceInCents();
			
			//Debit account's control account
			if(!accountEntryPtr->debitAccountPtr()->creditControlAccountPtr()) //If control accounts are not divided
			{
				//Then simply update the only control account
				accountEntryPtr->debitAccountPtr()->controlAccountPtr().modify()->_balanceInCents += value;
				controlAccsToUpdate.emplace_back(accountEntryPtr->debitAccountPtr()->controlAccountPtr()->parentPtr(), value);
			}
			else //Otherwise
			{
				if(newDebitAccBalance > 0) //If the new balance is in debit
				{
					if(prevDebitAccBalance < 0) //and if previously it was in credit
					{
						//We remove the effect of this account from the credit control account
						accountEntryPtr->debitAccountPtr()->creditControlAccountPtr().modify()->_balanceInCents += value - newDebitAccBalance;
						controlAccsToUpdate.emplace_back(accountEntryPtr->debitAccountPtr()->creditControlAccountPtr()->parentPtr(), value);
						//And add our new balance to switch over to affecting the debit control account
						accountEntryPtr->debitAccountPtr()->controlAccountPtr().modify()->_balanceInCents += newDebitAccBalance;
						controlAccsToUpdate.emplace_back(accountEntryPtr->debitAccountPtr()->controlAccountPtr()->parentPtr(), value);
					}
					else //but if previously the balance was either 0 or already in debit
					{
						//we just add to the debit control acc
						accountEntryPtr->debitAccountPtr()->controlAccountPtr().modify()->_balanceInCents += value;
						controlAccsToUpdate.emplace_back(accountEntryPtr->debitAccountPtr()->controlAccountPtr()->parentPtr(), value);
					}
				} //If the new balance is in credit, we can assume that the prev balance was also in credit
				else
				{
					//And we just reduce our credit control account balance
					accountEntryPtr->debitAccountPtr()->creditControlAccountPtr().modify()->_balanceInCents += value;
					controlAccsToUpdate.emplace_back(accountEntryPtr->debitAccountPtr()->creditControlAccountPtr()->parentPtr(), value);
				}
			}
			
			//Credit account
			long long prevCreditAccBalance = accountEntryPtr->_creditAccountPtr->balanceInCents();
			accountEntryPtr->_creditAccountPtr.modify()->_balanceInCents -= value;
			long long newCreditAccBalance = accountEntryPtr->_creditAccountPtr->balanceInCents();
			
			//Credit account's control account
			if(!accountEntryPtr->creditAccountPtr()->creditControlAccountPtr()) //If control accounts are not divided
			{
				//Then simply update the only control account
				accountEntryPtr->creditAccountPtr()->controlAccountPtr().modify()->_balanceInCents -= value;
				controlAccsToUpdate.emplace_back(accountEntryPtr->creditAccountPtr()->controlAccountPtr()->parentPtr(), -value);
			}
			else //Otherwise
			{
				if(newCreditAccBalance < 0) //If the new balance is in credit
				{
					if(prevCreditAccBalance > 0) //and if previously it was in debit
					{
						//We remove the effect of this account from the debit control account
						accountEntryPtr->creditAccountPtr()->controlAccountPtr().modify()->_balanceInCents -= value - newCreditAccBalance;
						controlAccsToUpdate.emplace_back(accountEntryPtr->creditAccountPtr()->controlAccountPtr()->parentPtr(), -value);
						//And subtract our new balance to switch over to affecting the credit control account
						accountEntryPtr->creditAccountPtr()->creditControlAccountPtr().modify()->_balanceInCents -= newCreditAccBalance;
						controlAccsToUpdate.emplace_back(accountEntryPtr->creditAccountPtr()->creditControlAccountPtr()->parentPtr(), -value);
					}
					else //but if previously the balance was either 0 or already in credit
					{
						//we just subtract the credit control acc
						accountEntryPtr->creditAccountPtr()->creditControlAccountPtr().modify()->_balanceInCents -= value;
						controlAccsToUpdate.emplace_back(accountEntryPtr->creditAccountPtr()->creditControlAccountPtr()->parentPtr(), -value);
					}
				} //If the new balance is in debit, we can assume that the prev balance was also in debit
				else
				{
					//And we just reduce our debit control account balance
					accountEntryPtr->creditAccountPtr()->controlAccountPtr().modify()->_balanceInCents -= value;
					controlAccsToUpdate.emplace_back(accountEntryPtr->creditAccountPtr()->controlAccountPtr()->parentPtr(), -value);
				}
			}
			
			if(!disableFlushing)
			{
				accountEntryPtr->debitAccountPtr()->creditControlAccountPtr().flush();
				accountEntryPtr->debitAccountPtr()->controlAccountPtr().flush();
				accountEntryPtr->debitAccountPtr().flush();
				accountEntryPtr->creditAccountPtr()->creditControlAccountPtr().flush();
				accountEntryPtr->creditAccountPtr()->controlAccountPtr().flush();
				accountEntryPtr->creditAccountPtr().flush();
			}
		}
		catch(const Dbo::StaleObjectException &)
		{
			accountEntryPtr->debitAccountPtr()->creditControlAccountPtr().reread();
			accountEntryPtr->debitAccountPtr()->controlAccountPtr().reread();
			accountEntryPtr->debitAccountPtr().reread();
			accountEntryPtr->creditAccountPtr()->creditControlAccountPtr().reread();
			accountEntryPtr->creditAccountPtr()->controlAccountPtr().reread();
			accountEntryPtr->creditAccountPtr().reread();
			if(secondAttempt)
			{
				Wt::log("warn") << "AccountsDatabase::_updateAccountBalances(): StaleObjectException caught twice";
				throw;
			}
			else
				_updateAccountBalances(accountEntryPtr, true, disableFlushing);
		}
		
		for(const auto &v : controlAccsToUpdate)
			_updateControlAccountBalances(v.first, v.second, disableFlushing);
	}
	
	void AccountsDatabase::_updateControlAccountBalances(Dbo::ptr<ControlAccount> controlAccPtr, long long valueInCents, bool disableFlushing)
	{
		if(!controlAccPtr)
			return;
		
		try
		{
			controlAccPtr.modify()->_balanceInCents += valueInCents;
			
			if(!disableFlushing)
				controlAccPtr.flush();
		}
		catch(const Dbo::StaleObjectException &)
		{
			controlAccPtr.reread();
			
			try
			{
				controlAccPtr.modify()->_balanceInCents += valueInCents;
				controlAccPtr.flush();
			}
			catch(const Dbo::StaleObjectException &)
			{
				Wt::log("warn") << "AccountsDatabase::_updateControlAccountBalances(): StaleObjectException caught twice";
				throw;
			}
		}
		
		_updateControlAccountBalances(controlAccPtr->parentPtr(), valueInCents, disableFlushing);
	}
	
	void AccountsDatabase::_recalculateAccountBalances()
	{
		Dbo::Transaction t(dboSession());
		size_t i;
		
		dboSession().execute("UPDATE " + Account::tStr() + " SET balance = 0, \"version\" = \"version\" + 1");
		dboSession().execute("UPDATE " + ControlAccount::tStr() + " SET balance = 0, \"version\" = \"version\" + 1");
		
		//Load all accounts and control accounts
		ControlAccountCollection controlAccountCollection = dboSession().find<ControlAccount>();
		std::vector<Dbo::ptr<ControlAccount>> controlAccounts(controlAccountCollection.size());
		i = 0;
		for(const auto &ptr : controlAccountCollection)
			controlAccounts[i++] = ptr;
		
		AccountCollection accountCollection = dboSession().find<Account>();
		std::vector<Dbo::ptr<Account>> accounts(accountCollection.size());
		i = 0;
		for(const auto &ptr : controlAccountCollection)
			controlAccounts[i++] = ptr;
		
		AccountEntryCollection entriesCollection = dboSession().find<AccountEntry>();
		for(const auto &entry : entriesCollection)
			_updateAccountBalances(entry, false, true);
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