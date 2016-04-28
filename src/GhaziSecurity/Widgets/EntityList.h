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
	template<int IdColumn>
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
	class AllEntityList : public QueryModelFilteredList<boost::tuple<long long, std::string, Entity::Type, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, EntityType, SpecificTypeMask };
		AllEntityList();

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class PersonList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		PersonList();

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class EmployeeList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		EmployeeList();

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class PersonnelList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		PersonnelList();

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class BusinessList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		BusinessList();

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class ClientList : public QueryModelFilteredList<boost::tuple<long long, std::string, Entity::Type, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, EntityType, SpecificTypeMask };
		ClientList();

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

}

#endif