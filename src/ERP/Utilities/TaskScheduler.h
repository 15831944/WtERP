#ifndef ERP_PAYMENTCYCLESCHEDULER_H
#define ERP_PAYMENTCYCLESCHEDULER_H

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
		TaskScheduler(WServer *server, Dbo::Session &session);

	protected:
		typedef tuple<Dbo::ptr<IncomeCycle>, Dbo::ptr<AccountEntry>> IncomeCycleTuple;
		typedef Dbo::collection<IncomeCycleTuple> IncomeTupleCollection;
		typedef tuple<Dbo::ptr<ExpenseCycle>, Dbo::ptr<AccountEntry>> ExpenseCycleTuple;
		typedef Dbo::collection<ExpenseCycleTuple> ExpenseTupleCollection;

		//void createSelfEntityAndAccount(bool scheduleNext);
		void createDefaultAccounts(bool scheduleNext);
		void recalculateAccountBalances(bool scheduleNext);
		void createPendingCycleEntries(bool scheduleNext);
		void checkAbnormalRecords(bool scheduleNext);
		
		steady_clock::duration _createPendingCycleEntries(bool scheduleNext);

		Dbo::Query<long long> _accountCheckAbnormal;
		Dbo::Query<long long> _locationCheckAbnormal;
		Dbo::Query<Dbo::ptr<AttendanceEntry>> _attendanceCheckAbnormal;
		Dbo::Query<Dbo::ptr<IncomeCycle>> _incomeCycleCheckAbnormal;
		Dbo::Query<Dbo::ptr<ExpenseCycle>> _expenseCycleCheckAbnormal;
		Dbo::Query<Dbo::ptr<AccountEntry>> _entryCheckAbnormal;
		Dbo::Query<IncomeCycleTuple> _incomeCycleQuery;
		Dbo::Query<ExpenseCycleTuple> _expenseCycleQuery;
		unique_ptr<Dbo::Call> _recalculateBalanceCall;

		Dbo::Session &dboSession;
		EntitiesDatabase _entitiesDatabase;
		AccountsDatabase _accountsDatabase;
		WServer *_server = nullptr;
		bool _isConstructing = false;
	};

}

#endif