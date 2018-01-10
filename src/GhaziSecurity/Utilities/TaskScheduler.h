#ifndef GS_PAYMENTCYCLESCHEDULER_H
#define GS_PAYMENTCYCLESCHEDULER_H

#include "Dbo/Dbos.h"
#include "Dbo/AccountsDatabase.h"
#include <Wt/Dbo/Session.h>

namespace GS
{
	using namespace std::chrono;

	class WServer;

	class TaskScheduler
	{
	public:
		TaskScheduler(WServer *server, Wt::Dbo::Session &session);

	protected:
		typedef std::tuple<Wt::Dbo::ptr<IncomeCycle>, Wt::Dbo::ptr<AccountEntry>> IncomeCycleTuple;
		typedef Wt::Dbo::collection<IncomeCycleTuple> IncomeTupleCollection;
		typedef std::tuple<Wt::Dbo::ptr<ExpenseCycle>, Wt::Dbo::ptr<AccountEntry>> ExpenseCycleTuple;
		typedef Wt::Dbo::collection<ExpenseCycleTuple> ExpenseTupleCollection;

		//void createSelfEntityAndAccount(bool scheduleNext);
		void createDefaultAccounts(bool scheduleNext);
		void recalculateAccountBalances(bool scheduleNext);
		void createPendingCycleEntries(bool scheduleNext);
		void checkAbnormalRecords(bool scheduleNext);
		
		steady_clock::duration _createPendingCycleEntries(bool scheduleNext);

		Wt::Dbo::Query<long long> _accountCheckAbnormal;
		Wt::Dbo::Query<long long> _locationCheckAbnormal;
		Wt::Dbo::Query<Wt::Dbo::ptr<AttendanceEntry>> _attendanceCheckAbnormal;
		Wt::Dbo::Query<Wt::Dbo::ptr<IncomeCycle>> _incomeCycleCheckAbnormal;
		Wt::Dbo::Query<Wt::Dbo::ptr<ExpenseCycle>> _expenseCycleCheckAbnormal;
		Wt::Dbo::Query<Wt::Dbo::ptr<AccountEntry>> _entryCheckAbnormal;
		Wt::Dbo::Query<IncomeCycleTuple> _incomeCycleQuery;
		Wt::Dbo::Query<ExpenseCycleTuple> _expenseCycleQuery;
		std::unique_ptr<Wt::Dbo::Call> _recalculateBalanceCall;

		Wt::Dbo::Session &dboSession;
		EntitiesDatabase _entitiesDatabase;
		AccountsDatabase _accountsDatabase;
		WServer *_server = nullptr;
		bool _isConstructing = false;
	};

}

#endif