#ifndef GS_ENTITYVIEW_WIDGET_H
#define GS_ENTITYVIEW_WIDGET_H

#include "Dbo/Dbos.h"
#include "Utilities/MyFormView.h"

#include <Wt/WTemplateFormView>
#include <Wt/WDialog>
#include <Wt/WLineEdit>

namespace Magick
{
	class Blob;
}

namespace GS
{
	class EntityView;
	class LocationView;
	class LocationFormModel;
	class IncomeCycleSummaryContainer;
	class ExpenseCycleSummaryContainer;
	class EntityExpenseCycleList;
	class EntityIncomeCycleList;

	//EntityFormModel
	class EntityFormModel : public Wt::WFormModel
	{
	public:
		static const Field nameField;

		EntityFormModel(EntityView *view, Wt::Dbo::ptr<Entity> entityPtr = Wt::Dbo::ptr<Entity>());
		Wt::WWidget *createFormWidget(Field field);
		Wt::Dbo::ptr<Entity> entityPtr() const { return _entityPtr; }
		void saveChanges();

	protected:
		EntityView *_view = nullptr;
		Wt::Dbo::ptr<Entity> _entityPtr;
	};

	//PersonFormModel
	class PersonFormModel : public Wt::WFormModel
	{
	public:
		static const Field dobField;
		static const Field cnicField;
		static const Field motherTongueField;
		static const Field identificationMarkField;
		static const Field heightField;
		static const Field bloodTypeField;
		static const Field marriedField;
		static const Field nextOfKinField;
		static const Field fatherField;
		static const Field motherField;
		static const Field remarksField;
		static const Field profileUploadField;
		static const Field cnicUploadField;
		static const Field cnicUpload2Field;

		PersonFormModel(EntityView *view, Wt::Dbo::ptr<Person> personPtr = Wt::Dbo::ptr<Person>());
		Wt::WWidget *createFormWidget(Field field);
		Wt::Dbo::ptr<Person> personPtr() const { return _personPtr; }
		void saveChanges();

	protected:
		EntityView *_view = nullptr;
		Wt::Dbo::ptr<Person> _personPtr;
	};

	//EmployeeFormModel
	class EmployeeFormModel : public Wt::WFormModel
	{
	public:
		static const Field companyNumberField;
		static const Field gradeField;
		static const Field recruitmentDateField;
		static const Field educationField;
		static const Field experienceField;
		static const Field addQualificationsField;

		EmployeeFormModel(EntityView *view, Wt::Dbo::ptr<Employee> employeePtr = Wt::Dbo::ptr<Employee>());
		Wt::WWidget *createFormWidget(Field field);
		Wt::Dbo::ptr<Employee> employeePtr() const { return _employeePtr; }
		void saveChanges();

	protected:
		EntityView *_view = nullptr;
		Wt::Dbo::ptr<Employee> _employeePtr;
	};

	//PersonnelFormModel
	class PersonnelFormModel : public Wt::WFormModel
	{
	public:
		static const Field policeStationField;
		static const Field policeVerifiedField;
		static const Field trainingCoursesField;
		static const Field armyNumberField;
		static const Field rankField;

		PersonnelFormModel(EntityView *view, Wt::Dbo::ptr<Personnel> personnelPtr = Wt::Dbo::ptr<Personnel>());
		Wt::WWidget *createFormWidget(Field field);
		Wt::Dbo::ptr<Personnel> personnelPtr() const { return _personnelPtr; }
		void saveChanges();

	protected:
		EntityView *_view = nullptr;
		Wt::Dbo::ptr<Personnel> _personnelPtr;
	};

	//BusinessFormModel
	class BusinessFormModel : public Wt::WFormModel
	{
	public:
		BusinessFormModel(EntityView *view, Wt::Dbo::ptr<Business> businessPtr = Wt::Dbo::ptr<Business>());
		Wt::WWidget *createFormWidget(Field field);
		Wt::Dbo::ptr<Business> businessPtr() const { return _businessPtr; }
		void saveChanges();

	protected:
		EntityView *_view = nullptr;
		Wt::Dbo::ptr<Business> _businessPtr;
	};

	//ContactNumbersFormModel
	class ContactNumbersManagerModel : public Wt::WFormModel
	{
	public:
		typedef std::vector<Wt::WString> NumberVector;
		typedef std::vector<Wt::Dbo::ptr<ContactNumber>> PtrVector;
		static const Field field;

		ContactNumbersManagerModel(EntityView *view);
		Wt::WWidget *createFormWidget(Field field);
		const PtrVector &ptrVector() const { return _ptrVector; }
		void saveChanges();

