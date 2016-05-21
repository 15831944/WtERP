#ifndef GS_PAYMENTCYCLESCHEDULER_H
#define GS_PAYMENTCYCLESCHEDULER_H

#include "Dbo/Dbos.h"
#include "Dbo/AccountsDatabase.h"
#include <Wt/Dbo/Session>
#include <Wt/Dbo/ptr_tuple>

namespace GS
{
	class WServer;

	class TaskScheduler
	{
	public:
		TaskScheduler(WServer *server, Wt::Dbo::Session &session);
		~TaskScheduler();

	protected:
		typedef Wt::Dbo::ptr_tuple<IncomeCycle, AccountEntry>::type IncomeCycleTuple;
		typedef Wt::Dbo::collection<IncomeCycleTuple> IncomeTupleCollection;
		typedef Wt::Dbo::ptr_tuple<ExpenseCycle, AccountEntry>::type ExpenseCycleTuple;
		typedef Wt::Dbo::collection<ExpenseCycleTuple> ExpenseTupleCollection;

		//void createSelfEntityAndAccount(bool scheduleNext);
		void createDefaultAccounts(bool scheduleNext);
		void recalculateAccountBalances(bool scheduleNext);
		void createPendingCycleEntries(bool scheduleNext);
		void _createPendingCycleEntries(bool scheduleNext);
		void checkAbnormalRecords(bool scheduleNext);

		Wt::Dbo::Query<long long> _accountCheckAbnormal;
		Wt::Dbo::Query<long long> _locationCheckAbnormal;
		Wt::Dbo::Query<Wt::Dbo::ptr<AttendanceEntry>> _attendanceCheckAbnormal;
		Wt::Dbo::Query<Wt::Dbo::ptr<IncomeCycle>> _incomeCycleCheckAbnormal;
		Wt::Dbo::Query<Wt::Dbo::ptr<ExpenseCycle>> _expenseCycleCheckAbnormal;
		Wt::Dbo::Query<Wt::Dbo::ptr<AccountEntry>> _entryCheckAbnormal;
		Wt::Dbo::Query<IncomeCycleTuple> _incomeCycleQuery;
		Wt::Dbo::Query<ExpenseCycleTuple> _expenseCycleQuery;
		Wt::Dbo::Call *_recalculateBalanceCall = nullptr;

		Wt::Dbo::Session &dboSession;
		EntitiesDatabase _entitiesDatabase;
		AccountsDatabase _accountsDatabase;
		WServer *_server = nullptr;
		bool _isConstructing = false;
	};

}

#endif