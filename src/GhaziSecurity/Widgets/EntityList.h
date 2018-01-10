#ifndef GS_ENTITYLIST_WIDGET_H
#define GS_ENTITYLIST_WIDGET_H

#include "Dbo/Dbos.h"
#include "Application/WApplication.h"
#include "Utilities/FilteredList.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/WTemplate.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLink.h>
#include <Wt/WValidator.h>
#include <Wt/WDialog.h>
#include <Wt/WCompositeWidget.h>

#include <Wt/Dbo/QueryModel.h>
#include <Wt/WBatchEditProxyModel.h>
#include <Wt/WSortFilterProxyModel.h>

#include <boost/optional.hpp>

namespace GS
{
	class EntityView;
	class AbstractFilterWidgetModel;

	//PROXY MODEL
	template<class FilteredList>
	class EntityListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		EntityListProxyModel(std::shared_ptr<Wt::WAbstractItemModel> model);
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	//ENTITY LISTS
	class AllEntityList : public QueryModelFilteredList<std::tuple<long long, std::string, Entity::Type, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEntityType, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewEntityType, ViewRole };

	protected:
		virtual Wt::Dbo::Query<ResultType> generateQuery() const override;
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class PersonList : public QueryModelFilteredList<std::tuple<long long, std::string, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewRole };

	protected:
		virtual Wt::Dbo::Query<ResultType> generateQuery() const override;
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class EmployeeList : public QueryModelFilteredList<std::tuple<long long, std::string, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewRole };

	protected:
		virtual Wt::Dbo::Query<ResultType> generateQuery() const override;
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class PersonnelList : public QueryModelFilteredList<std::tuple<long long, std::string, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewRole };

	protected:
		virtual Wt::Dbo::Query<ResultType> generateQuery() const override;
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class BusinessList : public QueryModelFilteredList<std::tuple<long long, std::string, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewRole };

	protected:
		virtual Wt::Dbo::Query<ResultType> generateQuery() const override;
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class ClientList : public QueryModelFilteredList<std::tuple<long long, std::string, Entity::Type, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEntityType, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewEntityType, ViewRole };

	protected:
		virtual Wt::Dbo::Query<ResultType> generateQuery() const override;
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	//TEMPLATE CLASS DEFINITIONS
	template<class FilteredList>
	EntityListProxyModel<FilteredList>::EntityListProxyModel(std::shared_ptr<Wt::WAbstractItemModel> model)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	template<class FilteredList>
	Wt::any EntityListProxyModel<FilteredList>::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("GS.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const FilteredList::ResultType &res = std::static_pointer_cast<Wt::Dbo::QueryModel<FilteredList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<FilteredList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, Entity::viewInternalPath(id));
			}
		}

		Wt::any viewIndexData = headerData(idx.column(), Wt::Orientation::Horizontal, Wt::ItemDataRole::ViewIndex);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = Wt::any_cast<int>(viewIndexData);

		if(viewIndex == FilteredList::ViewRole && role == Wt::ItemDataRole::Display)
		{
			const FilteredList::ResultType &res = std::static_pointer_cast<Wt::Dbo::QueryModel<FilteredList::ResultType>>(sourceModel())->resultRow(idx.row());
			int typeMask = Entity::UnspecificType;
			if(std::get<FilteredList::ResEmployeeAssignment>(res).is_initialized())
				typeMask |= Entity::EmployeeType;
			if(std::get<FilteredList::ResPersonnelPosition>(res).is_initialized())
				typeMask |= Entity::PersonnelType;
			if(std::get<FilteredList::ResClientAssignment>(res).is_initialized())
				typeMask |= Entity::ClientType;

			return Wt::WFlags<Entity::SpecificType>((Entity::SpecificType)typeMask);
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	template<class FilteredList>
	Wt::WFlags<Wt::ItemFlag> EntityListProxyModel<FilteredList>::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	template<class FilteredList>
	Wt::any EntityListProxyModel<FilteredList>::headerData(int section, Wt::Orientation orientation /* = Wt::Orientation::Horizontal */, Wt::ItemDataRole role /* = Wt::ItemDataRole::Display */) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	template<class FilteredList>
	void EntityListProxyModel<FilteredList>::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

}

#endif