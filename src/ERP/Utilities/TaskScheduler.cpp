#include "Utilities/TaskScheduler.h"
#include "Application/WServer.h"
#include "Dbo/Dbos.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Widgets/EntityView.h"

#include <Wt/WIOService.h>
#include <Wt/Dbo/SqlConnectionPool.h>

#include <boost/format.hpp>

namespace ERP
{

	TaskScheduler::TaskScheduler(WServer *server, Dbo::Session &session)
		: dboSession(session), _server(server), _entitiesDatabase(dboSession), _accountsDatabase(dboSession)
	{
		Dbo::Transaction t(dboSession);
		//Recalculate balance update query
		{
			_recalculateBalanceCall = make_unique<Dbo::Call>(dboSession.execute("UPDATE " + std::string(Account::tableName()) + " SET balance = "
				"COALESCE((SELECT SUM(dE.amount) FROM " + AccountEntry::tableName() + " dE WHERE dE.debit_account_id = " + Account::tableName() + ".id), 0) - COALESCE((SELECT SUM(cE.amount) FROM " + AccountEntry::tableName() + " cE WHERE cE.credit_account_id = " + Account::tableName() + ".id), 0)"
				", \"version\" = \"version\" + 1"));
			t.rollback();
		}

		//Abnormal account check query
		{
			_accountCheckAbnormal = dboSession.query<long long>("SELECT COUNT(*) FROM " + std::string(Account::tableName()) + " acc "
				"INNER JOIN " + Entity::tableName() + " balE ON (balE.bal_account_id = acc.id) "
				"INNER JOIN " + Entity::tableName() + " pnlE ON (pnlE.pnl_account_id = acc.id)");
		}

		//Abnormal entry check query
		{
			_entryCheckAbnormal = dboSession.find<AccountEntry>().where("timestamp IS null OR amount < 0");
		}

		//Abnormal entry cycle check queries
		{
			std::string condition = "timestamp IS null OR startDate IS null OR \"interval\" < 0 OR \"interval\" > ? OR nIntervals < 1 OR (endDate IS NOT null AND endDate <= startDate)";
			_incomeCycleCheckAbnormal = dboSession.find<IncomeCycle>().where(condition).bind(YearlyInterval);
			_expenseCycleCheckAbnormal = dboSession.find<ExpenseCycle>().where(condition).bind(YearlyInterval);
		}

		//Abnormal location query
		{
			_locationCheckAbnormal = dboSession.query<long long>(
				"SELECT COUNT(*) FROM " + std::string(Location::tableName()) + " l "
				"INNER JOIN " + City::tableName() + " city ON (city.id = l.city_id)"
				).where("city.country_code <> l.country_code");
		}

		//Abnormal attendance query
		{
			_attendanceCheckAbnormal = dboSession.find<AttendanceEntry>().where("timestampOut IS NOT null AND timestampOut < timestampIn");
		}

		//Entry cycle queries
		{
			std::string query = std::string("SELECT cycle, lastEntry FROM %1% cycle ") +
				"LEFT JOIN " + AccountEntry::tableName() + " lastEntry ON (cycle.id = lastEntry.%2%) "
				"LEFT JOIN " + AccountEntry::tableName() + " e2 ON (cycle.id = e2.%2% AND (lastEntry.timestamp < e2.timestamp OR lastEntry.timestamp = e2.timestamp AND lastEntry.id < e2.id)) "
				"WHERE (e2.id IS null) "
				"AND (cycle.startDate <= ? AND cycle.timestamp <= ?) "
				"AND (lastEntry.id IS null OR cycle.endDate IS null OR (lastEntry.timestamp <= ? AND cycle.endDate > lastEntry.timestamp AND cycle.endDate > cycle.startDate))";
			boost::format fString(query);

			fString % IncomeCycle::tableName() % "incomecycle_id";
			_incomeCycleQuery = dboSession.query<IncomeCycleTuple>(fString.str());

			fString % ExpenseCycle::tableName() % "expensecycle_id";
			_expenseCycleQuery = dboSession.query<ExpenseCycleTuple>(fString.str());
		}

		_isConstructing = true;
		//createSelfEntityAndAccount(true);
		createDefaultAccounts(true);
		recalculateAccountBalances(true);
		checkAbnormalRecords(true);
		createPendingCycleEntries(true);
		_isConstructing = false;
	}