	protected:
		EntityView *_view = nullptr;
		PtrVector _ptrVector;

	private:
		friend class ContactNumbersContainer;
	};

	//LocationsManagerModel
	class LocationsManagerModel : public Wt::WFormModel
	{
	public:
		typedef std::vector<Wt::Dbo::ptr<Location>> PtrVector;
		static const Field field;

		LocationsManagerModel(EntityView *view);
		virtual bool validate() override;
		Wt::WWidget *createFormWidget(Field field);
		const PtrVector &ptrVector() const { return boost::any_cast<const PtrVector&>(value(field)); }
		void saveChanges();

	protected:
		EntityView *_view = nullptr;
	};

	//EntityView
	class EntityView : public MyTemplateFormView
	{
	public:
		EntityView(Entity::Type type = Entity::InvalidType, Wt::WContainerWidget *parent = nullptr);
		EntityView(Wt::Dbo::ptr<Entity> EntityPtr, Wt::WContainerWidget *parent = nullptr);
		void init();
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;
		virtual bool updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual bool updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		//virtual void indicateValidation(Wt::WFormModel::Field field, bool validated, Wt::WText *info, Wt::WWidget *edit, const Wt::WValidator::Result &validation) override;

		void selectEntityType(Entity::Type type);
		void submit();

		void setSpecificType(Entity::SpecificType type);
		Entity::Type entityType() const { return _type; }
		void addEmployeeModel(Wt::Dbo::ptr<GS::Employee> employeePtr = Wt::Dbo::ptr<GS::Employee>());
		void addPersonnelModel(Wt::Dbo::ptr<GS::Personnel> personnelPtr = Wt::Dbo::ptr<GS::Personnel>());

		Wt::Dbo::ptr<Entity> entityPtr() const { return _entityModel->entityPtr(); }
		virtual Wt::WString viewName() const override { return _entityModel->valueText(EntityFormModel::nameField); }
		virtual std::string viewInternalPath() const override { return entityPtr() ? Entity::viewInternalPath(entityPtr().id()) : ""; }
		virtual MyTemplateFormView *createFormView() override { return new EntityView(); }

		void updateModel()
		{
			if(_entityModel) updateModel(_entityModel);
			if(_personModel) updateModel(_personModel);
			if(_employeeModel) updateModel(_employeeModel);
			if(_personnelModel) updateModel(_personnelModel);
			if(_contactNumbersModel) updateModel(_contactNumbersModel);
			if(_locationsModel) updateModel(_locationsModel);
			if(_businessModel) updateModel(_businessModel);
		}
		void updateView()
		{
			if(_entityModel) updateView(_entityModel);
			if(_personModel) updateView(_personModel);
			if(_employeeModel) updateView(_employeeModel);
			if(_personnelModel) updateView(_personnelModel);
			if(_contactNumbersModel) updateView(_contactNumbersModel);
			if(_locationsModel) updateView(_locationsModel);
			if(_businessModel) updateView(_businessModel);
		}
// 		void resetValidation()
// 		{
// 			if(_entityModel) _entityModel->resetValidation();
// 			if(_personModel) _personModel->resetValidation();
// 			if(_employeeModel) _employeeModel->resetValidation();
// 			if(_personnelModel) _personnelModel->resetValidation();
// 			if(_contactNumbersModel) _contactNumbersModel->resetValidation();
// 			if(_locationsModel) _locationsModel->resetValidation();
// 			if(_businessModel) _businessModel->resetValidation();
// 		}

	protected:
		using Wt::WTemplateFormView::updateView;
		using Wt::WTemplateFormView::updateModel;
		virtual void applyArguments(Wt::WWidget *w, const std::vector<Wt::WString> &args) override;

		Wt::WPushButton *_selectPerson = nullptr;
		Wt::WPushButton *_selectBusiness = nullptr;
		Wt::WPushButton *_addEmployee = nullptr;
		Wt::WPushButton *_addPersonnel = nullptr;
		EntityExpenseCycleList *_expenseCycles = nullptr;
		EntityIncomeCycleList *_incomeCycles = nullptr;

		EntityFormModel *_entityModel = nullptr;
		PersonFormModel *_personModel = nullptr;
		EmployeeFormModel *_employeeModel = nullptr;
		PersonnelFormModel *_personnelModel = nullptr;
		ContactNumbersManagerModel *_contactNumbersModel = nullptr;
		LocationsManagerModel *_locationsModel = nullptr;
		BusinessFormModel *_businessModel = nullptr;

		Entity::Type _type = Entity::InvalidType;
		Entity::Type _defaultType = Entity::InvalidType;
		Entity::SpecificType _specificType = Entity::UnspecificType;

