#include "ModelView/UserMVC.h"
#include "Application/WServer.h"
#include "Dbo/PermissionsDatabase.h"
#include <Wt/WLengthValidator.h>
#include <Wt/WDateEdit.h>
#include <Wt/WTimeEdit.h>
#include <Wt/WTableView.h>
#include <Wt/Auth/PasswordStrengthValidator.h>

namespace ERP
{

	void UserList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "ainfo.id", std::bind(&FiltersTemplate::initIdEdit, _1))); filtersTemplate()->addFilter(1);
	}

	void UserList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT ainfo.id, aid.identity, ainfo.email, r.name FROM "
			"auth_info ainfo "
			"INNER JOIN auth_identity aid ON (aid.auth_info_id = ainfo.id AND aid.provider = ?) "
			"LEFT JOIN " + User::tStr() + " u ON (u.id = ainfo.user_id) "
			"LEFT JOIN " + Region::tStr() + " r ON (r.id = u.region_id)"
		).bind(Wt::Auth::Identity::LoginName);
		app->authLogin().setPermissionConditionsToQuery(_baseQuery, false, "u.");

		model->setQuery(generateFilteredQuery());
		addColumn(ViewId, model->addColumn("ainfo.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewLoginName, model->addColumn("aid.identity"), tr("UserName"), 150);
		addColumn(ViewEmail, model->addColumn("ainfo.email"), tr("Email"), EmailColumnWidth);
		addColumn(ViewRegionName, model->addColumn("r.name"), tr("Region"), 150);

		_proxyModel = make_shared<UserListProxyModel>(_model);
	}

	UserListProxyModel::UserListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void UserListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::WFlags<Wt::ItemFlag> UserListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::any UserListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any UserListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const UserList::ResultType &res = static_pointer_cast<Dbo::QueryModel<UserList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<UserList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, User::viewInternalPath(id));
			}
		}
		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	const Wt::WFormModel::Field UserFormModel::loginNameField = "loginName";
	const Wt::WFormModel::Field UserFormModel::passwordField = "password";
	const Wt::WFormModel::Field UserFormModel::password2Field = "password2";
	const Wt::WFormModel::Field UserFormModel::emailField = "email";
	const Wt::WFormModel::Field UserFormModel::regionField = "region";
	const Wt::WFormModel::Field UserFormModel::permissionsField = "permissions";

	UserFormModel::UserFormModel(UserView *view, Dbo::ptr<AuthInfo> authInfoPtr)
		: RecordFormModel(view, Dbo::ptr<User>()), _view(view), _authInfoPtr(move(authInfoPtr))
	{
		addField(loginNameField);
		addField(passwordField);
		addField(password2Field);
		addField(emailField);
		addField(regionField);
		addField(permissionsField);
	}

	void UserFormModel::rereadDbos()
	{
		_recordPtr.reread();
		_authInfoPtr.reread();
	}

	void UserFormModel::updateFromDb()
	{
		WApplication *app = APP;
		TRANSACTION(app);

		Dbo::ptr<User> userPtr = _authInfoPtr->user();
		if(!userPtr)
		{
			userPtr = app->dboSession().addNew<User>();
			_authInfoPtr.modify()->setUser(userPtr);
		}
		_recordPtr = userPtr;

		setValue(loginNameField, _authInfoPtr->identity(Wt::Auth::Identity::LoginName));
		setValue(emailField, _authInfoPtr->email());
		setValue(regionField, _recordPtr->regionPtr());

		_permissionMap = SERVER->permissionsDatabase().getUserPermissions(_recordPtr, Wt::Auth::LoginState::Strong, &app->dboSession());
		if(_permissionMap.find(Permissions::GlobalAdministrator) != _permissionMap.end())
			setValue(permissionsField, (int)GlobalAdministrator);
		else if(_permissionMap.find(Permissions::RegionalAdministrator) != _permissionMap.end())
			setValue(permissionsField, (int)RegionalAdministrator);
		else
			setValue(permissionsField, (int)RegionalUser);

		setVisible(passwordField, false);
		setVisible(password2Field, false);

		if(_authInfoPtr.id() == app->authLogin().authInfoPtr().id())
			setReadOnly(permissionsField, true);
	}

	unique_ptr<Wt::WWidget> UserFormModel::createFormWidget(Field field)
	{
		if(field == loginNameField)
		{
			auto edit = make_unique<Wt::WLineEdit>();
			auto validator = make_shared<LoginNameValidator>();
			if(isRecordPersisted())
			{
				TRANSACTION(APP);
				validator->setAllowedName(_authInfoPtr->identity(Wt::Auth::Identity::LoginName));
			}
			setValidator(field, validator);
			edit->changed().connect(this, std::bind(&AbstractRecordFormModel::validateUpdateField, this, loginNameField));
			return edit;
		}
		if(field == passwordField)
		{
			auto edit = make_unique<Wt::WLineEdit>();
			edit->setEchoMode(Wt::EchoMode::Password);
			setValidator(field, shared_ptr<Wt::Auth::AbstractPasswordService::AbstractStrengthValidator>(SERVER->getPasswordService().strengthValidator()));
			return edit;
		}
		if(field == password2Field)
		{
			auto edit = make_unique<Wt::WLineEdit>();
			edit->setEchoMode(Wt::EchoMode::Password);
			return edit;
		}
		if(field == emailField)
		{
			auto edit = make_unique<Wt::WLineEdit>();
			auto validator = make_shared<Wt::WRegExpValidator>(".+\\@.+\\..+");
			validator->setMandatory(true);
			validator->setInvalidNoMatchText(tr("InvalidEmailAddress"));
			setValidator(field, validator);
			return edit;
		}
		if(field == regionField)
		{
			WApplication *app = APP;
			app->initRegionQueryModel();
			auto edit = make_unique<QueryProxyModelCB<RegionProxyModel>>(app->regionProxyModel());
			return edit;
		}
		if(field == permissionsField)
		{
			WApplication *app = APP;

			auto edit = make_unique<Wt::WComboBox>();
			edit->insertItem(RegionalUser, tr("RegionalUser"));

			if(app->authLogin().hasPermission(Permissions::GlobalAdministrator))
			{
				edit->insertItem(RegionalAdministrator, tr("RegionalAdministrator"));
				edit->insertItem(GlobalAdministrator, tr("GlobalAdministrator"));
			}

			if(!app->authLogin().hasPermission(Permissions::ModifyUserPermission))
				setReadOnly(field, true);

			return edit;
		}
		return nullptr;
	}

	bool UserFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_authInfoPtr)
		{
			Wt::Auth::User authUser = app->userDatabase().registerNew();
			authUser.setIdentity(Wt::Auth::Identity::LoginName, valueText(loginNameField));
			_authInfoPtr = app->userDatabase().find(authUser);

			_recordPtr = app->dboSession().addNew<User>();
			_authInfoPtr.modify()->setUser(_recordPtr);
			_recordPtr.modify()->setCreatedByValues(false);
		}

		if(!_recordPtr)
		{
			if(_authInfoPtr->user())
				_recordPtr = _authInfoPtr->user();
			else
			{
				_recordPtr = app->dboSession().addNew<User>();
				_authInfoPtr.modify()->setUser(_recordPtr);
			}
		}

		_recordPtr.modify()->_regionPtr = Wt::any_cast<Dbo::ptr<Region>>(value(regionField));
		_authInfoPtr.modify()->setEmail(valueText(emailField).toUTF8());

		if(isVisible(passwordField) && isVisible(password2Field))
		{
			const Wt::WString &newPassword = valueText(passwordField);
			SERVER->getPasswordService().updatePassword(app->userDatabase().find(_authInfoPtr), newPassword);
		}

		if(app->authLogin().hasPermission(Permissions::GlobalAdministrator) && app->authLogin().hasPermission(Permissions::ModifyUserPermission) && !isReadOnly(permissionsField))
		{
			auto permissionIndex = Wt::any_cast<int>(value(permissionsField));

			_recordPtr.modify()->userPermissionCollection.clear();
			app->dboSession().addNew<UserPermission>(_recordPtr, app->dboSession().loadLazy<Permission>(permissionIndexToId(permissionIndex)));
			_permissionMap = SERVER->permissionsDatabase().getUserPermissions(_recordPtr, Wt::Auth::LoginState::Strong, &app->dboSession());
		}

		t.commit();

		app->dboSession().flush();
		auto nameValidator = static_pointer_cast<LoginNameValidator>(validator(loginNameField));
		nameValidator->setAllowedName(valueText(loginNameField));

		if(isVisible(passwordField) && isVisible(password2Field))
		{
			setVisible(password2Field, false);
			setReadOnly(passwordField, true);
		}
		return true;
	}

	bool UserFormModel::validateField(Field field)
	{
		//if changing password
		if(!valueText(passwordField).empty())
		{
			if(field == passwordField)
			{
				auto v = SERVER->getPasswordService().strengthValidator();
				Wt::WValidator::Result r = v->validate(valueText(passwordField), valueText(loginNameField), valueText(emailField).toUTF8());

				if(r.state() == Wt::ValidationState::Valid)
					setValidation(field, Wt::WValidator::Result(Wt::ValidationState::Valid, r.message().empty() ? tr("Wt.Auth.valid") : r.message()));
				else
					setValidation(field, Wt::WValidator::Result(Wt::ValidationState::Invalid, r.message()));

				return validation(field).state() == Wt::ValidationState::Valid;
			}
			if(field == password2Field)
			{
				if(validation(passwordField).state() == Wt::ValidationState::Valid)
				{
					if(valueText(passwordField) == valueText(password2Field))
						setValidation(field, Wt::WValidator::Result(Wt::ValidationState::Valid));
					else
						setValidation(field, Wt::WValidator::Result(Wt::ValidationState::Invalid, tr("Wt.Auth.passwords-dont-match")));

					return validation(field).state() == Wt::ValidationState::Valid;
				}
				else
					return true; // Do not validate the repeat field yet
			}
		}
		return RecordFormModel::validateField(field);
	}

	AuthLogin::PermissionResult UserFormModel::checkViewPermission() const
	{
		auto res = RecordFormModel::checkViewPermission();
		if(res != AuthLogin::Permitted)
			return res;

		return APP->authLogin().checkPermission(Permissions::ViewUser);
	}

	AuthLogin::PermissionResult UserFormModel::checkModifyPermission() const
	{
		auto res = RecordFormModel::checkModifyPermission();
		if(res != AuthLogin::Permitted)
			return res;

		WApplication *app = APP;

		auto modifyUserPermission = app->authLogin().checkPermission(Permissions::ModifyUser);
		if(modifyUserPermission != AuthLogin::Permitted)
			return modifyUserPermission;

		if(!app->authLogin().hasPermission(Permissions::GlobalAdministrator) && _authInfoPtr.id() != app->authLogin().authInfoPtr().id())
		{
			if(_permissionMap.find(Permissions::GlobalAdministrator) != _permissionMap.end() || _permissionMap.find(Permissions::RegionalAdministrator) != _permissionMap.end())
				return AuthLogin::Denied;
		}
		return res;
	}

	AuthLogin::PermissionResult UserFormModel::checkCreatePermission() const
	{
		auto res = RecordFormModel::checkCreatePermission();
		if(res != AuthLogin::Permitted)
			return res;

		return APP->authLogin().checkPermission(Permissions::CreateUser);
	}

	void UserFormModel::persistedHandler()
	{
		setReadOnly(loginNameField, true);

		auto changePassword = _view->bindNew<Wt::WPushButton>("changePassword", tr("ChangePassword"));
		changePassword->clicked().connect(this, &UserFormModel::handleChangePassword);
	}

	void UserFormModel::handleChangePassword()
	{
		if(!isRecordPersisted())
			return;

		_view->updateModel();

		setReadOnly(passwordField, false);
		setValue(passwordField, Wt::any());
		setValue(password2Field, Wt::any());

		setVisible(passwordField, true);
		setVisible(password2Field, true);

		_view->updateView();
	}

	UserView::UserView(Dbo::ptr<AuthInfo> authInfoPtr)
		: RecordFormView(tr("ERP.Admin.UserView"))
	{
		_model = newFormModel<UserFormModel>("user", this, move(authInfoPtr));
	}

	Wt::WString UserView::viewName() const
	{
		if(authInfoPtr())
		{
			TRANSACTION(APP);
			return tr("UserViewName").arg(authInfoPtr()->identity(Wt::Auth::Identity::LoginName));
		}
		return "UserView";
	}

	void RegionList::initFilters()
	{
		filtersTemplate()->addFilterModel(make_shared<WLineEditFilterModel>(tr("ID"), "r.id", std::bind(&FiltersTemplate::initIdEdit, _1))); filtersTemplate()->addFilter(1);
	}

	void RegionList::initModel()
	{
		shared_ptr<QueryModelType> model;
		_model = model = make_shared<QueryModelType>();

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT r.id, r.name FROM " + Region::tStr() + " r ");

		model->setQuery(generateFilteredQuery());
		addColumn(ViewId, model->addColumn("r.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("r.name"), tr("Name"), 300);

		_proxyModel = make_shared<RegionListProxyModel>(_model);
	}

	RegionListProxyModel::RegionListProxyModel(shared_ptr<Wt::WAbstractItemModel> model)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void RegionListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::WFlags<Wt::ItemFlag> RegionListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemFlag::XHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	Wt::any RegionListProxyModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Width)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	Wt::any RegionListProxyModel::data(const Wt::WModelIndex &idx, Wt::ItemDataRole role) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::ItemDataRole::Display)
				return tr("ERP.LinkIcon");
			else if(role == Wt::ItemDataRole::Link)
			{
				const RegionList::ResultType &res = static_pointer_cast<Dbo::QueryModel<RegionList::ResultType>>(sourceModel())->resultRow(idx.row());
				long long id = std::get<RegionList::ResId>(res);
				return Wt::WLink(Wt::LinkType::InternalPath, Region::viewInternalPath(id));
			}
		}
		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	const Wt::WFormModel::Field RegionFormModel::nameField = "name";

	RegionFormModel::RegionFormModel(RegionView *view, Dbo::ptr<Region> regionPtr)
		: RecordFormModel(view, move(regionPtr)), _view(view)
	{
		addField(nameField);
	}

	void RegionFormModel::updateFromDb()
	{
		TRANSACTION(APP);
		setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));
	}

	unique_ptr<Wt::WWidget> RegionFormModel::createFormWidget(Field field)
	{
		if(field == nameField)
		{
			auto hostName = make_unique<Wt::WLineEdit>();
			hostName->setMaxLength(70);
			auto validator = make_shared<Wt::WLengthValidator>(0, 70);
			validator->setMandatory(true);
			setValidator(nameField, validator);
			return hostName;
		}
		return nullptr;
	}

	bool RegionFormModel::saveChanges()
	{
		if(!valid())
			return false;

		WApplication *app = APP;
		TRANSACTION(app);

		if(!_recordPtr)
			_recordPtr = app->dboSession().addNew<Region>();

		_recordPtr.modify()->name = valueText(nameField).toUTF8();

		if(app->regionQueryModel())
			app->regionQueryModel()->reload();

		return true;
	}

	AuthLogin::PermissionResult RegionFormModel::checkViewPermission() const
	{
		auto res = RecordFormModel::checkViewPermission();
		if(res != AuthLogin::Permitted)
			return res;

		return APP->authLogin().checkPermission(Permissions::ViewRegion);
	}

	AuthLogin::PermissionResult RegionFormModel::checkModifyPermission() const
	{
		auto res = RecordFormModel::checkModifyPermission();
		if(res != AuthLogin::Permitted)
			return res;

		return APP->authLogin().checkPermission(Permissions::ModifyRegion);
	}

	AuthLogin::PermissionResult RegionFormModel::checkCreatePermission() const
	{
		auto res = RecordFormModel::checkCreatePermission();
		if(res != AuthLogin::Permitted)
			return res;

		return APP->authLogin().checkPermission(Permissions::CreateRegion);
	}

	RegionView::RegionView(Dbo::ptr<Region> regionPtr)
		: RecordFormView(tr("ERP.Admin.RegionView"))
	{
		_model = newFormModel<RegionFormModel>("region", this, move(regionPtr));
	}

	Wt::WString RegionView::viewName() const
	{
		if(regionPtr())
		{
			TRANSACTION(APP);
			return tr("RegionViewName").arg(regionPtr()->name);
		}
		return "RegionView";
	}

	RegionProxyModel::RegionProxyModel(shared_ptr<QueryModel> sourceModel)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &RegionProxyModel::addAdditionalRows);
	}

	void RegionProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
			setData(index(0, 0), tr("None"));
	}

	Wt::WValidator::Result LoginNameValidator::validate(const Wt::WString &input) const
	{
		Wt::WValidator::Result res = Wt::WValidator::validate(input);
		if(res.state() != Wt::ValidationState::Valid)
			return res;

		if(!_allowedName.empty() && input == _allowedName)
			return res;

		bool valid = true;
		Wt::WString error;
		if(input.toUTF8().length() < _minLoginNameLength)
			error = tr("Wt.Auth.user-name-tooshort").arg(_minLoginNameLength);

		if(!error.empty())
			valid = false;
		else
		{
			Wt::Auth::User existingUser = APP->userDatabase().findWithIdentity(Wt::Auth::Identity::LoginName, input);
			valid = !existingUser.isValid();

			if(existingUser.isValid())
				error = tr("Wt.Auth.user-name-exists");
		}

		if(!valid)
			return Wt::WValidator::Result(Wt::ValidationState::Invalid, error);
		return res;
	}
}