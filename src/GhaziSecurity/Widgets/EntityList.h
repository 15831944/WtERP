#ifndef GS_ENTITYLIST_WIDGET_H
#define GS_ENTITYLIST_WIDGET_H

#include "Dbo/Dbos.h"
#include "Application/WApplication.h"
#include "Utilities/FilteredList.h"

#include <Wt/WTemplate>
#include <Wt/WLineEdit>
#include <Wt/WLink>
#include <Wt/WValidator>
#include <Wt/WDialog>

#include <Wt/Dbo/QueryModel>
#include <Wt/WBatchEditProxyModel>
#include <Wt/WSortFilterProxyModel>

namespace GS
{
	class EntityView;
	class AbstractFilterWidgetModel;

	typedef boost::tuple<std::string, long long, Entity::Type> _FEMTuple;
	class FindEntityModel : public Wt::Dbo::QueryModel<_FEMTuple>
	{
	public:
		enum ResultColumns { Name, Id, Type };

		FindEntityModel(Wt::WObject *parent = nullptr) : Wt::Dbo::QueryModel<_FEMTuple>(parent) { }
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
	};

	class FindEntityFilterModel : public Wt::WSortFilterProxyModel
	{
	public:
		FindEntityFilterModel(Entity::Type acceptType, Wt::WObject *parent = nullptr)
			: Wt::WSortFilterProxyModel(parent), _acceptType(acceptType)
		{ }
		virtual bool filterAcceptRow(int sourceRow, const Wt::WModelIndex &sourceParent) const override
		{
			Entity::Type rowType = boost::any_cast<Entity::Type>(sourceModel()->index(sourceRow, FindEntityModel::Type).data());
			if(rowType == _acceptType)
				return true;
			return false;
		}

	protected:
		Entity::Type _acceptType;
	};

	//FindEntityEdit
	class FindEntityEdit : public Wt::WLineEdit
	{
	public:
		FindEntityEdit(Entity::Type entityType = Entity::InvalidType, Wt::WContainerWidget *parent = nullptr);
		void showNewEntityDialog();
		void showEntityListDialog();

		Wt::Dbo::ptr<Entity> entityPtr() const { return _entityPtr; }
		void setEntityPtr(Wt::Dbo::ptr<Entity> ptr);
		Wt::WPushButton *newEntityBtn() const { return _newEntity; }
		Wt::WPushButton *showListBtn() const { return _showList; }
		Wt::Signal<void> &entityChanged() { return _entityChanged; }
		//Wt::WValidator *validator();

	protected:
		void handleEntityViewSubmitted(EntityView *view);
		void handleListSelectionChanged(AbstractFilteredList *listWidget);
		void handleActivated(int index, Wt::WFormWidget *lineEdit);

		Entity::Type _entityType;
		Wt::WDialog *_listDialog = nullptr;
		Wt::WDialog *_newDialog = nullptr;
		Wt::WSuggestionPopup *_suggestionPopup = nullptr;
		Wt::WPushButton *_newEntity = nullptr;
		Wt::WPushButton *_showList = nullptr;
		Wt::Dbo::ptr<Entity> _entityPtr;
		Wt::Signal<void> _entityChanged;

	private:
		friend class FindEntityValidator;
	};

	//FindEntityValidator
	class FindEntityValidator : public Wt::WValidator
	{
	public:
		FindEntityValidator(FindEntityEdit *findEdit, bool mandatory = false)
			: Wt::WValidator(mandatory, findEdit), _findEdit(findEdit)
		{ }
		virtual Result validate(const Wt::WString &input) const override;

	protected:
		FindEntityEdit *_findEdit;
	};

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
		AllEntityList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class PersonList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		PersonList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class EmployeeList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		EmployeeList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class PersonnelList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		PersonnelList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class BusinessList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		BusinessList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class ClientList : public QueryModelFilteredList<boost::tuple<long long, std::string, Entity::Type, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, EntityType, SpecificTypeMask };
		ClientList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	//TEMPLATE CLASS DEFINITIONS
	template<int IdColumn>
	EntityListProxyModel<IdColumn>::EntityListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	template<int IdColumn>
	boost::any EntityListProxyModel<IdColumn>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");

			if(role == Wt::LinkRole)
			{
				boost::any idData = data(index(idx.row(), IdColumn));
				return Wt::WLink(Wt::WLink::InternalPath, Entity::viewInternalPath(Wt::asString(idData).toUTF8()));
			}
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	template<int IdColumn>
	Wt::WFlags<Wt::ItemFlag> EntityListProxyModel<IdColumn>::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	template<int IdColumn>
	boost::any EntityListProxyModel<IdColumn>::headerData(int section, Wt::Orientation orientation /* = Wt::Horizontal */, int role /* = Wt::DisplayRole */) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	template<int IdColumn>
	void EntityListProxyModel<IdColumn>::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

}

#endif