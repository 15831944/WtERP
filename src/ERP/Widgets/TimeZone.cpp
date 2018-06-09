#include "Widgets/TimeZone.h"

#include <Wt/WTime.h>
#include <Wt/Date/tz_private.h>
#include <boost/algorithm/string.hpp>

namespace ERP
{
	void TimeZoneModel::load()
	{
		for(const date::time_zone &z : date::get_tzdb().zones)
		{
			if(z.name().size() > 3 && z.name().find("Etc/") == std::string::npos)
			{
				if(std::any_of(std::begin(z.name()), std::end(z.name()), [](char c){return (islower(c));}))
					_ids.push_back(z.name());
			}
		}
	}
	
	int TimeZoneModel::suggestedTimeZone(minutes currentOffset)
	{
		system_clock::time_point nowUtc = system_clock::now();
		
		int bestPreference = 0;
		int bestRow = -1;
		
		for(unsigned i = 0; i < _ids.size(); ++i)
		{
			std::string id = _ids[i];
			const date::time_zone* zone = date::locate_zone(id);
			auto zoneinfo = zone->get_info(nowUtc);
			
			if(zoneinfo.offset == currentOffset)
			{
				int pref = computePreference(id, zone);
				if(pref > bestPreference)
				{
					bestRow = i;
					bestPreference = pref;
				}
			}
		}
		
		return bestRow;
	}
	
	int TimeZoneModel::rowCount(const Wt::WModelIndex &parent) const
	{
		if (!parent.isValid())
			return static_cast<int>(_ids.size());
		else
			return 0;
	}
	
	int TimeZoneModel::columnCount(const Wt::WModelIndex &parent) const
	{
		if (!parent.isValid())
			return 1;
		else
			return 0;
	}
	
	std::string TimeZoneModel::locality(const std::string &id)
	{
		std::string result = id.substr(id.find('/') + 1);
		boost::replace_all(result, "_", " ");
		return result;
	}
	
	Wt::any TimeZoneModel::data(const Wt::WModelIndex &index, Wt::ItemDataRole role) const
	{
		std::string id = _ids[index.row()];
		
		switch (role.value())
		{
		case Wt::ItemDataRole::Display:
		{
			if(_showOffset)
			{
				const date::time_zone *zone = date::locate_zone(id);
				auto info = zone->get_info(system_clock::now());
				Wt::WTime t = Wt::WTime(0, 0, 0).addSecs(static_cast<int>(info.offset.count()));
				
				std::string result = locality(id) + " (GMT" + t.toString("+hh:mm").toUTF8() + ")";
				
				return result;
			}
			else
				return locality(id);
		}
		case Wt::ItemDataRole::Level:
			return id.substr(0, id.find('/'));
		case NameTimeZoneRole.value():
			return id;
		default:
			return Wt::any();
		}
	}
	
	Wt::any TimeZoneModel::headerData(int, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(orientation == Wt::Orientation::Horizontal)
		{
			switch (role.value())
			{
			case Wt::ItemDataRole::Display:
				return std::string("locality");
			default:
				return Wt::any();
			}
		}
		else
			return Wt::any();
	}
	
	int TimeZoneModel::computePreference(const std::string &id, const date::time_zone *zone)
	{
		/*
		 * We implement here the following heuristic:
		 *
		 * Take first city of the following 'preferred' list:
		 *   - Europe (5)
		 *   - Asia (4),
		 *   - Australia (3),
		 *   - America (2)
		 * Otherwise, 1
		 */
		if (boost::starts_with(id, "Europe/"))
			return 5;
		else if (boost::starts_with(id, "Australia/"))
			return 4;
		else if (boost::starts_with(id, "Asia/"))
			return 3;
		else if (boost::starts_with(id, "America/"))
			return 2;
		else
			return 1;
	}
}
