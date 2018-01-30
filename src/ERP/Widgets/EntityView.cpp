#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Widgets/AdminPages.h"
#include "Widgets/ImageUpload.h"
#include "Widgets/LocationMVC.h"
#include "Widgets/EntryCycleMVC.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/WLabel.h>
#include <Wt/WBreak.h>
#include <Wt/WMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WLineEdit.h>
#include <Wt/WDateEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WComboBox.h>
#include <Wt/WCheckBox.h>
#include <Wt/WFileUpload.h>
#include <Wt/WImage.h>
#include <Wt/WIntValidator.h>

#include <Wt/Dbo/QueryModel.h>

#include <boost/algorithm/string.hpp>

namespace ERP
{
	//ENTITY MODEL
	const Wt::WFormModel::Field EntityFormModel::nameField = "name";

	EntityFormModel::EntityFormModel(EntityView *view, Dbo::ptr<Entity> entityPtr)
		: RecordFormModel(view, entityPtr), _view(view)
	{
		addField(nameField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));
		}
	}

	unique_ptr<Wt::WWidget> EntityFormModel::createFormWidget(Field field)
	{
		if(field == nameField)
		{
			auto w = make_unique<Wt::WLineEdit>();
			w->setMaxLength(70);
			auto validator = make_shared<Wt::WLengthValidator>(0, 70);
			validator->setMandatory(true);
			setValidator(field, validator);
			return w;
		}
		return nullptr;
	}

	bool EntityFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);

		if(!_recordPtr)
		{
			_recordPtr = app->dboSession().addNew<Entity>();
			_recordPtr.modify()->setCreatedByValues();
		}

		_recordPtr.modify()->specificTypeMask = Wt::WFlags<Entity::SpecificType>();
		_recordPtr.modify()->name = valueText(nameField).toUTF8();

		t.commit();
		return true;
	}

	void EntityFormModel::persistedHandler()
	{
		_view->_expenseCycles = _view->bindNew<ExpenseCycleList>("expenseCycles", recordPtr());
		_view->_incomeCycles = _view->bindNew<IncomeCycleList>("incomeCycles", recordPtr());
	}

	//PERSON MODEL
	const Wt::WFormModel::Field PersonFormModel::dobField = "dob";
	const Wt::WFormModel::Field PersonFormModel::cnicField = "cnic";
	const Wt::WFormModel::Field PersonFormModel::motherTongueField = "motherTongue";
	const Wt::WFormModel::Field PersonFormModel::identificationMarkField = "identificationMark";
	const Wt::WFormModel::Field PersonFormModel::heightField = "height";
	const Wt::WFormModel::Field PersonFormModel::bloodTypeField = "bloodType";
	const Wt::WFormModel::Field PersonFormModel::marriedField = "married";
	const Wt::WFormModel::Field PersonFormModel::nextOfKinField = "nextOfKin";
	const Wt::WFormModel::Field PersonFormModel::fatherField = "father";
	const Wt::WFormModel::Field PersonFormModel::motherField = "mother";
	const Wt::WFormModel::Field PersonFormModel::remarksField = "remarks";
	const Wt::WFormModel::Field PersonFormModel::profileUploadField = "profileUpload";
	const Wt::WFormModel::Field PersonFormModel::cnicUploadField = "cnicUpload";
	const Wt::WFormModel::Field PersonFormModel::cnicUpload2Field = "cnicUpload2";

	PersonFormModel::PersonFormModel(EntityView *view, Dbo::ptr<Person> personPtr)
		: ChildRecordFormModel(view->_entityModel, view, personPtr), _view(view)
	{
		addField(dobField);
		addField(cnicField);
		addField(motherTongueField);
		addField(identificationMarkField);
		addField(heightField);
		addField(bloodTypeField);
		addField(marriedField);
		addField(nextOfKinField);
		addField(fatherField);
		addField(motherField);
		addField(remarksField);
		addField(profileUploadField);
		addField(cnicUploadField);
		addField(cnicUpload2Field);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(dobField, _recordPtr->dateOfBirth);
			setValue(cnicField, Wt::WString::fromUTF8(_recordPtr->cnicNumber));
			setValue(motherTongueField, Wt::WString::fromUTF8(_recordPtr->motherTongue));
			setValue(identificationMarkField, Wt::WString::fromUTF8(_recordPtr->identificationMark));
			setValue(heightField, _recordPtr->height);
			setValue(bloodTypeField, (int)_recordPtr->bloodType);
			setValue(marriedField, (int)_recordPtr->maritalStatus);
			setValue(nextOfKinField, _recordPtr->nextOfKinOfPtr.id());
			setValue(fatherField, _recordPtr->fatherPersonPtr.id());
			setValue(motherField, _recordPtr->motherPersonPtr.id());
			setValue(remarksField, Wt::WString::fromUTF8(_recordPtr->remarks));

			if(_recordPtr->profilePictureFilePtr) setValue(profileUploadField, UploadedImage(_recordPtr->profilePictureFilePtr));
			if(_recordPtr->cnicFile1Ptr) setValue(cnicUploadField, UploadedImage(_recordPtr->cnicFile1Ptr));
			if(_recordPtr->cnicFile2Ptr) setValue(cnicUpload2Field, UploadedImage(_recordPtr->cnicFile2Ptr));
		}
	}

	unique_ptr<Wt::WWidget> PersonFormModel::createFormWidget(Field field)
	{
		if(field == dobField)
		{
			auto w = make_unique<Wt::WDateEdit>();
			w->setPlaceholderText(WApplication::instance()->locale().dateFormat());
			auto validator = make_shared<Wt::WDateValidator>();
			validator->setTop(Wt::WDate::currentServerDate());
			setValidator(field, validator);
			return w;
		}
		if(field == cnicField)
		{
			auto w = make_unique<Wt::WLineEdit>();
			w->setInputMask("99999-9999999-9");
			return w;
		}
		if(field == motherTongueField)
		{
			auto w = make_unique<Wt::WLineEdit>();
			w->setMaxLength(70);
			setValidator(field, make_shared<Wt::WLengthValidator>(0, 70));
			return w;
		}
		if(field == identificationMarkField)
		{
			auto w = make_unique<Wt::WLineEdit>();
			w->setMaxLength(255);
			setValidator(field, make_shared<Wt::WLengthValidator>(0, 255));
			return w;
		}
		if(field == heightField)
		{
			auto w = make_unique<HeightEdit>();
			w->setLabelBuddy(_view);
			return w;
		}
		if(field == bloodTypeField)
		{
			auto w = make_unique<Wt::WComboBox>();
			w->insertItem(UnknownBT, Wt::any_traits<BloodType>::asString(UnknownBT, ""));
			w->insertItem(OPositive, Wt::any_traits<BloodType>::asString(OPositive, ""));
			w->insertItem(ONegative, Wt::any_traits<BloodType>::asString(ONegative, ""));
			w->insertItem(APositive, Wt::any_traits<BloodType>::asString(APositive, ""));
			w->insertItem(ANegative, Wt::any_traits<BloodType>::asString(ANegative, ""));
			w->insertItem(BPositive, Wt::any_traits<BloodType>::asString(BPositive, ""));
			w->insertItem(BNegative, Wt::any_traits<BloodType>::asString(BNegative, ""));
			w->insertItem(ABPositive, Wt::any_traits<BloodType>::asString(ABPositive, ""));
			w->insertItem(ABNegative, Wt::any_traits<BloodType>::asString(ABNegative, ""));
			return w;
		}
		if(field == marriedField)
		{
			auto w = make_unique<Wt::WComboBox>();
			w->insertItem(UnknownMS, Wt::any_traits<MaritalStatus>::asString(UnknownMS, ""));
			w->insertItem(Married, Wt::any_traits<MaritalStatus>::asString(Married, ""));
			w->insertItem(Unmarried, Wt::any_traits<MaritalStatus>::asString(Unmarried, ""));
			return w;
		}
		if(field == nextOfKinField)
		{
			auto w = make_unique<FindEntityEdit>(Entity::PersonType);
			setValidator(field, make_shared<FindEntityValidator>(w.get(), false));
			return w;
		}
		if(field == fatherField)
		{
			auto w = make_unique<FindEntityEdit>(Entity::PersonType);
			setValidator(field, make_shared<FindEntityValidator>(w.get(), false));
			return w;
		}
		if(field == motherField)
		{
			auto w = make_unique<FindEntityEdit>(Entity::PersonType);
			setValidator(field, make_shared<FindEntityValidator>(w.get(), false));
			return w;
		}
		if(field == remarksField)
		{
			auto w = make_unique<Wt::WTextArea>();
			w->setRows(3);
			return w;
		}
		if(field == profileUploadField)
		{
			auto w = make_unique<ImageUpload>(tr("ClickToUploadProfile"), tr("ClickToChangeProfile"));
			w->setPlaceholderImageLink(Wt::WLink("images/profile-placeholder.png"));
			w->setThumbnailHeight(160);
			return w;
		}
		if(field == cnicUploadField)
		{
			auto w = make_unique<ImageUpload>(tr("ClickToUploadCNIC"), tr("ClickToChangeCNIC"));
			w->setThumbnailHeight(160);
			return w;
		}
		if(field == cnicUpload2Field)
		{
			auto w = make_unique<ImageUpload>(tr("ClickToUploadCNIC"), tr("ClickToChangeCNIC"));
			w->setThumbnailHeight(160);
			return w;
		}
		return RecordFormModel::createFormWidget(field);
	}

	bool PersonFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);
		if(!_recordPtr)
			_recordPtr = app->dboSession().addNew<Person>();
		if(!_recordPtr->entityPtr())
			_recordPtr.modify()->_entityPtr = _view->entityPtr();
		_recordPtr->entityPtr().modify()->type = Entity::PersonType;

		_recordPtr.modify()->dateOfBirth = Wt::any_cast<Wt::WDate>(value(dobField));

		std::string cnicNumber = valueText(cnicField).toUTF8();
		boost::erase_all(cnicNumber, "-");
		_recordPtr.modify()->cnicNumber = cnicNumber;

		_recordPtr.modify()->motherTongue = valueText(motherTongueField).toUTF8();
		_recordPtr.modify()->identificationMark = valueText(identificationMarkField).toUTF8();
		_recordPtr.modify()->height = Wt::any_cast<float>(value(heightField));
		_recordPtr.modify()->bloodType = BloodType(Wt::any_cast<int>(value(bloodTypeField)));
		_recordPtr.modify()->maritalStatus = MaritalStatus(Wt::any_cast<int>(value(marriedField)));

		auto nextOfKinEntity = Wt::any_cast<Dbo::ptr<Entity>>(value(nextOfKinField));
		auto fatherEntity = Wt::any_cast<Dbo::ptr<Entity>>(value(fatherField));
		auto motherEntity = Wt::any_cast<Dbo::ptr<Entity>>(value(motherField));
		_recordPtr.modify()->nextOfKinOfPtr = nextOfKinEntity ? nextOfKinEntity->personWPtr : Dbo::ptr<Person>();
		_recordPtr.modify()->fatherPersonPtr = fatherEntity ? fatherEntity->personWPtr : Dbo::ptr<Person>();
		_recordPtr.modify()->motherPersonPtr = motherEntity ? motherEntity->personWPtr : Dbo::ptr<Person>();

		_recordPtr.modify()->remarks = valueText(remarksField).toUTF8();

		auto profileUpload = _view->resolve<ImageUpload*>(profileUploadField);
		if(!profileUpload->fileUpload()->canUpload())
			profileUpload->fileUpload()->upload();
		if(profileUpload->saveAndRelocate(_recordPtr->entityPtr(), "Profile picture"))
			_recordPtr.modify()->profilePictureFilePtr = profileUpload->imageInfo().filePtr;
		setValue(profileUploadField, profileUpload->imageInfo());

		auto cnicUpload1 = _view->resolve<ImageUpload*>(cnicUploadField);
		if(!cnicUpload1->fileUpload()->canUpload())
			cnicUpload1->fileUpload()->upload();
		if(cnicUpload1->saveAndRelocate(_recordPtr->entityPtr(), "CNIC front picture"))
			_recordPtr.modify()->cnicFile1Ptr = cnicUpload1->imageInfo().filePtr;
		setValue(cnicUploadField, cnicUpload1->imageInfo());

		auto cnicUpload2 = _view->resolve<ImageUpload*>(cnicUpload2Field);
		if(!cnicUpload2->fileUpload()->canUpload())
			cnicUpload2->fileUpload()->upload();
		if(cnicUpload2->saveAndRelocate(_recordPtr->entityPtr(), "CNIC back picture"))
			_recordPtr.modify()->cnicFile2Ptr = cnicUpload2->imageInfo().filePtr;
		setValue(cnicUpload2Field, cnicUpload2->imageInfo());

		t.commit();
		return true;
	}

	//EMPLOYEE MODEL
	const Wt::WFormModel::Field EmployeeFormModel::companyNumberField = "companyNumber";
	const Wt::WFormModel::Field EmployeeFormModel::gradeField = "grade";
	const Wt::WFormModel::Field EmployeeFormModel::recruitmentDateField = "recruitmentDate";
	const Wt::WFormModel::Field EmployeeFormModel::educationField = "education";
	const Wt::WFormModel::Field EmployeeFormModel::experienceField = "experience";
	const Wt::WFormModel::Field EmployeeFormModel::addQualificationsField = "addQualifications";

	EmployeeFormModel::EmployeeFormModel(EntityView *view, Dbo::ptr<Employee> employeePtr)
		: ChildRecordFormModel(view->_entityModel, view, employeePtr), _view(view)
	{
		addField(companyNumberField);
		addField(gradeField);
		addField(recruitmentDateField);
		addField(educationField);
		addField(experienceField);
		addField(addQualificationsField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(companyNumberField, Wt::WString::fromUTF8(_recordPtr->companyNumber));
			setValue(gradeField, Wt::WString::fromUTF8(_recordPtr->grade));
			setValue(recruitmentDateField, _recordPtr->recruitmentDate);
			setValue(educationField, Wt::WString::fromUTF8(_recordPtr->education));
			setValue(experienceField, Wt::WString::fromUTF8(_recordPtr->experience));
			setValue(addQualificationsField, Wt::WString::fromUTF8(_recordPtr->addtionalQualifications));
		}
	}

	unique_ptr<Wt::WWidget> EmployeeFormModel::createFormWidget(Field field)
	{
		if(field == companyNumberField || field == gradeField)
		{
			auto w = make_unique<Wt::WLineEdit>();
			w->setMaxLength(35);
			setValidator(field, make_shared<Wt::WLengthValidator>(0, 35));
			return w;
		}
		if(field == recruitmentDateField)
		{
			auto w = make_unique<Wt::WDateEdit>();
			w->setPlaceholderText(WApplication::instance()->locale().dateFormat());
			w->setDate(Wt::WDate::currentServerDate());
			auto validator = make_shared<Wt::WDateValidator>();
			validator->setMandatory(true);
			setValidator(field, validator);
			return w;
		}
		if(field == experienceField || field == addQualificationsField || field == educationField)
		{
			auto w = make_unique<Wt::WTextArea>();
			w->setRows(3);
			return w;
		}
		return ChildRecordFormModel::createFormWidget(field);
	}

	bool EmployeeFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);
		if(!_recordPtr)
			_recordPtr = app->dboSession().addNew<Employee>();
		if(!_recordPtr->personPtr())
			_recordPtr.modify()->_personPtr = _view->_personModel->recordPtr();

		_recordPtr->personPtr()->entityPtr().modify()->specificTypeMask |= Entity::EmployeeType;
		_recordPtr.modify()->companyNumber = valueText(companyNumberField).toUTF8();
		_recordPtr.modify()->grade = valueText(gradeField).toUTF8();
		_recordPtr.modify()->recruitmentDate = Wt::any_cast<Wt::WDate>(value(recruitmentDateField));
		_recordPtr.modify()->education = valueText(educationField).toUTF8();
		_recordPtr.modify()->experience = valueText(experienceField).toUTF8();
		_recordPtr.modify()->addtionalQualifications = valueText(addQualificationsField).toUTF8();

		t.commit();
		return true;
	}

	//PERSONNEL MODEL
	const Wt::WFormModel::Field PersonnelFormModel::policeStationField = "policeStation";
	const Wt::WFormModel::Field PersonnelFormModel::policeVerifiedField = "policeVerified";
	const Wt::WFormModel::Field PersonnelFormModel::trainingCoursesField = "trainingCourses";
	const Wt::WFormModel::Field PersonnelFormModel::armyNumberField = "armyNumber";
	const Wt::WFormModel::Field PersonnelFormModel::rankField = "rank";

	PersonnelFormModel::PersonnelFormModel(EntityView *view, Dbo::ptr<Personnel> personnelPtr)
		: ChildRecordFormModel(view->_entityModel, view, personnelPtr), _view(view)
	{
		addField(policeStationField);
		addField(policeVerifiedField);
		addField(trainingCoursesField);
		addField(armyNumberField);
		addField(rankField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(policeStationField, Wt::WString::fromUTF8(_recordPtr->policeStation));
			setValue(policeVerifiedField, _recordPtr->policeVerified);
			setValue(trainingCoursesField, Wt::WString::fromUTF8(_recordPtr->trainingCourses));
			setValue(armyNumberField, Wt::WString::fromUTF8(_recordPtr->armyNumber));
			setValue(rankField, Wt::WString::fromUTF8(_recordPtr->rank));
		}
	}

	unique_ptr<Wt::WWidget> PersonnelFormModel::createFormWidget(Field field)
	{
		if(field == policeStationField)
		{
			auto w = make_unique<Wt::WLineEdit>();
			w->setMaxLength(70);
			setValidator(field, make_shared<Wt::WLengthValidator>(0, 70));
			return w;
		}
		if(field == policeVerifiedField)
		{
			return make_unique<Wt::WCheckBox>();
		}
		if(field == trainingCoursesField)
		{
			auto w = make_unique<Wt::WTextArea>();
			w->setRows(3);
			return w;
		}
		if(field == armyNumberField || field == rankField)
		{
			auto w = make_unique<Wt::WLineEdit>();
			w->setMaxLength(35);
			setValidator(field, make_shared<Wt::WLengthValidator>(0, 35));
			return w;
		}
		return ChildRecordFormModel::createFormWidget(field);
	}

	bool PersonnelFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);
		if(!_recordPtr)
			_recordPtr = app->dboSession().addNew<Personnel>();
		if(!_recordPtr->employeePtr())
			_recordPtr.modify()->_employeePtr = _view->_employeeModel->recordPtr();

		_recordPtr->employeePtr()->personPtr()->entityPtr().modify()->specificTypeMask |= Entity::PersonnelType;
		_recordPtr.modify()->policeStation = valueText(policeStationField).toUTF8();
		_recordPtr.modify()->policeVerified = Wt::any_cast<bool>(value(policeVerifiedField));
		_recordPtr.modify()->armyNumber = valueText(armyNumberField).toUTF8();
		_recordPtr.modify()->rank = valueText(rankField).toUTF8();
		_recordPtr.modify()->trainingCourses = valueText(trainingCoursesField).toUTF8();

		t.commit();
		return true;
	}

	//CONTACT NUMBER MODEL
	template<>
	const Wt::WFormModel::Field MultipleRecordModel<ContactNumber>::field = "contactNumbers";

	ContactNumbersManagerModel::ContactNumbersManagerModel(EntityView *view, PtrCollection collection)
		: MultipleRecordModel(view, collection), _view(view)
	{
		const PtrVector &vec = Wt::any_cast<PtrVector>(value(field));
		if(vec.empty())
		{
			PtrVector newVec;
			newVec.push_back(nullptr);
			setValue(field, newVec);
		}
	}

	bool ContactNumbersManagerModel::saveChanges()
	{
		for(const auto &model : _modelVector)
		{
			model->setValue(ContactNumberFormModel::entityField, _view->entityPtr());
		}
		return MultipleRecordModel::saveChanges();
	}

	unique_ptr<Wt::WWidget> ContactNumbersManagerModel::createFormWidget(Field f)
	{
		if(f == field)
		{
			auto w = MultipleRecordModel::createContainer();
			auto btn = _view->bindNew<ShowEnabledButton>("add-contact-number");
			btn->setStyleClass("fa fa-plus");
			btn->clicked().connect(w.get(), std::bind(&RecordViewsContainer::addRecordView, w.get()));
			return w;
		}
		return nullptr;
	}

	tuple<unique_ptr<RecordFormView>, ContactNumbersManagerModel::ModelType*> ContactNumbersManagerModel::createRecordView(Dbo::ptr<RecordDbo> recordPtr)
	{
		auto view = make_unique<ContactNumberView>(recordPtr);
		view->load();
		view->model()->setVisible(ContactNumberFormModel::entityField, false);
		view->model()->setReadOnly(ContactNumberFormModel::entityField, true);
		view->model()->setValue(ContactNumberFormModel::entityField, _view->entityPtr());
		view->updateViewField(view->model(), ContactNumberFormModel::entityField);

		auto model = view->model();
		return std::make_tuple(move(view), model);
	}

	//LOCATIONS MANAGER MODEL
	template<>
	const Wt::WFormModel::Field MultipleRecordModel<Location>::field = "locations";

	LocationsManagerModel::LocationsManagerModel(EntityView *view, PtrCollection collection)
		: MultipleRecordModel(view, collection), _view(view)
	{
		const PtrVector &vec = Wt::any_cast<PtrVector>(value(field));
		if(vec.empty())
		{
			PtrVector newVec;
			newVec.push_back(nullptr);
			setValue(field, newVec);
		}
	}

	bool LocationsManagerModel::saveChanges()
	{
		for(const auto &model : _modelVector)
			model->setValue(LocationFormModel::entityField, _view->entityPtr());

		return MultipleRecordModel::saveChanges();
	}

	unique_ptr<Wt::WWidget> LocationsManagerModel::createFormWidget(Field f)
	{
		if(f == field)
		{
			auto w = MultipleRecordModel::createContainer();
			auto btn = _view->bindNew<ShowEnabledButton>("add-location");
			btn->setStyleClass("fa fa-plus");
			btn->clicked().connect(w.get(), std::bind(&RecordViewsContainer::addRecordView, w.get()));
			return w;
		}
		return nullptr;
	}

	tuple<unique_ptr<RecordFormView>, LocationsManagerModel::ModelType*> LocationsManagerModel::createRecordView(Dbo::ptr<RecordDbo> recordPtr)
	{
		auto view = make_unique<LocationView>(recordPtr);
		view->load();
		view->setTemplateText(tr("ERP.Admin.LocationView.Content"));
		view->model()->setVisible(LocationFormModel::entityField, false);
		view->model()->setReadOnly(LocationFormModel::entityField, true);
		view->model()->setValue(LocationFormModel::entityField, _view->entityPtr());
		view->updateViewField(view->model(), LocationFormModel::entityField);

		auto model = view->model();
		return std::make_tuple(move(view), model);
	}

	//BUSINESS MODEL
	BusinessFormModel::BusinessFormModel(EntityView *view, Dbo::ptr<Business> businessPtr)
		: ChildRecordFormModel(view->_entityModel, view, businessPtr), _view(view)
	{ }

	unique_ptr<Wt::WWidget> BusinessFormModel::createFormWidget(Field field)
	{
		return ChildRecordFormModel::createFormWidget(field);
	}

	bool BusinessFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = WApplication::instance();
		TRANSACTION(app);

		if(!_recordPtr)
			_recordPtr = app->dboSession().addNew<Business>();
		if(!_recordPtr->entityPtr())
			_recordPtr.modify()->_entityPtr = _view->entityPtr();
		_recordPtr->entityPtr().modify()->type = Entity::BusinessType;

		t.commit();
		return true;
	}

	//NEW ENTITIY VIEW
	EntityView::EntityView(Entity::Type type)
		: RecordFormView(tr("ERP.Admin.Entities.New")), _type(type), _defaultType(type)
	{ }

	EntityView::EntityView(Dbo::ptr<Entity> entityPtr)
		: RecordFormView(tr("ERP.Admin.Entities.New")), _tempPtr(entityPtr)
	{ }

	void EntityView::initView()
	{
		_entityModel = newFormModel<EntityFormModel>("entity", this, _tempPtr);

		if(entityPtr())
		{
			TRANSACTION(APP);
			if(entityPtr()->type == Entity::PersonType)
			{
				_type = _defaultType = Entity::PersonType;

				Dbo::ptr<Person> personPtr = entityPtr()->personWPtr;
				Dbo::ptr<Employee> employeePtr;
				Dbo::ptr<Personnel> personnelPtr;
				if(personPtr)
					employeePtr = personPtr->employeeWPtr;
				if(employeePtr)
					personnelPtr = employeePtr->personnelWPtr;

				if(personPtr)
					_personModel = newFormModel<PersonFormModel>("person", this, personPtr);
				if(employeePtr)
					addEmployeeModel(employeePtr);
				if(personnelPtr)
					addPersonnelModel(personnelPtr);
			}
			if(entityPtr()->type == Entity::BusinessType)
			{
				_type = _defaultType = Entity::BusinessType;
				Dbo::ptr<Business> businessPtr = entityPtr()->businessWPtr;
				if(businessPtr)
					_businessModel = newFormModel<BusinessFormModel>("business", this, businessPtr);
			}
		}

		setCondition("type-chosen", true);
		switch(_type)
		{
		case Entity::InvalidType:
			setCondition("type-chosen", false);
			break;
		case Entity::PersonType:
			setCondition("is-person", true);
			if(!_personModel) _personModel = newFormModel<PersonFormModel>("person", this);
			break;
		case Entity::BusinessType:
			setCondition("is-business", true);
			if(!_businessModel) _businessModel = newFormModel<BusinessFormModel>("business",  this);
			break;
		default:
			throw std::runtime_error("EntityView: Invalid EntityType");
		}
		setCondition("type-selection", !conditionValue("type-chosen"));

		_selectPerson = bindNew<Wt::WPushButton>("selectPerson", tr("Person"));
		_selectPerson->clicked().connect(this, std::bind(&EntityView::selectEntityType, this, Entity::PersonType));

		_selectBusiness = bindNew<Wt::WPushButton>("selectBusiness", tr("Business"));
		_selectBusiness->clicked().connect(this, std::bind(&EntityView::selectEntityType, this, Entity::BusinessType));

		auto addEmployee = bindNew<ShowEnabledButton>("add-employee", tr("AddEmployeeLabel"));
		addEmployee->clicked().connect(this, std::bind(&EntityView::setSpecificType, this, Entity::EmployeeType));
		if(_employeeModel)
			addEmployee->disable();

		auto addPersonnel = bindNew<ShowEnabledButton>("add-personnel", tr("AddPersonnelLabel"));
		addPersonnel->clicked().connect(this, std::bind(&EntityView::setSpecificType, this, Entity::PersonnelType));
		if(_personnelModel)
			addPersonnel->disable();

		_contactNumbersModel = newFormModel<ContactNumbersManagerModel>("contactnumbers",
				this, entityPtr() ? entityPtr()->contactNumberCollection : ContactNumberCollection());
		_locationsModel = newFormModel<LocationsManagerModel>("locations",
				this, entityPtr() ? entityPtr()->locationCollection : LocationCollection());
		
		bindEmpty("expenseCycles");
		setCondition("show-expenseCycles", false);
		bindEmpty("incomeCycles");
		setCondition("show-incomeCycles", false);
	}

	void EntityView::selectEntityType(Entity::Type type)
	{
		switch(type)
		{
		case Entity::InvalidType:
			return;
		case Entity::PersonType:
			setCondition("is-person", true);
			setCondition("is-business", false);
			if(!_personModel) _personModel = newFormModel<PersonFormModel>("person", this);
			_selectPerson->addStyleClass("btn-primary");
			_selectBusiness->removeStyleClass("btn-primary");
			break;
		case Entity::BusinessType:
			setCondition("is-business", true);
			setCondition("is-person", false);
			if(!_businessModel) _businessModel = newFormModel<BusinessFormModel>("business", this);
			_selectBusiness->addStyleClass("btn-primary");
			_selectPerson->removeStyleClass("btn-primary");
			break;
		}
		setCondition("type-chosen", true);
		_type = type;

		updateModel();
		updateView();
	}

	Wt::WString EntityView::viewName() const
	{
		return _entityModel ? _entityModel->valueText(EntityFormModel::nameField) : "EntityView";
	}

	void EntityView::setSpecificType(Entity::SpecificType type)
	{
		if(type == Entity::UnspecificType)
			return;

		if(type == Entity::EmployeeType)
		{
			addEmployeeModel();
			updateModel(_employeeModel);
			updateView(_employeeModel);
		}
		if(type == Entity::PersonnelType)
		{
			addPersonnelModel();
			updateModel(_employeeModel);
			updateView(_employeeModel);
			updateModel(_personnelModel);
			updateView(_personnelModel);
		}
	}

	void EntityView::addEmployeeModel(Dbo::ptr<Employee> employeePtr)
{
		if(_employeeModel || _type != Entity::PersonType)
			return;

		if(!_personModel)
			_personModel = newFormModel<PersonFormModel>("person", this);

	_employeeModel = newFormModel<EmployeeFormModel>("employee", this, employeePtr);

		if(auto btn = resolveWidget("add-employee"))
			btn->disable();
	}

	void EntityView::addPersonnelModel(Dbo::ptr<Personnel> personnelPtr)
	{
		if(_personnelModel || _type != Entity::PersonType)
			return;

		if(!_employeeModel)
			addEmployeeModel();

		_personnelModel = newFormModel<PersonnelFormModel>("personnel", this, personnelPtr);

		if(auto btn = resolveWidget("add-personnel"))
			btn->disable();
	}

	void EntityView::submit()
	{
		if(_type == Entity::InvalidType)
			return;

		RecordFormView::submit();
	}

	void EntityView::afterSubmitHandler()
	{
		setCondition("type-selection", false);
	}

	HeightEdit::HeightEdit()
		: Wt::WTemplate(tr("ERP.HeightEdit"))
	{
		Wt::WPushButton *unitSelect = bindNew<Wt::WPushButton>("unit-select");
		auto unitMenu = make_unique<Wt::WPopupMenu>();
		unitMenu->addItem("cm")->clicked().connect(this, std::bind(&HeightEdit::selectUnit, this, cm));
		unitMenu->addItem("ft")->clicked().connect(this, std::bind(&HeightEdit::selectUnit, this, ft));
		unitSelect->setMenu(move(unitMenu));

		Wt::WLineEdit *cmEdit = bindNew<Wt::WLineEdit>("cm-edit");
		cmEdit->setValidator(make_shared<Wt::WIntValidator>());

		Wt::WLineEdit *ftEdit = bindNew<Wt::WLineEdit>("ft-edit");
		ftEdit->setValidator(make_shared<Wt::WIntValidator>());

		Wt::WLineEdit *inEdit = bindNew<Wt::WLineEdit>("in-edit");
		inEdit->setValidator(make_shared<Wt::WIntValidator>());

		setUnit(ft);
	}

	void HeightEdit::setUnit(Unit unit)
	{
		_unit = unit;
		if(unit == ft)
		{
			resolve<Wt::WPushButton*>("unit-select")->setText("ft");
			setCondition("is-ft", true);
			setCondition("is-cm", false);
		}
		else
		{
			resolve<Wt::WPushButton*>("unit-select")->setText("cm");
			setCondition("is-cm", true);
			setCondition("is-ft", false);
		}
		setLabelBuddy(parent());
	}

	float HeightEdit::valueInCm()
	{
		const Wt::WLocale& locale = Wt::WLocale::currentLocale();
		if(_unit == ft)
		{
			Wt::WLineEdit *ftEdit = resolve<Wt::WLineEdit*>("ft-edit");
			Wt::WLineEdit *inEdit = resolve<Wt::WLineEdit*>("in-edit");

			if(ftEdit->valueText().empty())
				return -1;

			double ftVal = locale.toDouble(ftEdit->valueText());
			double inVal = 0;
			if(!inEdit->valueText().empty())
				inVal = locale.toDouble(inEdit->valueText());

			return static_cast<float>(ftVal * 30.48 + inVal * 2.54);
		}
		else
		{
			Wt::WLineEdit *cmEdit = resolve<Wt::WLineEdit*>("cm-edit");
			if(cmEdit->valueText().empty())
				return -1;

			return static_cast<float>(locale.toDouble(cmEdit->valueText()));
		}
	}

	void HeightEdit::setValueInCm(float val)
	{
		WApplication *app = APP;
		if(_unit == ft)
		{
			Wt::WLineEdit *ftEdit = resolve<Wt::WLineEdit*>("ft-edit");
			Wt::WLineEdit *inEdit = resolve<Wt::WLineEdit*>("in-edit");

			if(val == -1)
			{
				ftEdit->setValueText("");
				inEdit->setValueText("");
			}
			else
			{
				long long ftVal = (long long)std::floor(val / 30.48f);
				float inVal = std::round((val / 30.48f - ftVal) * 12);
				ftEdit->setValueText(app->locale().toString(ftVal));
				inEdit->setValueText(app->locale().toString(inVal));
			}
		}
		else
		{
			Wt::WLineEdit *cmEdit = resolve<Wt::WLineEdit*>("cm-edit");
			if(val == -1)
				cmEdit->setValueText("");
			else
				cmEdit->setValueText(app->locale().toString(val));
		}
	}

	void HeightEdit::setLabelBuddy(Wt::WWidget *templateView)
	{
		if(auto *newEntityView = dynamic_cast<EntityView*>(templateView))
		{
			if(_unit == ft)
				newEntityView->bindString("heightLabelFor", resolve<Wt::WLineEdit*>("ft-edit")->id());
			else
				newEntityView->bindString("heightLabelFor", resolve<Wt::WLineEdit*>("cm-edit")->id());
		}
	}

	const Wt::WFormModel::Field ContactNumberFormModel::entityField = "entity";
	const Wt::WFormModel::Field ContactNumberFormModel::numberField = "number";

	ContactNumberFormModel::ContactNumberFormModel(ContactNumberView *view, Dbo::ptr<ContactNumber> countryPtr)
		: RecordFormModel(view, countryPtr), _view(view)
	{
		addField(entityField);
		addField(numberField);

		if(_recordPtr)
		{
			TRANSACTION(APP);
			setValue(entityField, _recordPtr->entityPtr);
			setValue(numberField, Wt::WString::fromUTF8(_recordPtr->nationalNumber));
		}
	}

	unique_ptr<Wt::WWidget> ContactNumberFormModel::createFormWidget(Field field)
	{
		if(field == numberField)
		{
			return make_unique<Wt::WLineEdit>();
		}
		if(field == entityField)
		{
			auto w = make_unique<FindEntityEdit>();
			auto validator = make_shared<FindEntityValidator>(w.get(), false);
			validator->setModifyPermissionRequired(true);
			setValidator(entityField, validator);
			return w;
		}
		return RecordFormModel::createFormWidget(field);
	}

	bool ContactNumberFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		Wt::WString number = valueText(numberField);

		if(!_recordPtr)
		{
			if(number.empty())
			{
				return false;
			}
			_recordPtr = app->dboSession().addNew<ContactNumber>();
			//_recordPtr.modify()->setCreatedByValues();
		}
		else
		{
			if(number.empty())
			{
				_recordPtr.remove();
				t.commit();
				return true;
			}
		}

		_recordPtr.modify()->entityPtr = Wt::any_cast<Dbo::ptr<Entity>>(value(entityField));
		_recordPtr.modify()->nationalNumber = number.toUTF8();

		t.commit();
		return true;
	}

	ContactNumberView::ContactNumberView(Dbo::ptr<ContactNumber> countryPtr)
		: RecordFormView(tr("ERP.Admin.ContactNumberView")), _tempPtr(countryPtr)
	{ }

	ContactNumberView::ContactNumberView()
		: RecordFormView(tr("ERP.Admin.ContactNumberView"))
	{ }

	void ContactNumberView::initView()
	{
		_model = newFormModel<ContactNumberFormModel>("contactnumber", this, _tempPtr);
	}
}
