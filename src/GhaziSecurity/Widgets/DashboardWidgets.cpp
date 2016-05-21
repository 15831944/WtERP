#include "Widgets/DashboardWidgets.h"
#include "Application/WApplication.h"
#include "Dbo/AccountsDatabase.h"

#include <Wt/WText>

namespace GS
{

	DashboardOverviewTemplate::DashboardOverviewTemplate()
	{
		setTemplateText(tr("GS.Admin.Dashboard.Overview"));

		auto totalEntities = new RecordCountTemplate([]() -> long long {
			return APP->dboSession().find<Entity>().resultList().size();
		});
		bindWidget("totalEntities", totalEntities);
		bindWidget("persons", new RecordCountTemplate([]() -> long long {
			return APP->dboSession().find<Entity>().where("type = ?").bind(Entity::PersonType).resultList().size();
		}, totalEntities));
		bindWidget("businesses", new RecordCountTemplate([]() -> long long {
			return APP->dboSession().find<Entity>().where("type = ?").bind(Entity::BusinessType).resultList().size();
		}, totalEntities));
		bindWidget("employees", new RecordCountTemplate([]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().query<long long>(
				"SELECT COUNT(DISTINCT a.entity_id) FROM " + std::string(EmployeeAssignment::tableName()) + " a"
				).where("? >= startDate AND (endDate IS null OR ? < endDate)").bind(currentDate).bind(currentDate);
		}, totalEntities));
		bindWidget("personnel", new RecordCountTemplate([]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().query<long long>(
				"SELECT COUNT(DISTINCT a.entity_id) FROM " + std::string(EmployeeAssignment::tableName()) + " a "
				"INNER JOIN " + EmployeePosition::tableName() + " p ON (p.id = a.employeeposition_id AND p.type = ?)"
				).bind(EmployeePosition::PersonnelType).where("? >= startDate AND (endDate IS null OR ? < endDate)").bind(currentDate).bind(currentDate);
		}, totalEntities));
		bindWidget("clients", new RecordCountTemplate([]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().query<long long>(
				"SELECT COUNT(DISTINCT a.entity_id) FROM " + std::string(ClientAssignment::tableName()) + " a"
				).where("? >= startDate AND (endDate IS null OR ? < endDate)").bind(currentDate).bind(currentDate);
		}, totalEntities));

		auto clientAssignments = new RecordMultiCountTemplate([]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().find<ClientAssignment>().where("? >= startDate").bind(currentDate).resultList().size();
		},
			[]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().find<ClientAssignment>().where("? >= startDate AND (endDate IS null OR ? < endDate)").bind(currentDate).bind(currentDate)
				.resultList().size();
		});
		clientAssignments->setLeftStr(tr("NActive"));
		clientAssignments->setRightStr(tr("NNotStarted"));
		bindWidget("clientAssignments", clientAssignments);

		auto employeeAssignments = new RecordMultiCountTemplate([]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().find<EmployeeAssignment>().where("? >= startDate").bind(currentDate).resultList().size();
		},
			[]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().find<EmployeeAssignment>().where("? >= startDate AND (endDate IS null OR ? < endDate)").bind(currentDate).bind(currentDate)
				.resultList().size();
		});
		employeeAssignments->setLeftStr(tr("NActive"));
		employeeAssignments->setRightStr(tr("NNotStarted"));
		bindWidget("employeeAssignments", employeeAssignments);

		auto totalAccounts = new RecordCountTemplate([]() -> long long {
			return APP->dboSession().find<Account>().resultList().size();
		});
		bindWidget("totalAccounts", totalAccounts);
		bindWidget("entityAccounts", new RecordCountTemplate([]() -> long long {
			return APP->dboSession().query<long long>(
				"SELECT COUNT(1) FROM " + std::string(Account::tableName()) + " acc INNER JOIN " + Entity::tableName() + " e ON (e.bal_account_id = acc.id OR e.pnl_account_id = acc.id)"
			);
		}, totalAccounts));

		auto recurringIncomes = new RecordMultiCountTemplate([]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().find<IncomeCycle>().where("? >= startDate").bind(currentDate).resultList().size();
		},
		[]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().find<IncomeCycle>().where("? >= startDate AND (endDate IS null OR ? < endDate)").bind(currentDate).bind(currentDate)
				.resultList().size();
		});
		recurringIncomes->setLeftStr(tr("NActive"));
		recurringIncomes->setRightStr(tr("NNotStarted"));
		bindWidget("recurringIncomes", recurringIncomes);

		auto recurringExpenses = new RecordMultiCountTemplate([]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().find<ExpenseCycle>().where("? >= startDate").bind(currentDate).resultList().size();
		},
		[]() -> long long {
			Wt::WDate currentDate = Wt::WDate(boost::gregorian::day_clock::local_day());
			return APP->dboSession().find<ExpenseCycle>().where("? >= startDate AND (endDate IS null OR ? < endDate)").bind(currentDate).bind(currentDate)
				.resultList().size();
		});
		recurringExpenses->setLeftStr(tr("NActive"));
		recurringExpenses->setRightStr(tr("NNotStarted"));
		bindWidget("recurringExpenses", recurringExpenses);

		bindWidget("cashAccount", new CashAccountBalance());
		bindWidget("receivables", new ReceivablesBalance());
		bindWidget("payables", new PayablesBalance());
	}

	void DashboardOverviewTemplate::load()
	{
		TRANSACTION(APP);
		ReloadOnVisibleWidget::load();
	}

	void DashboardOverviewTemplate::reload()
	{
		TRANSACTION(APP);
		for(auto w : children())
		{
			if(auto rW = dynamic_cast<Reloadable*>(w))
			{
				rW->reload();
			}
		}
	}

	RecordCountTemplate::RecordCountTemplate(const std::function<long long()> &queryFunction, RecordCountTemplate *relativeTo)
		: _queryFunction(queryFunction), _relativeTo(relativeTo)
	{
		setTemplateText(tr("GS.RecordCountTemplate.ProgressBar"));
		auto container = new Wt::WContainerWidget();
		container->setWidth(Wt::WLength(100, Wt::WLength::Percentage));
		_text = new Wt::WText(tr("Loading..."), container);
		bindWidget("progress-bar", container);
	}

	void RecordCountTemplate::load()
	{
		if(!loaded())
			reload();

		Wt::WTemplate::load();
	}

	void RecordCountTemplate::reload()
	{
		try
		{
			TRANSACTION(APP);
			_currentCount = _queryFunction();
			_text->setText(boost::lexical_cast<std::string>(_currentCount));

			if(_relativeTo)
			{
				if(!_relativeTo->loaded())
					_relativeTo->load();

				if(_relativeTo->currentCount() == -1)
					resolveWidget("progress-bar")->setWidth(Wt::WLength(100, Wt::WLength::Percentage));
				else
				{
					double width = std::min(static_cast<double>(_currentCount) / _relativeTo->currentCount() * 100, 100.);
					resolveWidget("progress-bar")->setWidth(Wt::WLength(width, Wt::WLength::Percentage));
				}
			}
			else
				resolveWidget("progress-bar")->setWidth(Wt::WLength(100, Wt::WLength::Percentage));
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "RecordCountTemplate::reload(): Dbo error(" << e.code() << "): " << e.what();
			resolveWidget("progress-bar")->setWidth(Wt::WLength(100, Wt::WLength::Percentage));
			_currentCount = -1;
			_text->setText(tr("CouldNotLoad"));
		}
	}

	RecordMultiCountTemplate::RecordMultiCountTemplate(const std::function<long long()> &totalQueryFunction, const std::function<long long()> &leftQueryFunction)
		: RecordCountTemplate(leftQueryFunction), _totalCountFunction(totalQueryFunction)
	{
		setTemplateText(tr("GS.RecordCountTemplate.MultiProgressBar"));
		auto rightContainer = new Wt::WContainerWidget();
		rightContainer->setWidth(0);
		_rightText = new Wt::WText(tr("Loading..."), rightContainer);
		bindWidget("progress-bar-right", rightContainer);
	}

	void RecordMultiCountTemplate::reload()
	{
		try
		{
			TRANSACTION(APP);
			_totalCount = _totalCountFunction();

			if(_totalCount == 0)
			{
				_currentCount = 0;
				_text->setText(_leftStr.arg(0));
				_rightText->setText(_rightStr.arg(0));
				resolveWidget("progress-bar")->setWidth(Wt::WLength(50, Wt::WLength::Percentage));
				resolveWidget("progress-bar-right")->setWidth(Wt::WLength(50, Wt::WLength::Percentage));
				return;
			}

			_currentCount = _queryFunction();
			_text->setText(_leftStr.arg(_currentCount));
			_rightText->setText(_rightStr.arg(_totalCount - _currentCount));

			double leftWidth = static_cast<double>(_currentCount) / _totalCount * 100;
			resolveWidget("progress-bar")->setWidth(Wt::WLength(leftWidth, Wt::WLength::Percentage));
			resolveWidget("progress-bar-right")->setWidth(Wt::WLength(100 - leftWidth, Wt::WLength::Percentage));
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "RecordMultiCountTemplate::reload(): Dbo error(" << e.code() << "): " << e.what();
			_totalCount = -1;
			_currentCount = -1;
			resolveWidget("progress-bar")->setWidth(Wt::WLength(100, Wt::WLength::Percentage));
			resolveWidget("progress-bar-right")->setWidth(0);
			_text->setText(tr("CouldNotLoad"));
			_rightText->setText("");
		}
	}

	void CashAccountBalance::load()
	{
		if(!loaded())
			reload();

		Wt::WText::load();
	}

	void CashAccountBalance::reload()
	{
		try
		{
			WApplication *app = APP;
			TRANSACTION(app);
			auto cashAccountPtr = app->accountsDatabase().findOrCreateCashAccount();
			Wt::WString balanceStr = Wt::WLocale::currentLocale().toString(Money(std::abs(cashAccountPtr->balanceInCents()), DEFAULT_CURRENCY));
			if(cashAccountPtr->balanceInCents() > 0)
			{
				setText(tr("RsXDebit").arg(balanceStr));
				setStyleClass("text-success");
			}
			else if(cashAccountPtr->balanceInCents() < 0)
			{
				setText(tr("RsXCredit").arg(balanceStr));
				setStyleClass("text-danger");
			}
			else
			{
				setText(balanceStr);
				setStyleClass("");
			}
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "CashAccountBalance::reload(): Dbo error(" << e.code() << "): " << e.what();
			setText(tr("CouldNotLoad"));
			setStyleClass("");
		}
	}

	void ReceivablesBalance::load()
	{
		if(!loaded())
			reload();

		Wt::WText::load();
	}

	void ReceivablesBalance::reload()
	{
		try
		{
			WApplication *app = APP;
			TRANSACTION(app);

			double balanceInCents = app->dboSession().query<double>(
				"SELECT COALESCE(SUM(acc.balance), 0) FROM " + std::string(Account::tableName()) + " acc "
				"INNER JOIN " + Entity::tableName() + " e ON e.bal_account_id = acc.id"
				).where("acc.balance > 0");

			Wt::WString balanceStr = Wt::WLocale::currentLocale().toString(Money(std::abs(static_cast<long long>(balanceInCents)), DEFAULT_CURRENCY));
			if(balanceInCents > 0)
			{
				setText(tr("RsXReceivable").arg(balanceStr));
				setStyleClass("text-success");
			}
			else
			{
				setText(balanceStr);
				setStyleClass("");
			}
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "ReceivablesBalance::reload(): Dbo error(" << e.code() << "): " << e.what();
			setText(tr("CouldNotLoad"));
			setStyleClass("");
		}
	}

	void PayablesBalance::load()
	{
		if(!loaded())
			reload();

		Wt::WText::load();
	}

	void PayablesBalance::reload()
	{
		try
		{
			WApplication *app = APP;
			TRANSACTION(app);

			double balanceInCents = app->dboSession().query<double>(
				"SELECT COALESCE(SUM(acc.balance), 0) FROM " + std::string(Account::tableName()) + " acc "
				"INNER JOIN " + Entity::tableName() + " e ON e.bal_account_id = acc.id"
				).where("acc.balance < 0");

			Wt::WString balanceStr = Wt::WLocale::currentLocale().toString(Money(std::abs(static_cast<long long>(balanceInCents)), DEFAULT_CURRENCY));
			if(balanceInCents < 0)
			{
				setText(tr("RsXPayable").arg(balanceStr));
				setStyleClass("text-danger");
			}
			else
			{
				setText(balanceStr);
				setStyleClass("");
			}
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "PayablesBalance::reload(): Dbo error(" << e.code() << "): " << e.what();
			setText(tr("CouldNotLoad"));
			setStyleClass("");
		}
	}

}
