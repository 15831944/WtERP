#ifndef GS_DASHBOARD_WIDGETS_H
#define GS_DASHBOARD_WIDGETS_H

#include "Common.h"
#include "Utilities/ReloadableWidget.h"

#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/Dbo/Query.h>

namespace ERP
{
	class DashboardOverviewTemplate : public ReloadOnVisibleWidget<Wt::WTemplate>
	{
	public:
		DashboardOverviewTemplate();
		virtual void load() override;
		virtual void reload() override;
	};

	class RecordCountTemplate : public Wt::WTemplate, public Reloadable
	{
	public:
		RecordCountTemplate(const std::function<long long()> &queryFunction, RecordCountTemplate *relativeTo = nullptr);
		virtual void load() override;
		virtual void reload() override;
		long long currentCount() const { return _currentCount; }
		
	protected:
		RecordCountTemplate *_relativeTo = nullptr;
		Wt::WText *_text = nullptr;
		std::function<long long()> _queryFunction;
		long long _currentCount = -1;
	};

	class RecordMultiCountTemplate : public RecordCountTemplate
	{
	public:
		RecordMultiCountTemplate(const std::function<long long()> &totalQueryFunction, const std::function<long long()> &leftQueryFunction);
		virtual void reload() override;
		long long totalCount() const { return _totalCount; }
		void setLeftStr(const Wt::WString &str) { _leftStr = str; }
		void setRightStr(const Wt::WString &str) { _rightStr = str; }

	protected:
		Wt::WText *_rightText = nullptr;
		std::function<long long()> _totalCountFunction;
		long long _totalCount = -1;
		Wt::WString _leftStr = "{1}";
		Wt::WString _rightStr = "{1}";
	};

	class CashAccountBalance : public Wt::WText, public Reloadable
	{
	public:
		virtual void load() override;
		virtual void reload() override;
	};

	class ReceivablesBalance : public Wt::WText, public Reloadable
	{
	public:
		virtual void load() override;
		virtual void reload() override;
	};

	class PayablesBalance : public Wt::WText, public Reloadable
	{
	public:
		virtual void load() override;
		virtual void reload() override;
	};
}

#endif