	void TaskScheduler::createDefaultAccounts(bool scheduleNext)
	{
		Wt::log("gs-info") << "TaskScheduler: Checking default accounts in database";
		try
		{
			_accountsDatabase.findOrCreateCashAccount();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::createDefaultAccounts(): Error: " << e.what();
			if(_isConstructing)
				throw;
		}

		//Repeat every 24 hours
		if(scheduleNext)
			_server->ioService().schedule(hours(24), std::bind(&TaskScheduler::createDefaultAccounts, this, true));
	}

	void TaskScheduler::recalculateAccountBalances(bool scheduleNext)
	{
		Wt::log("gs-info") << "TaskScheduler: Recalculating account balances";
		Dbo::Transaction t(dboSession);
		try
		{
			_recalculateBalanceCall->run();
			t.commit();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::recalculateAccountBalances(): Error: " << e.what();
			if(_isConstructing)
				throw;
		}

		//Repeat
		if(scheduleNext)
			_server->ioService().schedule(hours(24), std::bind(&TaskScheduler::recalculateAccountBalances, this, true));
	}

	void TaskScheduler::createPendingCycleEntries(bool scheduleNext)
	{
		Wt::log("gs-info") << "TaskScheduler: Checking for pending EntryCycle entries";
		steady_clock::duration nextEntryDuration = _createPendingCycleEntries(scheduleNext);

		//Repeat
		if(scheduleNext)
			_server->ioService().schedule(nextEntryDuration, std::bind(&TaskScheduler::createPendingCycleEntries, this, true));
	}

	steady_clock::duration TaskScheduler::_createPendingCycleEntries(bool scheduleNext)
	{
		Dbo::Transaction t(dboSession);
		steady_clock::duration nextEntryDuration = hours(6);

		try
		{
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

				_accountsDatabase.createPendingCycleEntry(cyclePtr, lastEntryPtr, currentDt, &nextEntryDuration);
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

				_accountsDatabase.createPendingCycleEntry(cyclePtr, lastEntryPtr, currentDt, &nextEntryDuration);
			}

			t.commit();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::createPendingCycleEntries(): Error: " << e.what();
			if(_isConstructing)
				throw;
		}

		return nextEntryDuration;
	}

	void TaskScheduler::checkAbnormalRecords(bool scheduleNext)
	{
		Wt::log("gs-info") << "TaskScheduler: Checking for abnormal database entries";
		Dbo::Transaction t(dboSession);
		try
		{
			{
				long long abnormalRecords = _accountCheckAbnormal;
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal Account records were found";
			}
			{
				AccountEntryCollection collection = _entryCheckAbnormal;
				size_t abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal AccountEntry records were found";
			}
			{
				IncomeCycleCollection collection = _incomeCycleCheckAbnormal;
				size_t abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal IncomeCycle records were found";
			}
			{
				ExpenseCycleCollection collection = _expenseCycleCheckAbnormal;
				size_t abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal ExpenseCycle records were found";
			}
			{
				long long abnormalRecords = _locationCheckAbnormal;
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal Location records were found";
			}
			{
				AttendanceEntryCollection collection = _attendanceCheckAbnormal;
				size_t abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal AttendanceEntry records were found";
			}
			t.commit();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::checkAbnormalRecords(): Error: " << e.what();
			if(_isConstructing)
				throw;
		}

		//Repeat every 24 hours
		if(scheduleNext)
			_server->ioService().schedule(hours(24), std::bind(&TaskScheduler::checkAbnormalRecords, this, true));
	}

}
