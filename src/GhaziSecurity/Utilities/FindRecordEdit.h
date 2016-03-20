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

	//FindRecordEdit
	template<class Value>
	class FindRecordEdit : public Wt::WCompositeWidget
	{
	public:
		FindRecordEdit(Wt::WContainerWidget *parent = nullptr);
		virtual void setValuePtr(Wt::Dbo::ptr<Value> ptr);
		void setIdColumn(int column) { _idColumn = column; }
		virtual void load() override;

		Wt::Dbo::ptr<Value> valuePtr() const { return _valuePtr; }
		int idColumn() const { return _idColumn; }
		FindRecordEditTemplate *containerTemplate() const { return _containerTemplate; }
		Wt::WLineEdit *lineEdit() const { return _lineEdit; }
		Wt::Signal<void> &valueChanged() { return _valueChanged; }

	protected:
		void handleActivated(int index, Wt::WFormWidget *lineEdit);
		virtual void propagateSetEnabled(bool enabled) override;

		Wt::WLineEdit *_lineEdit = nullptr;
		FindRecordEditTemplate *_containerTemplate = nullptr;
		Wt::WSuggestionPopup *_suggestionPopup = nullptr;
		Wt::Dbo::ptr<Value> _valuePtr;
		Wt::Signal<void> _valueChanged;
		int _idColumn = 0;
	};
	
	//////////////////////////////////////////////////////////////////////////
	// FIND ENTITY
	//////////////////////////////////////////////////////////////////////////
	//FindEntitySuggestionPopup
	class FindEntitySuggestionPopup : public Wt::WSuggestionPopup
	{
	public:
		FindEntitySuggestionPopup(Entity::Type typeFilter, Wt::WObject *parent = nullptr);

	protected:
		void handleFilterModel(const Wt::WString &str);
		Entity::Type _typeFilter;
	};

	//FindEntityEdit
	class FindEntityEdit : public FindRecordEdit<Entity>
	{
	public:
		FindEntityEdit(Entity::Type entityType = Entity::InvalidType, Wt::WContainerWidget *parent = nullptr);
		virtual void setValuePtr(Wt::Dbo::ptr<Entity> ptr) override;
		void showNewEntityDialog();
		void showEntityListDialog();

	protected:
		void handleEntityViewSubmitted(EntityView *view);
		void handleListSelectionChanged(AbstractFilteredList *listWidget);

		Entity::Type _entityType;
		Wt::WDialog *_listDialog = nullptr;
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

	protected:
		FindEntityEdit *_findEdit;
	};

	//////////////////////////////////////////////////////////////////////////
	// FIND ACCOUNT
	//////////////////////////////////////////////////////////////////////////
	//FindAccountSuggestionPopup
	class FindAccountSuggestionPopup : public Wt::WSuggestionPopup
	{
	public:
		enum ResultColumns { Id, Name, Type, EntityName };
		FindAccountSuggestionPopup(Wt::WObject *parent = nullptr);

	protected:
		void handleFilterModel(const Wt::WString &str);
	};

	//FindAccountEdit
	class FindAccountEdit : public FindRecordEdit<Account>
	{
	public:
		FindAccountEdit(Wt::WContainerWidget *parent = nullptr);
		virtual void setValuePtr(Wt::Dbo::ptr<Account> ptr) override;
		void showNewAccountDialog();
		void showAccountListDialog();

	protected:
		void handleAccountViewSubmitted(AccountView *view);
		void handleListSelectionChanged(AbstractFilteredList *listWidget);

		Account::Type _accountType;
		Wt::WDialog *_listDialog = nullptr;
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

	protected:
		FindAccountEdit *_findEdit;
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