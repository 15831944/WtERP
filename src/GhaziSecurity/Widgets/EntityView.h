#ifndef GS_ENTITYVIEW_WIDGET_H
#define GS_ENTITYVIEW_WIDGET_H

#include "Dbo/Dbos.h"
#include "Utilities/RecordFormView.h"

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
	class ContactNumberView;
	class ExpenseCycleList;
	class IncomeCycleList;

	//EntityFormModel
	class EntityFormModel : public RecordFormModel<Entity>
	{
	public:
		static const Field nameField;

		EntityFormModel(EntityView *view, Wt::Dbo::ptr<Entity> entityPtr = Wt::Dbo::ptr<Entity>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
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

		PersonFormModel(EntityView *view, Wt::Dbo::ptr<Person> personPtr = Wt::Dbo::ptr<Person>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
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

		EmployeeFormModel(EntityView *view, Wt::Dbo::ptr<Employee> employeePtr = Wt::Dbo::ptr<Employee>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
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

		PersonnelFormModel(EntityView *view, Wt::Dbo::ptr<Personnel> personnelPtr = Wt::Dbo::ptr<Personnel>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		EntityView *_view = nullptr;
	};

	//BusinessFormModel
	class BusinessFormModel : public ChildRecordFormModel<Business>
	{
	public:
		BusinessFormModel(EntityView *view, Wt::Dbo::ptr<Business> businessPtr = Wt::Dbo::ptr<Business>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		EntityView *_view = nullptr;
	};
	
	class ContactNumberFormModel : public RecordFormModel<ContactNumber>
	{
	public:
		static const Wt::WFormModel::Field entityField;
		static const Wt::WFormModel::Field numberField;

		ContactNumberFormModel(ContactNumberView *view, Wt::Dbo::ptr<ContactNumber> contactNumberPtr = Wt::Dbo::ptr<ContactNumber>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		ContactNumberView *_view = nullptr;
	};

	class ContactNumberView : public RecordFormView
	{
	public:
		ContactNumberView(Wt::Dbo::ptr<ContactNumber> contactNumberPtr);
		ContactNumberView();
		virtual void initView() override;

		Wt::Dbo::ptr<ContactNumber> contactNumberPtr() const { return _model->recordPtr(); }
		ContactNumberFormModel *model() const { return _model; }

	protected:
		ContactNumberFormModel *_model = nullptr;
		Wt::Dbo::ptr<ContactNumber> _tempPtr;
	};

	//ContactNumbersFormModel
	class ContactNumbersManagerModel : public MultipleRecordModel<ContactNumber>
	{
	public:
		ContactNumbersManagerModel(EntityView *view, PtrCollection collection = PtrCollection());

	protected:
		virtual bool saveChanges() override;
		virtual RecordViewsContainer *createFormWidget(Field field) override;
		virtual std::tuple<RecordFormView*, ModelType*> createRecordView(Wt::Dbo::ptr<Dbo> recordPtr) override;

		EntityView *_view = nullptr;
	};

	//LocationsManagerModel
	class LocationsManagerModel : public MultipleRecordModel<Location>
	{
	public:
		LocationsManagerModel(EntityView *view, PtrCollection collection = PtrCollection());

	protected:
		virtual bool saveChanges() override;
		virtual RecordViewsContainer *createFormWidget(Field field) override;
		virtual std::tuple<RecordFormView*, ModelType*> createRecordView(Wt::Dbo::ptr<Dbo> recordPtr) override;

		EntityView *_view = nullptr;
	};

	//EntityView
	class EntityView : public RecordFormView
	{
	public:
		EntityView(Entity::Type type = Entity::InvalidType);
		EntityView(Wt::Dbo::ptr<Entity> entityPtr);
		virtual void initView() override;

		void selectEntityType(Entity::Type type);
		void setSpecificType(Entity::SpecificType type);
		void addEmployeeModel(Wt::Dbo::ptr<Employee> employeePtr = Wt::Dbo::ptr<Employee>());
		void addPersonnelModel(Wt::Dbo::ptr<Personnel> personnelPtr = Wt::Dbo::ptr<Personnel>());

		Entity::Type entityType() const { return _type; }
		Wt::Dbo::ptr<Entity> entityPtr() const { return _entityModel->recordPtr(); }

		virtual Wt::WString viewName() const override { return _entityModel->valueText(EntityFormModel::nameField); }
		virtual std::string viewInternalPath() const override { return entityPtr() ? Entity::viewInternalPath(entityPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new EntityView(); }

	protected:
		virtual void submit() override;
		virtual void afterSubmitHandler() override;

		Wt::WPushButton *_selectPerson = nullptr;
		Wt::WPushButton *_selectBusiness = nullptr;
		ExpenseCycleList *_expenseCycles = nullptr;
		IncomeCycleList *_incomeCycles = nullptr;

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
		Wt::Dbo::ptr<Entity> _tempPtr;

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

}

#endif