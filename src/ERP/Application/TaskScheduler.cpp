#include "Application/TaskScheduler.h"
#include "Application/WServer.h"
#include "Dbo/Dbos.h"
#include "Dbo/ConfigurationsDatabase.h"
#include "ModelView/EntityView.h"

#include <Wt/WIOService.h>
#include <Wt/Dbo/SqlConnectionPool.h>

namespace ERP
{

	TaskScheduler::TaskScheduler(WServer *server, DboSession &session)
		: dboSession(session), _server(server)
	{
		//Abnormal location query
		{
			_locationCheckAbnormal = dboSession.query<long long>(
				"SELECT COUNT(*) FROM " + Location::tStr() + " l "
				"INNER JOIN " + City::tStr() + " city ON (city.id = l.city_id)"
				).where("city.country_code <> l.country_code");
		}

		_isConstructing = true;
		recalculateAccountBalances();
		checkAbnormalRecords(true);
		createPendingCycleEntries(true);
		_isConstructing = false;
	}

	void TaskScheduler::recalculateAccountBalances()
	{
		Wt::log("erp-info") << "TaskScheduler: Recalculating account balances";
		Dbo::Transaction t(dboSession);
		try
		{
			_server->accountsDatabase()._recalculateAccountBalances();
			t.commit();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::recalculateAccountBalances(): Error: " << e.what();
			if(_isConstructing)
				throw;
		}
	}

	void TaskScheduler::createPendingCycleEntries(bool scheduleNext)
	{
		Wt::log("erp-info") << "TaskScheduler: Checking for pending EntryCycle entries";
		
		Dbo::Transaction t(dboSession);
		steady_clock::duration nextEntryDuration = hours(6);
		
		try
		{
			nextEntryDuration = _server->accountsDatabase().createAllPendingCycleEntries(nextEntryDuration);
			t.commit();
		}
		catch(const std::exception &e)
		{
			Wt::log("error") << "TaskScheduler::createPendingCycleEntries(): Error: " << e.what();
			if(_isConstructing)
				throw;
		}

		//Repeat
		if(scheduleNext)
			_server->ioService().schedule(nextEntryDuration, std::bind(&TaskScheduler::createPendingCycleEntries, this, true));
	}

	void TaskScheduler::checkAbnormalRecords(bool scheduleNext)
	{
		Wt::log("erp-info") << "TaskScheduler: Checking for abnormal database entries";
		Dbo::Transaction t(dboSession);
		try
		{
			{
				AccountEntryCollection collection = _server->accountsDatabase().abnormalAccountEntries();
				size_t abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal AccountEntry records were found";
			}
			{
				IncomeCycleCollection collection = _server->accountsDatabase().abnormalIncomeCycles();
				size_t abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal IncomeCycle records were found";
			}
			{
				ExpenseCycleCollection collection = _server->accountsDatabase().abnormalExpenseCycles();
				size_t abnormalRecords = collection.size();
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal ExpenseCycle records were found";
			}
			{
				long long abnormalRecords = _locationCheckAbnormal;
				if(abnormalRecords > 0)
					Wt::log("warn") << abnormalRecords << " abnormal Location records were found";
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
