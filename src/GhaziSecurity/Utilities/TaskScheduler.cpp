#include "Utilities/TaskScheduler.h"
#include "Application/WServer.h"
#include "Dbo/Dbos.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "Widgets/EntityView.h"

#include <Wt/WIOService>
#include <Wt/Dbo/SqlConnectionPool>

#include <boost/format.hpp>

namespace GS
{

	TaskScheduler::TaskScheduler(WServer *server, Wt::Dbo::Session &session)
		: _server(server), dboSession(session), _entitiesDatabase(dboSession), _accountsDatabase(dboSession)
	{
		//Recalculate balance update query
		{
			Wt::Dbo::Transaction t(dboSession);
			_recalculateBalanceCall = new Wt::Dbo::Call(dboSession.execute("UPDATE " + std::string(Account::tableName()) + " acc SET balance = "
				"COALESCE((SELECT SUM(dE.amount) FROM " + AccountEntry::tableName() + " dE WHERE dE.debit_account_id = acc.id), 0) - COALESCE((SELECT SUM(cE.amount) FROM " + AccountEntry::tableName() + " cE WHERE cE.credit_account_id = acc.id), 0)"
				", \"version\" = \"version\" + 1"));
			t.rollback();
		}

		//Abnormal account check query
		{
			_accountCheckAbnormal = dboSession.query<int>("SELECT count(*) FROM " + std::string(Account::tableName()) + " acc "
				"INNER JOIN " + Entity::tableName() + " balE ON (balE.bal_account_id = acc.id) "
				"INNER JOIN " + Entity::tableName() + " pnlE ON (pnlE.pnl_account_id = acc.id)");
		}

		//Abnormal entry check query
		{
			_entryCheckAbnormal = dboSession.find<AccountEntry>().where("timestamp IS null OR amount < 0");
		}

		//Abnormal entry cycle check queries
		{
			std::string condition = "creationDt IS null OR startDate IS null OR \"interval\" < 0 OR \"interval\" > ? OR nIntervals < 1 OR (endDate IS NOT null AND endDate <= startDate)";
			_incomeCycleCheckAbnormal = dboSession.find<IncomeCycle>().where(condition).bind(YearlyInterval);
			_expenseCycleCheckAbnormal = dboSession.find<ExpenseCycle>().where(condition).bind(YearlyInterval);
		}

		//Entry cycle queries
		{
			std::string query = std::string("SELECT cycle, lastEntry FROM %1% cycle ") +
				"LEFT JOIN " + AccountEntry::tableName() + " lastEntry ON (cycle.id = lastEntry.%2%) "
				"LEFT JOIN " + AccountEntry::tableName() + " e2 ON (cycle.id = e2.%2% AND (lastEntry.timestamp < e2.timestamp OR lastEntry.timestamp = e2.timestamp AND lastEntry.id < e2.id)) "
				"WHERE (e2.id IS null) "
				"AND (cycle.startDate <= ? AND cycle.creationDt <= ?) "
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

	TaskScheduler::~TaskScheduler()
	{
		delete _recalculateBalanceCall;
	}

	void TaskScheduler::createDefaultAccounts(bool scheduleNext)
	{
		try
		{
			_accountsDatabase.findOrCreateCashAccount();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::createDefaultAccounts(): Error: " << e.what();
			if(_isConstructing)
				throw e;
		}

		//Repeat every 24 hours
		if(scheduleNext)
			_server->ioService().schedule(static_cast<int>(boost::posix_time::hours(24).total_milliseconds()),
				boost::bind(&TaskScheduler::createDefaultAccounts, this, true));
	}

	void TaskScheduler::recalculateAccountBalances(bool scheduleNext)
	{
		Wt::Dbo::Transaction t(dboSession);
		try
		{
			_recalculateBalanceCall->run();
			t.commit();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::recalculateAccountBalances(): Error: " << e.what();
			if(_isConstructing)
				throw e;
		}

		//Repeat
		if(scheduleNext)
			_server->ioService().schedule(static_cast<int>(boost::posix_time::hours(24).total_milliseconds()),
				boost::bind(&TaskScheduler::recalculateAccountBalances, this, true));
	}

	void TaskScheduler::createPendingCycleEntries(bool scheduleNext)
	{
		_createPendingCycleEntries(scheduleNext);
	}

	void TaskScheduler::_createPendingCycleEntries(bool scheduleNext)
	{
		Wt::Dbo::Transaction t(dboSession);
		boost::posix_time::time_duration nextEntryDuration = boost::posix_time::hours(6);

		try
		{
			auto currentPTime = boost::posix_time::microsec_clock::local_time();
			Wt::WDateTime currentDt(currentPTime);

			//Income cycle
			_incomeCycleQuery.reset();
			_incomeCycleQuery.bind(currentDt).bind(currentDt).bind(currentDt);

			IncomeTupleCollection incomeCollection = _incomeCycleQuery;
			for(auto &tuple : incomeCollection)
			{
				Wt::Dbo::ptr<IncomeCycle> cyclePtr;
				Wt::Dbo::ptr<AccountEntry> lastEntryPtr;
				boost::tie(cyclePtr, lastEntryPtr) = tuple;

				_accountsDatabase.createPendingCycleEntry(cyclePtr, lastEntryPtr, currentPTime, &nextEntryDuration);
			}

			//Expense cycle
			_expenseCycleQuery.reset();
			_expenseCycleQuery.bind(currentDt).bind(currentDt).bind(currentDt);

			ExpenseTupleCollection expenseCollection = _expenseCycleQuery;
			for(auto &tuple : expenseCollection)
			{
				Wt::Dbo::ptr<ExpenseCycle> cyclePtr;
				Wt::Dbo::ptr<AccountEntry> lastEntryPtr;
				boost::tie(cyclePtr, lastEntryPtr) = tuple;

				_accountsDatabase.createPendingCycleEntry(cyclePtr, lastEntryPtr, currentPTime, &nextEntryDuration);
			}

			t.commit();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::createPendingCycleEntries(): Error: " << e.what();
			if(_isConstructing)
				throw e;
		}

		//Repeat
		if(scheduleNext)
			_server->ioService().schedule(static_cast<int>(nextEntryDuration.total_milliseconds()),
				boost::bind(&TaskScheduler::createPendingCycleEntries, this, true));
	}

	void TaskScheduler::checkAbnormalRecords(bool scheduleNext)
	{
		Wt::Dbo::Transaction t(dboSession);
		try
		{
			{
				int abnormalRecords = _accountCheckAbnormal;
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal Account records were found";
			}
			{
				AccountEntryCollection collection = _entryCheckAbnormal;
				int abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal AccountEntry records were found";
			}
			{
				IncomeCycleCollection collection = _incomeCycleCheckAbnormal;
				int abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal IncomeCycle records were found";
			}
			{
				ExpenseCycleCollection collection = _expenseCycleCheckAbnormal;
				int abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal ExpenseCycle records were found";
			}
			t.commit();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::checkAbnormalRecords(): Error: " << e.what();
			if(_isConstructing)
				throw e;
		}

		//Repeat every 24 hours
		if(scheduleNext)
			_server->ioService().schedule(static_cast<int>(boost::posix_time::hours(24).total_milliseconds()),
				boost::bind(&TaskScheduler::createPendingCycleEntries, this, true));
	}

// 	void TaskScheduler::createSelfEntityAndAccount(bool scheduleNext)
// 	{
// 		try
// 		{
// 			_accountsDatabase.findOrCreateSelfAccount();
// 		}
// 		catch(const std::exception &e)
// 		{
// 			Wt::log("error") << "TaskScheduler::createSelfEntityAndAccount(): Error: " << e.what();
// 			if(_isConstructing)
// 				throw e;
// 		}
// 
// 		//Repeat every 24 hours
// 		if(scheduleNext)
// 			_server->ioService().schedule(static_cast<int>(boost::posix_time::hours(24).total_milliseconds()),
// 				boost::bind(&TaskScheduler::createSelfEntityAndAccount, this, true));
// 	}

}
