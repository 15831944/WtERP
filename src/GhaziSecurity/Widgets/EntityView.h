#ifndef GS_ENTITYVIEW_WIDGET_H
#define GS_ENTITYVIEW_WIDGET_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Utilities/RecordFormView.h"

#include <Wt/WTemplateFormView.h>
#include <Wt/WDialog.h>
#include <Wt/WLineEdit.h>

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
	class ContactNumberView;
	class ExpenseCycleList;
	class IncomeCycleList;

	//EntityFormModel
	class EntityFormModel : public RecordFormModel<Entity>
	{
	public:
		static const Field nameField;

		EntityFormModel(EntityView *view, Dbo::ptr<Entity> entityPtr = Dbo::ptr<Entity>());
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		virtual void persistedHandler() override;
		EntityView *_view = nullptr;
	};

	//PersonFormModel
	class PersonFormModel : public ChildRecordFormModel<Person>
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

		PersonFormModel(EntityView *view, Dbo::ptr<Person> personPtr = Dbo::ptr<Person>());
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		EntityView *_view = nullptr;
	};

	//EmployeeFormModel
	class EmployeeFormModel : public ChildRecordFormModel<Employee>
	{
	public:
		static const Field companyNumberField;
		static const Field gradeField;
		static const Field recruitmentDateField;
		static const Field educationField;
		static const Field experienceField;
		static const Field addQualificationsField;

		EmployeeFormModel(EntityView *view, Dbo::ptr<Employee> employeePtr = Dbo::ptr<Employee>());
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		EntityView *_view = nullptr;
	};

	//PersonnelFormModel
	class PersonnelFormModel : public ChildRecordFormModel<Personnel>
	{
	public:
		static const Field policeStationField;
		static const Field policeVerifiedField;
		static const Field trainingCoursesField;
		static const Field armyNumberField;
		static const Field rankField;

		PersonnelFormModel(EntityView *view, Dbo::ptr<Personnel> personnelPtr = Dbo::ptr<Personnel>());
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		EntityView *_view = nullptr;
	};

	//BusinessFormModel
	class BusinessFormModel : public ChildRecordFormModel<Business>
	{
	public:
		BusinessFormModel(EntityView *view, Dbo::ptr<Business> businessPtr = Dbo::ptr<Business>());
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		EntityView *_view = nullptr;
	};
	
	class ContactNumberFormModel : public RecordFormModel<ContactNumber>
	{
	public:
		static const Wt::WFormModel::Field entityField;
		static const Wt::WFormModel::Field numberField;

		ContactNumberFormModel(ContactNumberView *view, Dbo::ptr<ContactNumber> contactNumberPtr = Dbo::ptr<ContactNumber>());
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		ContactNumberView *_view = nullptr;
	};

	class ContactNumberView : public RecordFormView
	{
	public:
		ContactNumberView(Dbo::ptr<ContactNumber> contactNumberPtr);
		ContactNumberView();
		virtual void initView() override;

		Dbo::ptr<ContactNumber> contactNumberPtr() const { return _model->recordPtr(); }
		shared_ptr<ContactNumberFormModel> model() const { return _model; }

	protected:
		shared_ptr<ContactNumberFormModel> _model;
		Dbo::ptr<ContactNumber> _tempPtr;
	};

	//ContactNumbersFormModel
	class ContactNumbersManagerModel : public MultipleRecordModel<ContactNumber>
	{
	public:
		ContactNumbersManagerModel(EntityView *view, PtrCollection collection = PtrCollection());

	protected:
		virtual bool saveChanges() override;
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual tuple<unique_ptr<RecordFormView>, shared_ptr<ModelType>> createRecordView(Dbo::ptr<RecordDbo> recordPtr) override;

		EntityView *_view = nullptr;
	};

	//LocationsManagerModel
	class LocationsManagerModel : public MultipleRecordModel<Location>
	{
	public:
		LocationsManagerModel(EntityView *view, PtrCollection collection = PtrCollection());

	protected:
		virtual bool saveChanges() override;
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual tuple<unique_ptr<RecordFormView>, shared_ptr<ModelType>> createRecordView(Dbo::ptr<RecordDbo> recordPtr) override;

		EntityView *_view = nullptr;
	};

	//EntityView
	class EntityView : public RecordFormView
	{
	public:
		EntityView(Entity::Type type = Entity::InvalidType);
		EntityView(Dbo::ptr<Entity> entityPtr);
		virtual void initView() override;

		void selectEntityType(Entity::Type type);
		void setSpecificType(Entity::SpecificType type);
		void addEmployeeModel(Dbo::ptr<Employee> employeePtr = Dbo::ptr<Employee>());
		void addPersonnelModel(Dbo::ptr<Personnel> personnelPtr = Dbo::ptr<Personnel>());

		Entity::Type entityType() const { return _type; }
		Dbo::ptr<Entity> entityPtr() const { return _entityModel->recordPtr(); }

		virtual Wt::WString viewName() const override { return _entityModel->valueText(EntityFormModel::nameField); }
		virtual std::string viewInternalPath() const override { return entityPtr() ? Entity::viewInternalPath(entityPtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<EntityView>(); }

	protected:
		virtual void submit() override;
		virtual void afterSubmitHandler() override;

		Wt::WPushButton *_selectPerson = nullptr;
		Wt::WPushButton *_selectBusiness = nullptr;
		ExpenseCycleList *_expenseCycles = nullptr;
		IncomeCycleList *_incomeCycles = nullptr;

		shared_ptr<EntityFormModel> _entityModel;
		shared_ptr<PersonFormModel> _personModel;
		shared_ptr<EmployeeFormModel> _employeeModel;
		shared_ptr<PersonnelFormModel> _personnelModel;
		shared_ptr<ContactNumbersManagerModel> _contactNumbersModel;
		shared_ptr<LocationsManagerModel> _locationsModel;
		shared_ptr<BusinessFormModel> _businessModel;

		Entity::Type _type = Entity::InvalidType;
		Entity::Type _defaultType = Entity::InvalidType;
		Entity::SpecificType _specificType = Entity::UnspecificType;
		Dbo::ptr<Entity> _tempPtr;

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

		HeightEdit();
		void selectUnit(Unit unit) { if(unit != _unit) setUnit(unit); }
		float valueInCm();
		void setValueInCm(float val);
		void setLabelBuddy(Wt::WWidget *templateView);

	protected:
		void setUnit(Unit unit);
		Unit _unit = cm;
	};

}

#endif