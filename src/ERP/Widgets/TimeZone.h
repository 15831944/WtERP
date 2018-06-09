#ifndef ERP_TIMEZONECOMBO_WIDGET_H
#define ERP_TIMEZONECOMBO_WIDGET_H

#include "Common.h"
#include <Wt/WAbstractTableModel.h>

namespace date
{
	class time_zone;
}

namespace ERP
{
	class TimeZoneModel : public Wt::WAbstractTableModel
	{
	public:
		static constexpr Wt::ItemDataRole NameTimeZoneRole = Wt::ItemDataRole::User + 1;
		
		TimeZoneModel() : _showOffset(true) { }
		void load();
		
		virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;
		virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;
		virtual Wt::any data(const Wt::WModelIndex& index, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		
		void setShowOffset(bool enabled) { _showOffset = enabled; }
		
		int suggestedTimeZone(minutes currentOffset);
		static std::string locality(const std::string &id);
	
	protected:
		virtual int computePreference(const std::string &id, const date::time_zone *zone);
	
	private:
		std::vector<std::string> _ids;
		bool _showOffset;
	};
}

#endif