#ifndef GS_FINDRECORDEDIT_H
#define GS_FINDRECORDEDIT_H

#include "Application/WApplication.h"

#include <Wt/WTemplate>
#include <Wt/WLineEdit>
#include <Wt/WCompositeWidget>
#include <Wt/WSuggestionPopup>
#include <Wt/Dbo/QueryModel>

namespace GS
{
	class AbstractFilteredList;
	class AccountView;
	class EntityView;
	class LocationView;

	//FindRecordEditTemplate
	class FindRecordEditTemplate : public Wt::WTemplate
	{
	public:
		FindRecordEditTemplate(Wt::WLineEdit *edit, Wt::WContainerWidget *parent = nullptr);
		virtual void setDisabled(bool disabled) override;

	protected:
		void setReadOnly(bool readOnly);
		Wt::WLineEdit *_edit = nullptr;
	};

	class AbstractFindRecordEdit : public Wt::WCompositeWidget
	{
	public:
		AbstractFindRecordEdit();
		virtual void load() override;
		virtual void setValuePtr(boost::any ptrData) = 0;
		virtual boost::any valueAny() const = 0;

		FindRecordEditTemplate *containerTemplate() const { return _containerTemplate; }
		Wt::WLineEdit *lineEdit() const { return _lineEdit; }
		Wt::Signal<void> &valueChanged() { return _valueChanged; }

	protected:
		virtual void propagateSetEnabled(bool enabled) override;
		virtual void setTextFromValuePtr() = 0;
		void handleLineEditChanged();

		Wt::WLineEdit *_lineEdit = nullptr;
		FindRecordEditTemplate *_containerTemplate = nullptr;
		Wt::WSuggestionPopup *_suggestionPopup = nullptr;
		Wt::Signal<void> _valueChanged;
	};

	//FindRecordEdit
	template<class Value>
	class FindRecordEdit : public AbstractFindRecordEdit
	{
	public:
		virtual void setValuePtr(boost::any ptrData) override { setValuePtr(ptrData.empty() ? Wt::Dbo::ptr<Value>() : boost::any_cast<Wt::Dbo::ptr<Value>>(ptrData)); }
		void setValuePtr(Wt::Dbo::ptr<Value> ptr);

		Wt::Dbo::ptr<Value> valuePtr() const { return _valuePtr; }
		virtual boost::any valueAny() const override { return valuePtr(); }
		void setIdColumn(int column) { _idColumn = column; }
		int idColumn() const { return _idColumn; }

	protected:
		void handleActivated(int index, Wt::WFormWidget *lineEdit);

		Wt::Dbo::ptr<Value> _valuePtr;
		int _idColumn = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// FIND ENTITY
	//////////////////////////////////////////////////////////////////////////
	//FindEntitySuggestionPopup
	class FindEntitySuggestionPopup : public Wt::WSuggestionPopup
	{
	public:
		FindEntitySuggestionPopup(Wt::WObject *parent = nullptr);

	protected:
		void handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit);
	};

	//FindEntityEdit
	class FindEntityEdit : public FindRecordEdit<Entity>
	{
	public:
		FindEntityEdit(Entity::Type entityType = Entity::InvalidType, Entity::SpecificType specificType = Entity::UnspecificType);
		Entity::Type entityType() const { return _entityType; }
		Entity::SpecificType specificType() const { return _specificType; }

		void showNewEntityDialog();
		void showEntityListDialog();

	protected:
		virtual void setTextFromValuePtr() override;
		void handleEntityViewSubmitted(EntityView *view);
		void handleListSelectionChanged(long long id);

		Entity::Type _entityType;
		Entity::SpecificType _specificType;
		Wt::WDialog *_newDialog = nullptr;

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
		void setModifyPermissionRequired(bool required) { _modifyPermissionRequired = required; }
		bool modifyPermissionRequired() const { return _modifyPermissionRequired; }

	protected:
		FindEntityEdit *_findEdit;
		bool _modifyPermissionRequired = false;
	};

