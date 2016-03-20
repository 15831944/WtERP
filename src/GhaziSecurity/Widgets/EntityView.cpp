#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Widgets/AdminPages.h"
#include "Widgets/ImageUpload.h"
#include "Widgets/LocationMVC.h"
#include "Widgets/EntryCycleMVC.h"
#include "Utilities/FindRecordEdit.h"

#include <Wt/WLabel>
#include <Wt/WBreak>
#include <Wt/WMenu>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WDateEdit>
#include <Wt/WTextArea>
#include <Wt/WPopupMenu>
#include <Wt/WComboBox>
#include <Wt/WCheckBox>
#include <Wt/WFileUpload>
#include <Wt/WImage>
#include <Wt/WIntValidator>

#include <Wt/Dbo/QueryModel>

#include <boost/algorithm/string.hpp>

namespace GS
{
	//ENTITY MODEL
	const Wt::WFormModel::Field EntityFormModel::nameField = "name";

	EntityFormModel::EntityFormModel(EntityView *view, Wt::Dbo::ptr<Entity> entityPtr /*= Wt::Dbo::ptr<Entity>()*/)
		: Wt::WFormModel(view), _view(view), _entityPtr(entityPtr)
	{
		addField(nameField);

		if(entityPtr)
			setValue(nameField, Wt::WString::fromUTF8(entityPtr->name));
	}

	Wt::WWidget *EntityFormModel::createFormWidget(Field field)
	{
		if(field == nameField)
		{
			auto w = new Wt::WLineEdit();
			w->setMaxLength(70);
			auto validator = new Wt::WLengthValidator(0, 70);
			validator->setMandatory(true);
			setValidator(field, validator);
			return w;
		}
		return nullptr;
	}

	void EntityFormModel::saveChanges()
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		if(!_entityPtr)
			_entityPtr = app->session().add(new Entity());

		_entityPtr.modify()->specificTypeMask = 0;
		_entityPtr.modify()->name = valueText(nameField).toUTF8();
		t.commit();
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

	PersonFormModel::PersonFormModel(EntityView *view, Wt::Dbo::ptr<Person> personPtr)
		: Wt::WFormModel(view), _view(view), _personPtr(personPtr)
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

