#include "Widgets/UserMVC.h"
#include "Application/WServer.h"
#include "Dbo/PermissionsDatabase.h"
#include <Wt/WLengthValidator>
#include <Wt/WDateEdit>
#include <Wt/WTimeEdit>
#include <Wt/WTableView>
#include <Wt/Auth/Identity>
#include <Wt/Auth/PasswordStrengthValidator>

namespace GS
{

	void UserList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "ainfo.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1))); filtersTemplate()->addFilter(1);
	}

	void UserList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT ainfo.id ainfo_id, aid.identity, ainfo.email, r.name FROM "
			"auth_info ainfo "
			"INNER JOIN auth_identity aid ON (aid.auth_info_id = ainfo.id AND aid.provider = ?) "
			"LEFT JOIN " + std::string(User::tableName()) + " u ON (u.id = ainfo.user_id) "
			"LEFT JOIN " + std::string(Region::tableName()) + " r ON (r.id = u.region_id)"
			).bind(Wt::Auth::Identity::LoginName);

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("ainfo.id ainfo_id"), tr("ID"), IdColumnWidth);
		addColumn(ViewLoginName, model->addColumn("aid.identity"), tr("UserName"), 150);
		addColumn(ViewEmail, model->addColumn("ainfo.email"), tr("Email"), EmailColumnWidth);
		addColumn(ViewRegionName, model->addColumn("r.name"), tr("Region"), 150);

		_proxyModel = new UserListProxyModel(_model, _model);
	}

	UserListProxyModel::UserListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
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
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	boost::any UserListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	boost::any UserListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const UserList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<UserList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<UserList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, User::viewInternalPath(id));
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

	UserFormModel::UserFormModel(UserView *view, Wt::Dbo::ptr<AuthInfo> authInfoPtr /*= Wt::Dbo::ptr<AuthInfo>()*/)
		: RecordFormModel(view, Wt::Dbo::ptr<User>()), _view(view), _authInfoPtr(authInfoPtr)
	{
		addField(loginNameField);
		addField(passwordField);
		addField(password2Field);
		addField(emailField);
		addField(regionField);
		addField(permissionsField);

		if(_authInfoPtr)
		{
			WApplication *app = APP;
			TRANSACTION(app);

			Wt::Dbo::ptr<User> userPtr = _authInfoPtr->user();
			if(!userPtr)
			{
				userPtr = app->dboSession().add(new User());
				_authInfoPtr.modify()->setUser(userPtr);
			}
			_recordPtr = userPtr;

			setValue(loginNameField, _authInfoPtr->identity(Wt::Auth::Identity::LoginName));
			setValue(emailField, _authInfoPtr->email());
			setValue(regionField, _recordPtr->regionPtr);

			auto permissions = SERVER->permissionsDatabase()->getUserPermissions(_recordPtr, Wt::Auth::StrongLogin, &app->dboSession());
			if(permissions.find(Permissions::GlobalAdministrator) != permissions.end())
				setValue(permissionsField, (int)GlobalAdministrator);
			else if(permissions.find(Permissions::RegionalAdministrator) != permissions.end())
				setValue(permissionsField, (int)RegionalAdministrator);
			else
				setValue(permissionsField, (int)RegionalUser);

			setVisible(passwordField, false);
			setVisible(password2Field, false);

			if(_authInfoPtr.id() == app->authLogin().authInfoPtr().id())
				setReadOnly(permissionsField, true);
		}
	}

	Wt::WWidget *UserFormModel::createFormWidget(Field field)
	{
		if(field == loginNameField)
		{
			Wt::WLineEdit *edit = new Wt::WLineEdit();
			LoginNameValidator *validator = new LoginNameValidator(edit);
			if(isRecordPersisted())
			{
				TRANSACTION(APP);
				validator->setAllowedName(_authInfoPtr->identity(Wt::Auth::Identity::LoginName));
			}
			setValidator(field, validator);
			edit->changed().connect(boost::bind(&AbstractRecordFormModel::validateUpdateField, this, loginNameField));
			return edit;
		}
		if(field == passwordField)
		{
			Wt::WLineEdit *edit = new Wt::WLineEdit();
			edit->setEchoMode(Wt::WLineEdit::Password);
			setValidator(field, SERVER->getPasswordService().strengthValidator());
			return edit;
		}
		if(field == password2Field)
		{
			Wt::WLineEdit *edit = new Wt::WLineEdit();
			edit->setEchoMode(Wt::WLineEdit::Password);
			return edit;
		}
		if(field == emailField)
		{
			Wt::WLineEdit *edit = new Wt::WLineEdit();
			Wt::WRegExpValidator *validator = new Wt::WRegExpValidator(".+\\@.+\\..+", edit);
			validator->setMandatory(true);
			validator->setInvalidNoMatchText(Wt::WString::tr("InvalidEmailAddress"));
			setValidator(field, validator);
			return edit;
		}
		if(field == regionField)
		{
			WApplication *app = APP;
			app->initRegionQueryModel();
			auto edit = new QueryProxyModelCB<RegionProxyModel>(app->regionProxyModel());
			return edit;
		}
		if(field == permissionsField)
		{
			WApplication *app = APP;

			Wt::WComboBox *edit = new Wt::WComboBox();
			edit->insertItem(RegionalUser, Wt::WString::tr("RegionalUser"));

			if(app->authLogin().hasPermission(Permissions::GlobalAdministrator))
			{
				edit->insertItem(RegionalAdministrator, Wt::WString::tr("RegionalAdministrator"));
				edit->insertItem(GlobalAdministrator, Wt::WString::tr("GlobalAdministrator"));
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

			_recordPtr = app->dboSession().add(new User());
			_authInfoPtr.modify()->setUser(_recordPtr);
			_recordPtr.modify()->setCreatedByValues(false);
		}

		if(!_recordPtr)
		{
			if(_authInfoPtr->user())
				_recordPtr = _authInfoPtr->user();
			else
			{
				_recordPtr = app->dboSession().add(new User());
				_authInfoPtr.modify()->setUser(_recordPtr);
			}
		}

		_recordPtr.modify()->regionPtr = boost::any_cast<Wt::Dbo::ptr<Region>>(value(regionField));
		_authInfoPtr.modify()->setEmail(valueText(emailField).toUTF8());

		if(isVisible(passwordField) && isVisible(password2Field))
		{
			const Wt::WString &newPassword = valueText(passwordField);
			SERVER->getPasswordService().updatePassword(app->userDatabase().find(_authInfoPtr), newPassword);
		}

		if(app->authLogin().hasPermission(Permissions::GlobalAdministrator) && app->authLogin().hasPermission(Permissions::ModifyUserPermission) && !isReadOnly(permissionsField))
		{
			int permissionIndex = boost::any_cast<int>(value(permissionsField));

			_recordPtr.modify()->userPermissionCollection.clear();
			app->dboSession().add(new UserPermission(_recordPtr, app->dboSession().loadLazy<Permission>(permissionIndexToId(permissionIndex))));
		}

		t.commit();

		app->dboSession().flush();
		auto nameValidator = dynamic_cast<LoginNameValidator*>(validator(loginNameField));
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

				if(r.state() == Wt::WValidator::Valid)
					setValidation(field, Wt::WValidator::Result(Wt::WValidator::Valid, r.message().empty() ? Wt::WString::tr("Wt.Auth.valid") : r.message()));
				else
					setValidation(field, Wt::WValidator::Result(Wt::WValidator::Invalid, r.message()));

				return validation(field).state() == Wt::WValidator::Valid;
			}
			if(field == password2Field)
			{
				if(validation(passwordField).state() == Wt::WValidator::Valid)
				{
					if(valueText(passwordField) == valueText(password2Field))
						setValidation(field, Wt::WValidator::Result(Wt::WValidator::Valid));
					else
						setValidation(field, Wt::WValidator::Result(Wt::WValidator::Invalid, Wt::WString::tr("Wt.Auth.passwords-dont-match")));

					return validation(field).state() == Wt::WValidator::Valid;
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

		return APP->authLogin().checkPermission(Permissions::ModifyUser);
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

		Wt::WPushButton *changePassword = new Wt::WPushButton(Wt::WString::tr("ChangePassword"));
		changePassword->clicked().connect(this, &UserFormModel::handleChangePassword);
		_view->bindWidget("changePassword", changePassword);
	}

	void UserFormModel::handleChangePassword()
	{
		if(!isRecordPersisted())
			return;

		_view->updateModel();

		setReadOnly(passwordField, false);
		setValue(passwordField, boost::any());
		setValue(password2Field, boost::any());

		setVisible(passwordField, true);
		setVisible(password2Field, true);

		_view->updateView();
	}

	UserView::UserView(Wt::Dbo::ptr<AuthInfo> authInfoPtr /*= Wt::Dbo::ptr<AuthInfo>()*/)
		: RecordFormView(tr("GS.Admin.UserView")), _tempPtr(authInfoPtr)
	{ }

	void UserView::initView()
	{
		_model = new UserFormModel(this, _tempPtr);
		addFormModel("user", _model);
	}

	Wt::WString UserView::viewName() const
	{
		TRANSACTION(APP);
		return tr("UserViewName").arg(authInfoPtr()->identity(Wt::Auth::Identity::LoginName));
	}

	void RegionList::initFilters()
	{
		filtersTemplate()->addFilterModel(new WLineEditFilterModel(tr("ID"), "r.id", std::bind(&FiltersTemplate::initIdEdit, std::placeholders::_1))); filtersTemplate()->addFilter(1);
	}

	void RegionList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		WApplication *app = APP;
		_baseQuery = app->dboSession().query<ResultType>(
			"SELECT r.id, r.name FROM "
			+ std::string(Region::tableName()) + " r ");

		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);

		addColumn(ViewId, model->addColumn("r.id"), tr("ID"), IdColumnWidth);
		addColumn(ViewName, model->addColumn("r.name"), tr("Name"), 300);

		_proxyModel = new RegionListProxyModel(_model, _model);
	}

	RegionListProxyModel::RegionListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
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
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	boost::any RegionListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
		{
			if(role == Wt::WidthRole)
				return 40;
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		}

		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	boost::any RegionListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				const RegionList::ResultType &res = dynamic_cast<Wt::Dbo::QueryModel<RegionList::ResultType>*>(sourceModel())->resultRow(idx.row());
				long long id = boost::get<RegionList::ResId>(res);
				return Wt::WLink(Wt::WLink::InternalPath, Region::viewInternalPath(id));
			}
		}
		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	const Wt::WFormModel::Field RegionFormModel::nameField = "name";

	RegionFormModel::RegionFormModel(RegionView *view, Wt::Dbo::ptr<Region> regionPtr /*= Wt::Dbo::ptr<Region>()*/)
		: RecordFormModel(view, regionPtr), _view(view)
	{
		addField(nameField);

		if(_recordPtr)
		{
			WApplication *app = APP;
			TRANSACTION(app);

			setValue(nameField, Wt::WString::fromUTF8(_recordPtr->name));
		}
	}

	Wt::WWidget *RegionFormModel::createFormWidget(Field field)
	{
		if(field == nameField)
		{
			Wt::WLineEdit *hostName = new Wt::WLineEdit();
			hostName->setMaxLength(70);
			Wt::WLengthValidator *validator = new Wt::WLengthValidator(0, 70);
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
			_recordPtr = app->dboSession().add(new Region());

		_recordPtr.modify()->name = valueText(nameField).toUTF8();
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

	RegionView::RegionView(Wt::Dbo::ptr<Region> regionPtr /*= Wt::Dbo::ptr<Region>()*/)
		: RecordFormView(tr("GS.Admin.RegionView")), _tempPtr(regionPtr)
	{ }

	void RegionView::initView()
	{
		_model = new RegionFormModel(this, _tempPtr);
		addFormModel("region", _model);
	}

	Wt::WString RegionView::viewName() const
	{
		TRANSACTION(APP);
		return tr("RegionViewName").arg(regionPtr()->name);
	}

	RegionProxyModel::RegionProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<Region>> *sourceModel, Wt::WObject *parent /*= nullptr*/)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &RegionProxyModel::addAdditionalRows);
	}

	void RegionProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
			setData(index(0, 0), Wt::WString::tr("None"));
	}

	Wt::WValidator::Result LoginNameValidator::validate(const Wt::WString &input) const
	{
		Wt::WValidator::Result res = Wt::WValidator::validate(input);
		if(res.state() != Wt::WValidator::Valid)
			return res;

		if(!_allowedName.empty() && input == _allowedName)
			return res;

		bool valid = true;
		Wt::WString error;
		if(input.toUTF8().length() < _minLoginNameLength)
			error = Wt::WString::tr("Wt.Auth.user-name-tooshort").arg(_minLoginNameLength);

		if(!error.empty())
			valid = false;
		else
		{
			Wt::Auth::User existingUser = APP->userDatabase().findWithIdentity(Wt::Auth::Identity::LoginName, input);
			valid = !existingUser.isValid();

			if(existingUser.isValid())
				error = Wt::WString::tr("Wt.Auth.user-name-exists");
		}

		if(!valid)
			return Wt::WValidator::Result(Wt::WValidator::Invalid, error);
		return res;
	}
}