	private:
		friend class EntityFormModel;
		friend class PersonFormModel;
		friend class EmployeeFormModel;
		friend class PersonnelFormModel;
		friend class BusinessFormModel;
		friend class ClientFormModel;
		friend class ContactNumbersManagerModel;
		friend class LocationsManagerModel;
	};

	//HeightEdit
	class HeightEdit : public Wt::WTemplate
	{
	public:
		enum Unit
		{
			cm,
			ft
		};

		HeightEdit(Wt::WContainerWidget *parent = nullptr);
		void selectUnit(Unit unit) { if(unit != _unit) setUnit(unit); }
		float valueInCm();
		void setValueInCm(float val);
		void setLabelBuddy(Wt::WWidget *templateView);

	protected:
		void setUnit(Unit unit);
		Unit _unit = cm;
	};

	//ContactNumbersContainer
	class ContactNumbersContainer : public Wt::WContainerWidget
	{
	public:
		ContactNumbersContainer(ContactNumbersManagerModel *model, Wt::WContainerWidget *parent = nullptr);
		void reset();
		void updateViews(const ContactNumbersManagerModel::NumberVector &numberVector);
		void updateModels(ContactNumbersManagerModel::NumberVector &numberVector);
		void addFieldWidget();

	protected:
		ContactNumbersManagerModel *_model;
	};

// 	void initEntryCycleSummary(Wt::WTemplate *summaryTemplate, const EntryCycle &cycle, long long id);
// 
// 	class ExpenseCycleSummaryContainer : public Wt::WContainerWidget
// 	{
// 	public:
// 		typedef std::vector<Wt::Dbo::ptr<ExpenseCycle>> PtrVector;
// 
// 		ExpenseCycleSummaryContainer(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent = nullptr);
// 		const PtrVector &ptrVector() const { return _ptrVector; }
// 
// 	protected:
// 		PtrVector _ptrVector;
// 	};
// 
// 	class IncomeCycleSummaryContainer : public Wt::WContainerWidget
// 	{
// 	public:
// 		typedef std::vector<Wt::Dbo::ptr<IncomeCycle>> PtrVector;
// 
// 		IncomeCycleSummaryContainer(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent = nullptr);
// 		const PtrVector &ptrVector() const { return _ptrVector; }
// 
// 	protected:
// 		PtrVector _ptrVector;
// 	};

// 	class ExpenseCycleSummary : public Wt::WText
// 	{
// 	public:
// 		ExpenseCycleSummary(int index, Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::WContainerWidget *parent = nullptr)
// 			: Wt::WText(parent)
// 		{
// 			setText(tr("ExpenseCycleSummary"));
// 		}
// 
// 	protected:
// 		Wt::Dbo::ptr<ExpenseCycle> _cyclePtr;
// 	};
// 
// 	class IncomeCycleSummary : public Wt::WText
// 	{
// 	public:
// 		IncomeCycleSummary(int index, Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::WContainerWidget *parent = nullptr)
// 			: Wt::WText(parent)
// 		{
// 			setText(tr("IncomeCycleSummary"));
// 		}
// 
// 	protected:
// 		Wt::Dbo::ptr<IncomeCycle> _cyclePtr;
// 	};
// 
// 	//ExpenseCyclesManagerModel
// 	class ExpenseCyclesManagerModel : public Wt::WFormModel
// 	{
// 	public:
// 		typedef std::vector<Wt::Dbo::ptr<ExpenseCycle>> PtrVector;
// 		static const Field field;
// 
// 		ExpenseCyclesManagerModel(EntityView *view);
// 		Wt::WDialog *createAddCycleDialog();
// 		virtual bool validate() override;
// 		Wt::WWidget *createFormWidget(Field field);
// 		const PtrVector &ptrVector() const { return boost::any_cast<const PtrVector&>(value(field)); }
// 		void saveChanges();
// 
// 	protected:
// 		EntityView *_view = nullptr;
// 	};
// 
// 	//IncomeCyclesManagerModel
// 	class IncomeCyclesManagerModel : public Wt::WFormModel
// 	{
// 	public:
// 		typedef std::vector<Wt::Dbo::ptr<IncomeCycle>> PtrVector;
// 		static const Field field;
// 
// 		IncomeCyclesManagerModel(EntityView *view);
// 		Wt::WDialog *createAddCycleDialog();
// 		virtual bool validate() override;
// 		Wt::WWidget *createFormWidget(Field field);
// 		const PtrVector &ptrVector() const { return boost::any_cast<const PtrVector&>(value(field)); }
// 		void saveChanges();
// 
// 	protected:
// 		EntityView *_view = nullptr;
// 	};

}

#endif