		if(personPtr)
		{
			setValue(dobField, personPtr->dateOfBirth);
			setValue(cnicField, Wt::WString::fromUTF8(personPtr->cnicNumber));
			setValue(motherTongueField, Wt::WString::fromUTF8(personPtr->motherTongue));
			setValue(identificationMarkField, Wt::WString::fromUTF8(personPtr->identificationMark));
			setValue(heightField, personPtr->height);
			setValue(bloodTypeField, (int)personPtr->bloodType);
			setValue(marriedField, (int)personPtr->maritalStatus);
			setValue(nextOfKinField, personPtr->nextOfKinOfPtr.id());
			setValue(fatherField, personPtr->fatherPersonPtr.id());
			setValue(motherField, personPtr->motherPersonPtr.id());
			setValue(remarksField, Wt::WString::fromUTF8(personPtr->remarks));

			if(personPtr->profilePictureFilePtr) setValue(profileUploadField, UploadedImage(personPtr->profilePictureFilePtr));
			if(personPtr->cnicFile1Ptr) setValue(cnicUploadField, UploadedImage(personPtr->cnicFile1Ptr));
			if(personPtr->cnicFile2Ptr) setValue(cnicUpload2Field, UploadedImage(personPtr->cnicFile2Ptr));
		}
	}

	Wt::WWidget *PersonFormModel::createFormWidget(Field field)
	{
		if(field == dobField)
		{
			auto w = new Wt::WDateEdit();
			w->setPlaceholderText(WApplication::instance()->locale().dateFormat());
			auto validator = new Wt::WDateValidator();
			validator->setTop(Wt::WDate(boost::gregorian::day_clock::local_day()));
			setValidator(field, validator);
			return w;
		}
		if(field == cnicField)
		{
			auto w = new Wt::WLineEdit();
			w->setInputMask("99999-9999999-9");
			return w;
		}
		if(field == motherTongueField)
		{
			auto w = new Wt::WLineEdit();
			w->setMaxLength(70);
			setValidator(field, new Wt::WLengthValidator(0, 70));
			return w;
		}
		if(field == identificationMarkField)
		{
			auto w = new Wt::WLineEdit();
			w->setMaxLength(255);
			setValidator(field, new Wt::WLengthValidator(0, 255));
			return w;
		}
		if(field == heightField)
		{
			auto w = new HeightEdit();
			w->setLabelBuddy(_view);
			return w;
		}
		if(field == bloodTypeField)
		{
			auto w = new Wt::WComboBox();
			w->insertItem(UnknownBT, Wt::boost_any_traits<BloodType>::asString(UnknownBT, ""));
			w->insertItem(OPositive, Wt::boost_any_traits<BloodType>::asString(OPositive, ""));
			w->insertItem(ONegative, Wt::boost_any_traits<BloodType>::asString(ONegative, ""));
			w->insertItem(APositive, Wt::boost_any_traits<BloodType>::asString(APositive, ""));
			w->insertItem(ANegative, Wt::boost_any_traits<BloodType>::asString(ANegative, ""));
			w->insertItem(BPositive, Wt::boost_any_traits<BloodType>::asString(BPositive, ""));
			w->insertItem(BNegative, Wt::boost_any_traits<BloodType>::asString(BNegative, ""));
			w->insertItem(ABPositive, Wt::boost_any_traits<BloodType>::asString(ABPositive, ""));
			w->insertItem(ABNegative, Wt::boost_any_traits<BloodType>::asString(ABNegative, ""));
			return w;
		}
		if(field == marriedField)
		{
			auto w = new Wt::WComboBox();
			w->insertItem(UnknownMS, Wt::boost_any_traits<MaritalStatus>::asString(UnknownMS, ""));
			w->insertItem(Married, Wt::boost_any_traits<MaritalStatus>::asString(Married, ""));
			w->insertItem(Unmarried, Wt::boost_any_traits<MaritalStatus>::asString(Unmarried, ""));
			return w;
		}
		if(field == nextOfKinField)
		{
			auto w = new FindEntityEdit(Entity::PersonType);
			setValidator(field, new FindEntityValidator(w, false));
			return w;
		}
		if(field == fatherField)
		{
			auto w = new FindEntityEdit(Entity::PersonType);
			setValidator(field, new FindEntityValidator(w, false));
			return w;
		}
		if(field == motherField)
		{
			auto w = new FindEntityEdit(Entity::PersonType);
			setValidator(field, new FindEntityValidator(w, false));
			return w;
		}
		if(field == remarksField)
		{
			auto w = new Wt::WTextArea();
			w->setRows(3);
			return w;
		}
		if(field == profileUploadField)
		{
			auto w = new ImageUpload(Wt::WString::tr("ClickToUploadProfile"), Wt::WString::tr("ClickToChangeProfile"));
			w->setPlaceholderImageLink(Wt::WLink("images/profile-placeholder.png"));
			w->setThumbnailHeight(160);
			return w;
		}
		if(field == cnicUploadField)
		{
			auto w = new ImageUpload(Wt::WString::tr("ClickToUploadCNIC"), Wt::WString::tr("ClickToChangeCNIC"));
			w->setThumbnailHeight(160);
			return w;
		}
		if(field == cnicUpload2Field)
		{
			auto w = new ImageUpload(Wt::WString::tr("ClickToUploadCNIC"), Wt::WString::tr("ClickToChangeCNIC"));
			w->setThumbnailHeight(160);
			return w;
		}
		return nullptr;
	}

	void PersonFormModel::saveChanges()
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());
		if(!_personPtr)
			_personPtr = app->session().add(new Person());
		if(!_personPtr->entityPtr())
			_personPtr.modify()->_entityPtr = _view->_entityModel->entityPtr();
		_personPtr->entityPtr().modify()->type = Entity::PersonType;

		_personPtr.modify()->dateOfBirth = boost::any_cast<Wt::WDate>(value(dobField));

		std::string cnicNumber = valueText(cnicField).toUTF8();
		boost::erase_all(cnicNumber, "-");
		_personPtr.modify()->cnicNumber = cnicNumber;

		_personPtr.modify()->motherTongue = valueText(motherTongueField).toUTF8();
		_personPtr.modify()->identificationMark = valueText(identificationMarkField).toUTF8();
		_personPtr.modify()->height = boost::any_cast<float>(value(heightField));
		_personPtr.modify()->bloodType = BloodType(boost::any_cast<int>(value(bloodTypeField)));
		_personPtr.modify()->maritalStatus = MaritalStatus(boost::any_cast<int>(value(marriedField)));

		auto nextOfKinEntity = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(nextOfKinField));
		auto fatherEntity = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(fatherField));
		auto motherEntity = boost::any_cast<Wt::Dbo::ptr<Entity>>(value(motherField));
		_personPtr.modify()->nextOfKinOfPtr = nextOfKinEntity ? nextOfKinEntity->personWPtr : Wt::Dbo::ptr<Person>();
		_personPtr.modify()->fatherPersonPtr = fatherEntity ? fatherEntity->personWPtr : Wt::Dbo::ptr<Person>();
		_personPtr.modify()->motherPersonPtr = motherEntity ? motherEntity->personWPtr : Wt::Dbo::ptr<Person>();

		_personPtr.modify()->remarks = valueText(remarksField).toUTF8();

		auto profileUpload = _view->resolve<ImageUpload*>(profileUploadField);
		if(!profileUpload->fileUpload()->canUpload())
			profileUpload->fileUpload()->upload();
		if(profileUpload->saveAndRelocate(_personPtr->entityPtr(), "Profile picture"))
			_personPtr.modify()->profilePictureFilePtr = profileUpload->imageInfo().filePtr;
		setValue(profileUploadField, profileUpload->imageInfo());

		auto cnicUpload1 = _view->resolve<ImageUpload*>(cnicUploadField);
		if(!cnicUpload1->fileUpload()->canUpload())
			cnicUpload1->fileUpload()->upload();
		if(cnicUpload1->saveAndRelocate(_personPtr->entityPtr(), "CNIC front picture"))
			_personPtr.modify()->cnicFile1Ptr = cnicUpload1->imageInfo().filePtr;
		setValue(cnicUploadField, cnicUpload1->imageInfo());

		auto cnicUpload2 = _view->resolve<ImageUpload*>(cnicUpload2Field);
		if(!cnicUpload2->fileUpload()->canUpload())
			cnicUpload2->fileUpload()->upload();
		if(cnicUpload2->saveAndRelocate(_personPtr->entityPtr(), "CNIC back picture"))
			_personPtr.modify()->cnicFile2Ptr = cnicUpload2->imageInfo().filePtr;
		setValue(cnicUpload2Field, cnicUpload2->imageInfo());

		t.commit();
	}

	//EMPLOYEE MODEL
	const Wt::WFormModel::Field EmployeeFormModel::companyNumberField = "companyNumber";
	const Wt::WFormModel::Field EmployeeFormModel::gradeField = "grade";
	const Wt::WFormModel::Field EmployeeFormModel::recruitmentDateField = "recruitmentDate";
	const Wt::WFormModel::Field EmployeeFormModel::educationField = "education";
	const Wt::WFormModel::Field EmployeeFormModel::experienceField = "experience";
	const Wt::WFormModel::Field EmployeeFormModel::addQualificationsField = "addQualifications";

	EmployeeFormModel::EmployeeFormModel(EntityView *view, Wt::Dbo::ptr<Employee> employeePtr)
		: Wt::WFormModel(view), _view(view), _employeePtr(employeePtr)
	{
		addField(companyNumberField);
		addField(gradeField);
		addField(recruitmentDateField);
		addField(educationField);
		addField(experienceField);
		addField(addQualificationsField);

		if(employeePtr)
		{
			setValue(companyNumberField, Wt::WString::fromUTF8(employeePtr->companyNumber));
			setValue(gradeField, Wt::WString::fromUTF8(employeePtr->grade));
			setValue(recruitmentDateField, employeePtr->recruitmentDate);
			setValue(educationField, Wt::WString::fromUTF8(employeePtr->education));
			setValue(experienceField, Wt::WString::fromUTF8(employeePtr->experience));
			setValue(addQualificationsField, Wt::WString::fromUTF8(employeePtr->addtionalQualifications));
		}
	}

	Wt::WWidget *EmployeeFormModel::createFormWidget(Field field)
	{
		if(field == companyNumberField || field == gradeField)
		{
			auto w = new Wt::WLineEdit();
			w->setMaxLength(35);
			setValidator(field, new Wt::WLengthValidator(0, 35));
			return w;
		}
		if(field == recruitmentDateField)
		{
			auto w = new Wt::WDateEdit();
			w->setPlaceholderText(WApplication::instance()->locale().dateFormat());
			w->setDate(Wt::WDate(boost::gregorian::day_clock::local_day()));
			auto validator = new Wt::WDateValidator();
			validator->setMandatory(true);
			setValidator(field, validator);
			return w;
		}
		if(field == experienceField || field == addQualificationsField || field == educationField)
		{
			auto w = new Wt::WTextArea();
			w->setRows(3);
			return w;
		}
		return nullptr;
	}

	void EmployeeFormModel::saveChanges()
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());
		if(!_employeePtr)
			_employeePtr = app->session().add(new Employee());
		if(!_employeePtr->personPtr())
			_employeePtr.modify()->_personPtr = _view->_personModel->personPtr();

		_employeePtr->personPtr()->entityPtr().modify()->specificTypeMask |= Entity::EmployeeType;
		_employeePtr.modify()->companyNumber = valueText(companyNumberField).toUTF8();
		_employeePtr.modify()->grade = valueText(gradeField).toUTF8();
		_employeePtr.modify()->recruitmentDate = boost::any_cast<Wt::WDate>(value(recruitmentDateField));
		_employeePtr.modify()->education = valueText(educationField).toUTF8();
		_employeePtr.modify()->experience = valueText(experienceField).toUTF8();
		_employeePtr.modify()->addtionalQualifications = valueText(addQualificationsField).toUTF8();
		t.commit();
	}

	//PERSONNEL MODEL
	const Wt::WFormModel::Field PersonnelFormModel::policeStationField = "policeStation";
	const Wt::WFormModel::Field PersonnelFormModel::policeVerifiedField = "policeVerified";
	const Wt::WFormModel::Field PersonnelFormModel::trainingCoursesField = "trainingCourses";
	const Wt::WFormModel::Field PersonnelFormModel::armyNumberField = "armyNumber";
	const Wt::WFormModel::Field PersonnelFormModel::rankField = "rank";

	PersonnelFormModel::PersonnelFormModel(EntityView *view, Wt::Dbo::ptr<Personnel> personnelPtr)
		: Wt::WFormModel(view), _view(view), _personnelPtr(personnelPtr)
	{
		addField(policeStationField);
		addField(policeVerifiedField);
		addField(trainingCoursesField);
		addField(armyNumberField);
		addField(rankField);

		if(personnelPtr)
		{
			setValue(policeStationField, Wt::WString::fromUTF8(personnelPtr->policeStation));
			setValue(policeVerifiedField, personnelPtr->policeVerified);
			setValue(trainingCoursesField, Wt::WString::fromUTF8(personnelPtr->trainingCourses));
			setValue(armyNumberField, Wt::WString::fromUTF8(personnelPtr->armyNumber));
			setValue(rankField, Wt::WString::fromUTF8(personnelPtr->rank));
		}
	}

	Wt::WWidget * PersonnelFormModel::createFormWidget(Field field)
	{
		if(field == policeStationField)
		{
			auto w = new Wt::WLineEdit();
			w->setMaxLength(70);
			setValidator(field, new Wt::WLengthValidator(0, 70));
			return w;
		}
		if(field == policeVerifiedField)
		{
			return new Wt::WCheckBox();
		}
		if(field == trainingCoursesField)
		{
			auto w = new Wt::WTextArea();
			w->setRows(3);
			return w;
		}
		if(field == armyNumberField || field == rankField)
		{
			auto w = new Wt::WLineEdit();
			w->setMaxLength(35);
			setValidator(field, new Wt::WLengthValidator(0, 35));
			return w;
		}
		return nullptr;
	}

	void PersonnelFormModel::saveChanges()
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());
		if(!_personnelPtr)
			_personnelPtr = app->session().add(new Personnel());
		if(!_personnelPtr->employeePtr())
			_personnelPtr.modify()->_employeePtr = _view->_employeeModel->employeePtr();

		_personnelPtr->employeePtr()->personPtr()->entityPtr().modify()->specificTypeMask |= Entity::PersonnelType;
		_personnelPtr.modify()->policeStation = valueText(policeStationField).toUTF8();
		_personnelPtr.modify()->policeVerified = boost::any_cast<bool>(value(policeVerifiedField));
		_personnelPtr.modify()->armyNumber = valueText(armyNumberField).toUTF8();
		_personnelPtr.modify()->rank = valueText(rankField).toUTF8();
		_personnelPtr.modify()->trainingCourses = valueText(trainingCoursesField).toUTF8();
		t.commit();
	}

	//CONTACT NUMBER MODEL
	const Wt::WFormModel::Field ContactNumbersManagerModel::field = "contactNumbers";

	ContactNumbersManagerModel::ContactNumbersManagerModel(EntityView *view)
		: Wt::WFormModel(view), _view(view)
	{
		addField(field);

		Wt::Dbo::Transaction t(APP->session());
		if(_view->entityPtr())
		{
			_ptrVector = PtrVector(_view->entityPtr()->contactNumberCollection.begin(), _view->entityPtr()->contactNumberCollection.end());

			NumberVector numberVector;
			numberVector.resize(_ptrVector.size() + 1);
			for(size_t i = 0; i < _ptrVector.size(); ++i)
				numberVector[i] = Wt::WString::fromUTF8(_ptrVector[i]->nationalNumber);

			setValue(field, numberVector);
		}
	}

	Wt::WWidget *ContactNumbersManagerModel::createFormWidget(Field f)
	{
		if(f == field)
		{
			auto w = new ContactNumbersContainer(this);
			auto btn = new Wt::WPushButton();
			btn->setStyleClass("fa fa-plus");
			btn->clicked().connect(w, &ContactNumbersContainer::addFieldWidget);
			_view->bindWidget("add-contact-number", btn);
			return w;
		}
		return nullptr;
	}

	void ContactNumbersManagerModel::saveChanges()
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		auto &numberVector = boost::any_cast<NumberVector>(value(field));
		if(_ptrVector.size() < numberVector.size())
			_ptrVector.resize(numberVector.size());

		for(size_t i = 0; i < numberVector.size(); ++i)
		{
			std::string text = numberVector[i].toUTF8();
			if(text.empty())
			{
				_ptrVector[i].remove();
				_ptrVector[i].reset();
				continue;
			}

			if(!_ptrVector[i])
				_ptrVector[i] = app->session().add(new ContactNumber());
			_ptrVector[i].modify()->nationalNumber = text;
			_ptrVector[i].modify()->entityPtr = _view->entityPtr();
		}

		t.commit();
	}

	//LOCATIONS MANAGER MODEL
	const Wt::WFormModel::Field LocationsManagerModel::field = "locations";

	LocationsManagerModel::LocationsManagerModel(EntityView *view)
		: Wt::WFormModel(view), _view(view)
	{
		addField(field);

		if(_view->entityPtr())
			setValue(field, PtrVector(_view->entityPtr()->locationCollection.begin(), _view->entityPtr()->locationCollection.end()));
		else
			setValue(field, PtrVector());
	}

	Wt::WWidget *LocationsManagerModel::createFormWidget(Field f)
	{
		if(f == field)
		{
			auto w = new LocationsContainer(this);
			auto btn = new Wt::WPushButton();
			btn->setStyleClass("fa fa-plus");
			btn->clicked().connect(boost::bind(&LocationsContainer::addFieldWidget, w, Wt::Dbo::ptr<Location>()));
			_view->bindWidget("add-location", btn);
			return w;
		}
		return nullptr;
	}

	void LocationsManagerModel::saveChanges()
	{
		if(!valid())
			return;

		auto container = _view->resolve<LocationsContainer*>(field);
		auto vector = boost::any_cast<PtrVector>(value(field));
		if(vector.size() < container->count())
			vector.resize(container->count());

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());
		for(int i = 0; i < container->count(); ++i)
		{
			auto model = container->viewModel(i);
			model->saveChanges(vector[i], _view->entityPtr());
		}
		t.commit();
		setValue(field, vector);
	}

	bool LocationsManagerModel::validate()
	{
		auto container = _view->resolve<LocationsContainer*>(field);
		if(!container)
			return false;

		bool result = container->validateAll();
		setValidation(field, Wt::WValidator::Result(result ? Wt::WValidator::Valid : Wt::WValidator::Invalid));
		return result;
	}

	//BUSINESS MODEL
	BusinessFormModel::BusinessFormModel(EntityView *view, Wt::Dbo::ptr<Business> businessPtr)
		: Wt::WFormModel(view), _view(view), _businessPtr(businessPtr)
	{
	}

	Wt::WWidget *BusinessFormModel::createFormWidget(Field field)
	{
		return nullptr;
	}

	void BusinessFormModel::saveChanges()
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		if(!_businessPtr)
			_businessPtr = app->session().add(new Business());
		if(!_businessPtr->entityPtr())
			_businessPtr.modify()->_entityPtr = _view->_entityModel->entityPtr();
		_businessPtr->entityPtr().modify()->type = Entity::BusinessType;

		t.commit();
	}

	//NEW ENTITIY VIEW
	EntityView::EntityView(Entity::Type type, Wt::WContainerWidget *parent)
		: MyTemplateFormView(tr("GS.Admin.Entities.New"), parent), _type(type), _defaultType(type)
	{
		init();
	}

	EntityView::EntityView(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent)
		: MyTemplateFormView(tr("GS.Admin.Entities.New"), parent)
	{
		WApplication *app = APP;
		Wt::Dbo::Transaction t(app->session());

		_entityModel = new EntityFormModel(this, entityPtr);
		if(entityPtr->type == Entity::PersonType)
		{
			_type = _defaultType = Entity::PersonType;

// 			typedef Wt::Dbo::ptr_tuple<Person, Employee, Personnel>::type PersonTuple;
// 			PersonTuple result = app->session().query<PersonTuple>(
// 				"SELECT p, e, pl FROM " + std::string(Person::tableName()) + " p "
// 				"LEFT JOIN " + Employee::tableName() + " e ON e.person_entity_id = p.entity_id "
// 				"LEFT JOIN " + Personnel::tableName() + " pl ON pl.employee_person_entity_id = e.person_entity_id "
// 				"WHERE p.entity_id = ?").bind(entityPtr.id());

			Wt::Dbo::ptr<Person> personPtr = entityPtr->personWPtr;
			Wt::Dbo::ptr<Employee> employeePtr;
			Wt::Dbo::ptr<Personnel> personnelPtr;
			if(personPtr)
				employeePtr = personPtr->employeeWPtr;
			if(employeePtr)
				personnelPtr = employeePtr->personnelWPtr;

			if(personPtr)
				_personModel = new PersonFormModel(this, personPtr);
			if(employeePtr)
				addEmployeeModel(employeePtr);
			if(personnelPtr)
				addPersonnelModel(personnelPtr);
		}
		if(entityPtr->type == Entity::BusinessType)
		{
			_type = _defaultType = Entity::BusinessType;
			Wt::Dbo::ptr<Business> businessPtr = entityPtr->businessWPtr;
			if(businessPtr)
				_businessModel = new BusinessFormModel(this, businessPtr);
		}

		init();
	}

	void EntityView::init()
	{
		Wt::Dbo::Transaction t(APP->session());

		if(!_entityModel)
			_entityModel = new EntityFormModel(this);

		setCondition("type-chosen", true);
		switch(_type)
		{
		case Entity::InvalidType:
			setCondition("type-chosen", false);
			break;
		case Entity::PersonType:
			setCondition("is-person", true);
			if(!_personModel) _personModel = new PersonFormModel(this);
			break;
		case Entity::BusinessType:
			setCondition("is-business", true);
			if(!_businessModel) _businessModel = new BusinessFormModel(this);
			break;
		default:
			throw std::exception("NewEntityTemplate: Invalid EntityType");
			break;
		}
		setCondition("type-selection", !conditionValue("type-chosen"));

		_selectPerson = new Wt::WPushButton(tr("Person"));
		_selectPerson->clicked().connect(boost::bind(&EntityView::selectEntityType, this, Entity::PersonType));
		bindWidget("selectPerson", _selectPerson);

		_selectBusiness = new Wt::WPushButton(tr("Business"));
		_selectBusiness->clicked().connect(boost::bind(&EntityView::selectEntityType, this, Entity::BusinessType));
		bindWidget("selectBusiness", _selectBusiness);

		_addEmployee = new Wt::WPushButton(tr("AddEmployeeLabel"));
		_addEmployee->clicked().connect(boost::bind(&EntityView::setSpecificType, this, Entity::EmployeeType));
		if(_employeeModel) _addEmployee->hide();
		bindWidget("add-employee", _addEmployee);

		_addPersonnel = new Wt::WPushButton(tr("AddPersonnelLabel"));
		_addPersonnel->clicked().connect(boost::bind(&EntityView::setSpecificType, this, Entity::PersonnelType));
		if(_personnelModel) _addPersonnel->hide();
		bindWidget("add-personnel", _addPersonnel);

		_contactNumbersModel = new ContactNumbersManagerModel(this);
		_locationsModel = new LocationsManagerModel(this);
		
		if(entityPtr())
		{
			_expenseCycles = new EntityExpenseCycleList(entityPtr());
			bindWidget("expenseCycles", _expenseCycles);
			setCondition("show-expenseCycles", _expenseCycles->queryModel()->rowCount() != 0);

			_incomeCycles = new EntityIncomeCycleList(entityPtr());
			bindWidget("incomeCycles", _incomeCycles);
			setCondition("show-incomeCycles", _incomeCycles->queryModel()->rowCount() != 0);
		}
		else
		{
			bindEmpty("expenseCycles");
			setCondition("show-expenseCycles", false);
			bindEmpty("incomeCycles");
			setCondition("show-incomeCycles", false);
		}

		Wt::WPushButton *submit = new Wt::WPushButton(tr("Submit"));
		submit->clicked().connect(this, &EntityView::submit);
		bindWidget("submit", submit);

		updateView();
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
			if(!_personModel) _personModel = new PersonFormModel(this);
			_selectPerson->addStyleClass("btn-primary");
			_selectBusiness->removeStyleClass("btn-primary");
			break;
		case Entity::BusinessType:
			setCondition("is-business", true);
			setCondition("is-person", false);
			if(!_businessModel) _businessModel = new BusinessFormModel(this);
			_selectBusiness->addStyleClass("btn-primary");
			_selectPerson->removeStyleClass("btn-primary");
			break;
		}
		setCondition("type-chosen", true);
		_type = type;

		updateModel();
		updateView();
	}

	void EntityView::setSpecificType(Entity::SpecificType type)
	{
		if(type == Entity::UnspecificType)
			return;

		if(type == Entity::EmployeeType)
		{
			addEmployeeModel();
			_addEmployee->hide();
			updateModel(_employeeModel);
			updateView(_employeeModel);
		}
		if(type == Entity::PersonnelType)
		{
			addPersonnelModel();
			_addEmployee->hide();
			_addPersonnel->hide();
			updateModel(_employeeModel);
			updateView(_employeeModel);
			updateModel(_personnelModel);
			updateView(_personnelModel);
		}
	}

	void EntityView::addEmployeeModel(Wt::Dbo::ptr<Employee> employeePtr)
{
		if(_employeeModel || _type != Entity::PersonType)
			return;

		if(!_personModel)
			_personModel = new PersonFormModel(this);

		_employeeModel = new EmployeeFormModel(this, employeePtr);
		setCondition("is-employee", true);
	}

	void EntityView::addPersonnelModel(Wt::Dbo::ptr<Personnel> personnelPtr)
{
		if(_personnelModel || _type != Entity::PersonType)
			return;

		if(!_employeeModel)
			addEmployeeModel();

		_personnelModel = new PersonnelFormModel(this, personnelPtr);
		setCondition("is-personnel", true);
	}

	void EntityView::applyArguments(Wt::WWidget *w, const std::vector<Wt::WString> &args)
	{
		Wt::WTemplate::applyArguments(w, args);

		if(auto tw = dynamic_cast<Wt::WTemplate*>(w))
		{
			for(unsigned i = 0; i < args.size(); ++i)
			{
				std::string s = args[i].toUTF8();
				if(boost::starts_with(s, "label="))
				{
					if(auto lw = tw->resolve<Wt::WLabel*>("label"))
						lw->setText(Wt::WString::tr(s.substr(6)));
				}
			}
		}
	}

	Wt::WWidget *EntityView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(_entityModel)
		{
			if(auto result = _entityModel->createFormWidget(field))
				return result;
		}
		if(_personModel)
		{
			if(auto result = _personModel->createFormWidget(field))
				return result;
		}
		if(_employeeModel)
		{
			if(auto result = _employeeModel->createFormWidget(field))
				return result;
		}
		if(_personnelModel)
		{
			if(auto result = _personnelModel->createFormWidget(field))
				return result;
		}
		if(_contactNumbersModel)
		{
			if(auto result = _contactNumbersModel->createFormWidget(field))
				return result;
		}
		if(_locationsModel)
		{
			if(auto result = _locationsModel->createFormWidget(field))
				return result;
		}
		if(_businessModel)
		{
			if(auto result = _businessModel->createFormWidget(field))
				return result;
		}
		return MyTemplateFormView::createFormWidget(field);
	}

	bool EntityView::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit)
	{
		if(MyTemplateFormView::updateViewValue(model, field, edit))
			return true;

		if(auto contactNumbers = dynamic_cast<ContactNumbersContainer*>(edit))
		{
			const boost::any &v = model->value(field);

			if(v.empty())
				contactNumbers->reset();
			else
			{
				auto &numberVector = boost::any_cast<ContactNumbersManagerModel::NumberVector>(v);
				contactNumbers->updateViews(numberVector);
			}
			return true;
		}
		return false;
	}

	bool EntityView::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit)
	{
		if(MyTemplateFormView::updateModelValue(model, field, edit))
			return true;

		if(auto contactNumbers = dynamic_cast<ContactNumbersContainer*>(edit))
		{
			ContactNumbersManagerModel::NumberVector val;
			contactNumbers->updateModels(val);
			model->setValue(field, val);
			return true;
		}
		return false;
	}

	void EntityView::submit()
	{
		if(_type == Entity::InvalidType)
			return;

		updateModel();

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		try
		{
			bool valid = true;

			if(!_entityModel->validate()) valid = false;

			if(_type == Entity::PersonType)
			{
				if(!_personModel->validate()) valid = false;
				if(_employeeModel && !_employeeModel->validate()) valid = false;
				if(_personnelModel && !_personnelModel->validate()) valid = false;
			}
			else if(_type == Entity::BusinessType)
			{
				if(!_businessModel->validate()) valid = false;
			}

			if(_contactNumbersModel && !_contactNumbersModel->validate()) valid = false;
			if(_locationsModel && !_locationsModel->validate()) valid = false;

			if(!valid)
				return;

			if(!_entityModel->isAllReadOnly()) _entityModel->saveChanges();

			if(_type == Entity::PersonType)
			{
				if(!_personModel->isAllReadOnly()) _personModel->saveChanges();
				if(_employeeModel && !_employeeModel->isAllReadOnly()) _employeeModel->saveChanges();
				if(_personnelModel && !_personnelModel->isAllReadOnly()) _personnelModel->saveChanges();
			}
			else if(_type == Entity::BusinessType)
			{
				if(!_businessModel->isAllReadOnly()) _businessModel->saveChanges();
			}

			if(_contactNumbersModel && !_contactNumbersModel->isAllReadOnly()) _contactNumbersModel->saveChanges();
			if(_locationsModel && !_locationsModel->isAllReadOnly()) _locationsModel->saveChanges();

			t.commit();

			setCondition("type-selection", false);
			updateView();

			_submitted.emit();
		}
		catch(const Wt::Dbo::StaleObjectException &)
		{
			app->session().rereadAll();
			app->showStaleObjectError(tr("Entity"));
			//valid = false;
		}
		catch(const Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "EntityView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
			//valid = false;
		}
	}

	HeightEdit::HeightEdit(Wt::WContainerWidget *parent)
		: Wt::WTemplate(tr("GS.HeightEdit"), parent)
	{
		Wt::WPushButton *unitSelect = new Wt::WPushButton();
		Wt::WPopupMenu *unitMenu = new Wt::WPopupMenu();
		unitMenu->addItem("cm")->clicked().connect(boost::bind(&HeightEdit::selectUnit, this, cm));
		unitMenu->addItem("ft")->clicked().connect(boost::bind(&HeightEdit::selectUnit, this, ft));
		unitSelect->setMenu(unitMenu);

		Wt::WLineEdit *cmEdit = new Wt::WLineEdit();
		cmEdit->setValidator(new Wt::WIntValidator());

		Wt::WLineEdit *ftEdit = new Wt::WLineEdit();
		ftEdit->setValidator(new Wt::WIntValidator());

		Wt::WLineEdit *inEdit = new Wt::WLineEdit();
		inEdit->setValidator(new Wt::WIntValidator());

		bindWidget("unit-select", unitSelect);
		bindWidget("cm-edit", cmEdit);
		bindWidget("ft-edit", ftEdit);
		bindWidget("in-edit", inEdit);
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
		if(_unit == ft)
		{
			Wt::WLineEdit *ftEdit = resolve<Wt::WLineEdit*>("ft-edit");
			Wt::WLineEdit *inEdit = resolve<Wt::WLineEdit*>("in-edit");

			if(ftEdit->valueText().empty())
				return -1;

			float ftVal = boost::lexical_cast<float>(ftEdit->valueText());
			float inVal = 0;
			if(!inEdit->valueText().empty())
				inVal = boost::lexical_cast<float>(inEdit->valueText());

			return ftVal * 30.48f + inVal * 2.54f;
		}
		else
		{
			Wt::WLineEdit *cmEdit = resolve<Wt::WLineEdit*>("cm-edit");
			if(cmEdit->valueText().empty())
				return -1;

			return boost::lexical_cast<float>(cmEdit->valueText());
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

	ContactNumbersContainer::ContactNumbersContainer(ContactNumbersManagerModel *model, Wt::WContainerWidget *parent)
		: Wt::WContainerWidget(parent), _model(model)
	{
		for(int i = 0; i < _model->_ptrVector.size() + 1; ++i)
			addFieldWidget();
	}

	void ContactNumbersContainer::reset()
	{
		for(int i = 0; i < count(); ++i)
			dynamic_cast<Wt::WLineEdit*>(widget(i))->setText("");
	}

	void ContactNumbersContainer::updateViews(const ContactNumbersManagerModel::NumberVector &numberVector)
	{
		while(count() > numberVector.size())
		{
			auto w = widget(count() - 1);
			removeWidget(w);
			delete w;
		}
		while(count() < numberVector.size())
			addFieldWidget();

		for(int i = 0; i < numberVector.size(); ++i)
			dynamic_cast<Wt::WLineEdit*>(widget(i))->setText(numberVector[i]);
	}

	void ContactNumbersContainer::updateModels(ContactNumbersManagerModel::NumberVector &numberVector)
	{
		numberVector.resize(count());
		for(int i = 0; i < count(); ++i)
			numberVector[i] = dynamic_cast<Wt::WLineEdit*>(widget(i))->valueText();
	}

	void ContactNumbersContainer::addFieldWidget()
	{
		addWidget(new Wt::WLineEdit());
	}

// 	void initEntryCycleSummary(Wt::WTemplate *summaryTemplate, const EntryCycle &cycle, long long id)
// 	{
// 		WApplication *app = APP;
// 
// 		summaryTemplate->addFunction("tr", &Wt::WTemplate::Functions::tr);
// 		summaryTemplate->addFunction("block", &Wt::WTemplate::Functions::block);
// 		//summaryTemplate->setEncodeTemplateText(false);
// 		summaryTemplate->bindString("id", boost::lexical_cast<std::string>(id));
// 		summaryTemplate->bindString("startDate", cycle.startDate.toString(app->locale().dateFormat()));
// 		summaryTemplate->bindString("endDate", cycle.endDate.toString(app->locale().dateFormat()));
// 
// 		Wt::WString amountStr = Wt::WString::tr("Unknown");
// 		switch(cycle.interval)
// 		{
// 		case DailyInterval: amountStr = Wt::WString::trn("RsEveryNDays", cycle.nIntervals); break;
// 		case WeeklyInterval: amountStr = Wt::WString::trn("RsEveryNWeeks", cycle.nIntervals); break;
// 		case MonthlyInterval: amountStr = Wt::WString::trn("RsEveryNMonths", cycle.nIntervals); break;
// 		case YearlyInterval: amountStr = Wt::WString::trn("RsEveryNYears", cycle.nIntervals); break;
// 		}
// 		amountStr.arg(app->locale().toFixedString(cycle.amount, 2));
// 		amountStr.arg(cycle.nIntervals);
// 		summaryTemplate->bindString("amount", amountStr);
// 
// 		summaryTemplate->setCondition("end-date", cycle.endDate.isValid());
// 		if(cycle.endDate.isValid() && cycle.endDate <= Wt::WDate(boost::gregorian::day_clock::local_day()))
// 		{
// 			summaryTemplate->bindString("text-class", "text-muted");
// 			summaryTemplate->setCondition("has-ended", true);
// 		}
// 		else
// 		{
// 			summaryTemplate->bindEmpty("text-class");
// 			summaryTemplate->setCondition("has-ended", false);
// 		}
// 	}
// 
// 	ExpenseCycleSummaryContainer::ExpenseCycleSummaryContainer(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent /*= nullptr*/) : Wt::WContainerWidget(parent)
// 	{
// 		if(!entityPtr)
// 			return;
// 
// 		WApplication *app = APP;
// 		Wt::Dbo::Transaction t(app->session());
// 
// 		ExpenseCycleCollection collection = app->session().find<ExpenseCycle>().where("entity_id = ?").orderBy("startDate DESC").bind(entityPtr.id());
// 		_ptrVector = PtrVector(collection.begin(), collection.end());
// 		for(int i = 0; i < _ptrVector.size(); ++i)
// 		{
// 			auto ptr = _ptrVector[i];
// 			Wt::WTemplate *summaryTemplate = new Wt::WTemplate(tr("GS.Admin.ExpenseCycleSummary"), this);
// 			summaryTemplate->bindInt("index", i + 1);
// 			initEntryCycleSummary(summaryTemplate, *ptr, ptr.id());
// 
// 			if(ptr->positionPtr)
// 			{
// 				summaryTemplate->setCondition("has-extra", true);
// 				summaryTemplate->bindString("extra", ptr->positionPtr->title);
// 			}
// 			else
// 			{
// 				summaryTemplate->setCondition("has-extra", false);
// 				summaryTemplate->bindEmpty("extra");
// 			}
// 		}
// 	}
// 
// 	IncomeCycleSummaryContainer::IncomeCycleSummaryContainer(Wt::Dbo::ptr<Entity> entityPtr, Wt::WContainerWidget *parent /*= nullptr*/) : Wt::WContainerWidget(parent)
// 	{
// 		if(!entityPtr)
// 			return;
// 
// 		WApplication *app = APP;
// 		Wt::Dbo::Transaction t(app->session());
// 
// 		IncomeCycleCollection collection = app->session().find<IncomeCycle>().where("entity_id = ?").orderBy("startDate DESC").bind(entityPtr.id());
// 		_ptrVector = PtrVector(collection.begin(), collection.end());
// 		for(int i = 0; i < _ptrVector.size(); ++i)
// 		{
// 			auto ptr = _ptrVector[i];
// 			Wt::WTemplate *summaryTemplate = new Wt::WTemplate(tr("GS.Admin.IncomeCycleSummary"), this);
// 			summaryTemplate->bindInt("index", i + 1);
// 			initEntryCycleSummary(summaryTemplate, *ptr, ptr.id());
// 
// 			if(ptr->servicePtr)
// 			{
// 				summaryTemplate->setCondition("has-extra", true);
// 				summaryTemplate->bindString("extra", ptr->servicePtr->title);
// 			}
// 			else
// 			{
// 				summaryTemplate->setCondition("has-extra", false);
// 				summaryTemplate->bindEmpty("extra");
// 			}
// 		}
// 	}

// 	//EXPENSE CYCLES MANAGER MODEL
// 	const Wt::WFormModel::Field ExpenseCyclesManagerModel::field = "expenseCycles";
// 
// 	ExpenseCyclesManagerModel::ExpenseCyclesManagerModel(EntityView *view)
// 		: Wt::WFormModel(view), _view(view)
// 	{
// 		addField(field);
// 
// 		if(_view->entityPtr())
// 			setValue(field, PtrVector(_view->entityPtr()->expenseCycleCollection.begin(), _view->entityPtr()->expenseCycleCollection.end()));
// 		else
// 			setValue(field, PtrVector());
// 	}
// 
// 	bool ExpenseCyclesManagerModel::validate()
// 	{
// 		auto container = _view->resolve<ExpenseCyclesContainer*>(field);
// 		if(!container)
// 			return false;
// 
// 		bool result = container->validateAll();
// 		setValidation(field, Wt::WValidator::Result(result ? Wt::WValidator::Valid : Wt::WValidator::Invalid));
// 		return result;
// 	}
// 
// 	Wt::WWidget *ExpenseCyclesManagerModel::createFormWidget(Field f)
// 	{
// 		if(f == field)
// 		{
// 			auto w = new ExpenseCyclesContainer(this);
// 			auto btn = new Wt::WPushButton();
// 			btn->setText(Wt::WString::tr("AddRecurringExpense"));
// 			btn->clicked().connect(boost::bind(&ExpenseCyclesManagerModel::createAddCycleDialog, this));
// 			_view->bindWidget("add-expense-cycle", btn);
// 			return w;
// 		}
// 		return nullptr;
// 	}
// 
// 	void ExpenseCyclesManagerModel::saveChanges()
// 	{
// 		if(!valid())
// 			return;
// 
// 		auto container = _view->resolve<ExpenseCyclesContainer*>(field);
// 		auto vector = boost::any_cast<PtrVector>(value(field));
// 		if(vector.size() < container->count())
// 			vector.resize(container->count());
// 
// 		WApplication *app = WApplication::instance();
// 		Wt::Dbo::Transaction t(app->session());
// 		for(int i = 0; i < container->count(); ++i)
// 		{
// 			auto model = container->viewModel(i);
// 			model->saveChanges(vector[i], _view->entityPtr());
// 		}
// 		t.commit();
// 		setValue(field, vector);
// 	}
// 
// 	Wt::WDialog *ExpenseCyclesManagerModel::createAddCycleDialog()
// 	{
// 		Wt::WDialog *dialog = new Wt::WDialog(Wt::WString::tr("AddNewRecurringExpense"), this);
// 		dialog->setClosable(true);
// 		dialog->setWidth(Wt::WLength(700));
// 
// 		ExpenseCycleView *cycleView = new ExpenseCycleView(Wt::Dbo::ptr<ExpenseCycle>(), dialog->contents());
// 		cycleView->bindInt("index", _view->resolve<ExpenseCyclesContainer*>(field)->count() + 1);
// 		auto hr = new Wt::WBreak(dialog->contents());
// 		hr->setHtmlTagName("hr");
// 		Wt::WPushButton *submitBtn = new Wt::WPushButton(Wt::WString::tr("Continue"), dialog->contents());
// 		submitBtn->addStyleClass("btn-primary");
// 		submitBtn->clicked().connect(std::bind([=]() {
// 			cycleView->updateModel(cycleView->model());
// 			bool valid = cycleView->model()->validate();
// 			cycleView->updateView(cycleView->model());
// 
// 			if(valid)
// 				dialog->accept();
// 		}));
// 
// 		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
// 			if(code == Wt::WDialog::Accepted)
// 			{
// 				dialog->contents()->removeWidget(cycleView);
// 				_view->resolve<ExpenseCyclesContainer*>(field)->addWidget(cycleView);
// 			}
// 			delete dialog;
// 		}, std::placeholders::_1));
// 
// 		dialog->show();
// 		return dialog;
// 	}
// 
// 	//INCOME CYCLES MANAGER MODEL
// 	const Wt::WFormModel::Field IncomeCyclesManagerModel::field = "incomeCycles";
// 
// 	IncomeCyclesManagerModel::IncomeCyclesManagerModel(EntityView *view)
// 		: Wt::WFormModel(view), _view(view)
// 	{
// 		addField(field);
// 
// 		if(_view->entityPtr())
// 			setValue(field, PtrVector(_view->entityPtr()->incomeCycleCollection.begin(), _view->entityPtr()->incomeCycleCollection.end()));
// 		else
// 			setValue(field, PtrVector());
// 	}
// 
// 	bool IncomeCyclesManagerModel::validate()
// 	{
// 		auto container = _view->resolve<IncomeCyclesContainer*>(field);
// 		if(!container)
// 			return false;
// 
// 		bool result = container->validateAll();
// 		setValidation(field, Wt::WValidator::Result(result ? Wt::WValidator::Valid : Wt::WValidator::Invalid));
// 		return result;
// 	}
// 
// 	Wt::WWidget *IncomeCyclesManagerModel::createFormWidget(Field f)
// 	{
// 		if(f == field)
// 		{
// 			auto w = new IncomeCyclesContainer(this);
// 			auto btn = new Wt::WPushButton();
// 			btn->setText(Wt::WString::tr("AddRecurringIncome"));
// 			btn->clicked().connect(boost::bind(&IncomeCyclesManagerModel::createAddCycleDialog, this));
// 			_view->bindWidget("add-income-cycle", btn);
// 			return w;
// 		}
// 		return nullptr;
// 	}
// 
// 	void IncomeCyclesManagerModel::saveChanges()
// 	{
// 		if(!valid())
// 			return;
// 
// 		auto container = _view->resolve<IncomeCyclesContainer*>(field);
// 		auto vector = boost::any_cast<PtrVector>(value(field));
// 		if(vector.size() < container->count())
// 			vector.resize(container->count());
// 
// 		WApplication *app = WApplication::instance();
// 		Wt::Dbo::Transaction t(app->session());
// 		for(int i = 0; i < container->count(); ++i)
// 		{
// 			auto model = container->viewModel(i);
// 			model->saveChanges(vector[i], _view->entityPtr());
// 		}
// 		t.commit();
// 		setValue(field, vector);
// 	}
// 
// 	Wt::WDialog *IncomeCyclesManagerModel::createAddCycleDialog()
// 	{
// 		Wt::WDialog *dialog = new Wt::WDialog(Wt::WString::tr("AddNewRecurringIncome"), this);
// 		dialog->setClosable(true);
// 		dialog->setWidth(Wt::WLength(700));
// 
// 		IncomeCycleView *cycleView = new IncomeCycleView(Wt::Dbo::ptr<IncomeCycle>(), dialog->contents());
// 		cycleView->bindInt("index", _view->resolve<IncomeCyclesContainer*>(field)->count() + 1);
// 		auto hr = new Wt::WBreak(dialog->contents());
// 		hr->setHtmlTagName("hr");
// 		Wt::WPushButton *submitBtn = new Wt::WPushButton(Wt::WString::tr("Continue"), dialog->contents());
// 		submitBtn->addStyleClass("btn-primary");
// 		submitBtn->clicked().connect(std::bind([=]() {
// 			cycleView->updateModel(cycleView->model());
// 			bool valid = cycleView->model()->validate();
// 			cycleView->updateView(cycleView->model());
// 
// 			if(valid)
// 				dialog->accept();
// 		}));
// 
// 		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
// 			if(code == Wt::WDialog::Accepted)
// 			{
// 				dialog->contents()->removeWidget(cycleView);
// 				_view->resolve<IncomeCyclesContainer*>(field)->addWidget(cycleView);
// 			}
// 			delete dialog;
// 		}, std::placeholders::_1));
// 
// 		dialog->show();
// 		return dialog;
// 	}


}
