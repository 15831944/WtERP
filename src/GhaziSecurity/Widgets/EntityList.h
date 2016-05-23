#ifndef GS_ENTITYLIST_WIDGET_H
#define GS_ENTITYLIST_WIDGET_H

#include "Dbo/Dbos.h"
#include "Application/WApplication.h"
#include "Utilities/FilteredList.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/WTemplate>
#include <Wt/WLineEdit>
#include <Wt/WLink>
#include <Wt/WValidator>
#include <Wt/WDialog>
#include <Wt/WCompositeWidget>

#include <Wt/Dbo/QueryModel>
#include <Wt/WBatchEditProxyModel>
#include <Wt/WSortFilterProxyModel>

namespace GS
{
	class EntityView;
	class AbstractFilterWidgetModel;

	//PROXY MODEL
	template<class FilteredList>
	class EntityListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		EntityListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	//ENTITY LISTS
	class AllEntityList : public QueryModelFilteredList<boost::tuple<long long, std::string, Entity::Type, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEntityType, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewEntityType, ViewRole };
		virtual void reload() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class PersonList : public QueryModelFilteredList<boost::tuple<long long, std::string, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewRole };
		virtual void reload() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class EmployeeList : public QueryModelFilteredList<boost::tuple<long long, std::string, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewRole };
		virtual void reload() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class PersonnelList : public QueryModelFilteredList<boost::tuple<long long, std::string, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewRole };
		virtual void reload() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class BusinessList : public QueryModelFilteredList<boost::tuple<long long, std::string, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewRole };
		virtual void reload() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class ClientList : public QueryModelFilteredList<boost::tuple<long long, std::string, Entity::Type, boost::optional<long long>, boost::optional<long long>, boost::optional<long long>>>
	{
	public:
		enum ResultColumns { ResId, ResName, ResEntityType, ResEmployeeAssignment, ResPersonnelPosition, ResClientAssignment };
		enum ViewColumns { ViewId, ViewName, ViewEntityType, ViewRole };
		virtual void reload() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	//TEMPLATE CLASS DEFINITIONS
	template<class FilteredList>
	EntityListProxyModel<FilteredList>::EntityListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	template<class FilteredList>
	boost::any EntityListProxyModel<FilteredList>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const FilteredList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<FilteredList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<FilteredList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, Entity::viewInternalPath(id));
			}
		}

		boost::any viewIndexData = headerData(idx.column(), Wt::Horizontal, Wt::ViewIndexRole);
		if(viewIndexData.empty())
			return Wt::WBatchEditProxyModel::data(idx, role);
		int viewIndex = boost::any_cast<int>(viewIndexData);

		if(viewIndex == FilteredList::ViewRole && role == Wt::DisplayRole)
		{
			const FilteredList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<FilteredList::ResultType>*>(sourceModel())->resultRow(idx.row());
			int typeMask = Entity::UnspecificType;
			if(boost::get<FilteredList::ResEmployeeAssignment>(res).is_initialized())
				typeMask |= Entity::EmployeeType;
			if(boost::get<FilteredList::ResPersonnelPosition>(res).is_initialized())
				typeMask |= Entity::PersonnelType;
			if(boost::get<FilteredList::ResClientAssignment>(res).is_initialized())
				typeMask |= Entity::ClientType;

			return Wt::WFlags<Entity::SpecificType>((Entity::SpecificType)typeMask);
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	template<class FilteredList>
	Wt::WFlags<Wt::ItemFlag> EntityListProxyModel<FilteredList>::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	template<class FilteredList>
	boost::any EntityListProxyModel<FilteredList>::headerData(int section, Wt::Orientation orientation /* = Wt::Horizontal */, int role /* = Wt::DisplayRole */) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
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