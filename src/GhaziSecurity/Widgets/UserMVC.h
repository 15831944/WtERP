#ifndef GS_USER_MVC_H
#define GS_USER_MVC_H

#include "Dbo/Dbos.h"
#include "Utilities/FilteredList.h"
#include "Utilities/RecordFormView.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/Dbo/QueryModel>
#include <Wt/WBatchEditProxyModel>

namespace GS
{
	class UserView;
	class RegionView;

	//USERS
	class UserListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		UserListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class UserList : public QueryModelFilteredList<boost::tuple<long long, std::string, std::string, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResLoginName, ResEmail, ResRegionName };
		enum ViewColumns { ViewId, ViewLoginName, ViewEmail, ViewRegionName };

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class RegionProxyModel : public QueryProxyModel<Wt::Dbo::ptr<Region>>
	{
	public:
		RegionProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<Region>> *sourceModel, Wt::WObject *parent = nullptr);

	protected:
		void addAdditionalRows();
	};

	class LoginNameValidator : public Wt::WValidator
	{
	public:
		LoginNameValidator(Wt::WObject *parent = nullptr) : Wt::WValidator(true, parent) { }
		virtual Result validate(const Wt::WString &input) const override;
		int minLoginNameLength() const { return _minLoginNameLength; }
		void setMinLoginNameLength(int length) { _minLoginNameLength = length; }
		void setAllowedName(const Wt::WString &name) { _allowedName = name; }

	protected:
		Wt::WString _allowedName;
		int _minLoginNameLength = 4;
	};

	class UserFormModel : public RecordFormModel<User>
	{
	public:
		static const Wt::WFormModel::Field loginNameField;
		static const Wt::WFormModel::Field passwordField;
		static const Wt::WFormModel::Field password2Field;
		static const Wt::WFormModel::Field emailField;
		static const Wt::WFormModel::Field regionField;
		static const Wt::WFormModel::Field permissionsField;

		enum PermissionCBValue
		{
			RegionalUser, RegionalAdministrator, GlobalAdministrator, PermissionCBValues
		};
		Permissions::GSPermissions permissionIndexToId(int idx) const { return idx < PermissionCBValues ? _permissionIdxToId[idx] : Permissions::RegionalUser; }

		UserFormModel(UserView *view, Wt::Dbo::ptr<AuthInfo> autoInfoPtr = Wt::Dbo::ptr<AuthInfo>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;
		virtual bool validateField(Field field);
		Wt::Dbo::ptr<AuthInfo> authInfoPtr() const { return _authInfoPtr; }

		virtual AuthLogin::PermissionResult checkViewPermission() const override;
		virtual AuthLogin::PermissionResult checkModifyPermission() const override;
		virtual AuthLogin::PermissionResult checkCreatePermission() const override;

	protected:
		virtual void persistedHandler() override;
		void handleChangePassword();

		UserView *_view = nullptr;
		Wt::Dbo::ptr<AuthInfo> _authInfoPtr;
		const Permissions::GSPermissions _permissionIdxToId[PermissionCBValues] = { Permissions::RegionalUser, Permissions::RegionalAdministrator, Permissions::GlobalAdministrator };
	};

	class UserView : public RecordFormView
	{
	public:
		UserView(Wt::Dbo::ptr<AuthInfo> authInfoPtr = Wt::Dbo::ptr<AuthInfo>());
		virtual void initView() override;

		Wt::Dbo::ptr<AuthInfo> authInfoPtr() const { return _model->authInfoPtr(); }
		UserFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return authInfoPtr() ? User::viewInternalPath(authInfoPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new UserView(); }

	protected:
		UserFormModel *_model = nullptr;
		Wt::Dbo::ptr<AuthInfo> _tempPtr;
	};

	//REGIONS
	class RegionListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		RegionListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class RegionList : public QueryModelFilteredList<boost::tuple<long long, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResName };
		enum ViewColumns { ViewId, ViewName };

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class RegionFormModel : public RecordFormModel<Region>
	{
	public:
		static const Wt::WFormModel::Field nameField;

		RegionFormModel(RegionView *view, Wt::Dbo::ptr<Region> regionPtr = Wt::Dbo::ptr<Region>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

		virtual AuthLogin::PermissionResult checkViewPermission() const override;
		virtual AuthLogin::PermissionResult checkModifyPermission() const override;
		virtual AuthLogin::PermissionResult checkCreatePermission() const override;

	protected:
		RegionView *_view = nullptr;
	};

	class RegionView : public RecordFormView
	{
	public:
		RegionView(Wt::Dbo::ptr<Region> regionPtr = Wt::Dbo::ptr<Region>());
		virtual void initView() override;

		Wt::Dbo::ptr<Region> regionPtr() const { return _model->recordPtr(); }
		RegionFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return regionPtr() ? Region::viewInternalPath(regionPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new RegionView(); }

	protected:
		RegionFormModel *_model = nullptr;
		Wt::Dbo::ptr<Region> _tempPtr;
	};

}

#endif