	//////////////////////////////////////////////////////////////////////////
	// FIND ACCOUNT
	//////////////////////////////////////////////////////////////////////////
	//FindAccountSuggestionPopup
	class FindAccountSuggestionPopup : public Wt::WSuggestionPopup
	{
	public:
		enum ResultColumns { Name, Id, Type, EntityName };
		FindAccountSuggestionPopup(Wt::WObject *parent = nullptr);

	protected:
		void handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit);
	};

	//FindAccountEdit
	class FindAccountEdit : public FindRecordEdit<Account>
	{
	public:
		FindAccountEdit();
		void showNewAccountDialog();
		void showAccountListDialog();

	protected:
		virtual void setTextFromValuePtr() override;
		void handleAccountViewSubmitted(AccountView *view);
		void handleListSelectionChanged(long long id);

		Account::Type _accountType;
		Wt::WDialog *_newDialog = nullptr;

	private:
		friend class FindAccountValidator;
	};

	//FindAccountValidator
	class FindAccountValidator : public Wt::WValidator
	{
	public:
		FindAccountValidator(FindAccountEdit *findEdit, bool mandatory = false)
			: Wt::WValidator(mandatory, findEdit), _findEdit(findEdit)
		{ }
		virtual Result validate(const Wt::WString &input) const override;
		void setModifyPermissionRequired(bool required) { _modifyPermissionRequired = required; }
		bool modifyPermissionRequired() const { return _modifyPermissionRequired; }

	protected:
		FindAccountEdit *_findEdit;
		bool _modifyPermissionRequired = false;
	};

	//////////////////////////////////////////////////////////////////////////
	// FIND LOCATION
	//////////////////////////////////////////////////////////////////////////
	//FindLocationSuggestionPopup
	class FindLocationSuggestionPopup : public Wt::WSuggestionPopup
	{
	public:
		enum ResultColumns { Id, Address, CountryName, CityName, EntityName };
		FindLocationSuggestionPopup(Wt::WObject *parent = nullptr);

	protected:
		typedef boost::tuple<long long, std::string, std::string, std::string, std::string> ResultTuple;
		void handleFilterModel(const Wt::WString &str, Wt::WFormWidget *edit);
	};

	//FindLocationEdit
	class FindLocationEdit : public FindRecordEdit<Location>
	{
	public:
		FindLocationEdit();
		void showNewLocationDialog();
		void showLocationListDialog();

	protected:
		virtual void setTextFromValuePtr() override;
		void handleLocationViewSubmitted(LocationView *view);
		void handleListSelectionChanged(long long id);

		Wt::WDialog *_newDialog = nullptr;

	private:
		friend class FindLocationValidator;
	};

	//FindLocationValidator
	class FindLocationValidator : public Wt::WValidator
	{
	public:
		FindLocationValidator(FindLocationEdit *findEdit, bool mandatory = false)
			: Wt::WValidator(mandatory, findEdit), _findEdit(findEdit)
		{ }
		virtual Result validate(const Wt::WString &input) const override;
		void setModifyPermissionRequired(bool required) { _modifyPermissionRequired = required; }
		bool modifyPermissionRequired() const { return _modifyPermissionRequired; }

	protected:
		FindLocationEdit *_findEdit;
		bool _modifyPermissionRequired = false;
	};

	//OLD CRAP
// 	//FindEntityModel
// 	typedef boost::tuple<std::string, long long, Entity::Type> _FEMTuple;
// 	class FindEntityModel : public Wt::Dbo::QueryModel<_FEMTuple>
// 	{
// 	public:
// 		enum ResultColumns { Name, Id, Type };
// 
// 		FindEntityModel(Wt::WObject *parent = nullptr) : Wt::Dbo::QueryModel<_FEMTuple>(parent) { }
// 		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
// 	};

// 	//FindAccountModel
// 	typedef boost::tuple<std::string, long long, Account::Type, std::string> _FAMTuple;
// 	class FindAccountModel : public Wt::Dbo::QueryModel<_FAMTuple>
// 	{
// 	public:
// 		enum ResultColumns { Name, Id, Type, EntityName };
// 
// 		FindAccountModel(Wt::WObject *parent = nullptr);
// 		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
// 	};

}

#endif