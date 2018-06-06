#ifndef ERP_TASKSCHEDULER_H
#define ERP_TASKSCHEDULER_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Dbo/AccountsDatabase.h"
#include <Wt/Dbo/Session.h>

namespace ERP
{
	using namespace std::chrono;

	class WServer;

	class TaskScheduler
	{
	public:
		TaskScheduler(WServer *server, DboSession &session);

	protected:
		void createDefaultAccounts(bool scheduleNext);
		void createPendingCycleEntries(bool scheduleNext);
		void checkAbnormalRecords(bool scheduleNext);
		void recalculateAccountBalances();

		Dbo::Query<long long> _locationCheckAbnormal;

		DboSession &dboSession;
		WServer *_server = nullptr;
		bool _isConstructing = false;
	};

}

#endif