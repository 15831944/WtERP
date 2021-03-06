#ifndef ERP_RECORDFORMVIEW_WIDGET_H
#define ERP_RECORDFORMVIEW_WIDGET_H

#include "Common.h"
#include "Widgets/QueryProxyMVC.h"
#include "Widgets/ReloadableWidget.h"
#include "Application/WApplication.h"

#include <Wt/WTemplateFormView.h>
#include <Wt/WPushButton.h>
#include <Wt/WContainerWidget.h>

namespace ERP
{
	class AbstractRecordFormModel;
	class AbstractMultipleRecordModel;
	class RecordFormView;

	class SubmittableRecordWidget : public ReloadOnVisibleWidget<Wt::WTemplateFormView>
	{
	public:
		virtual Wt::WString viewName() const { return tr("Unknown"); }
		virtual std::string viewInternalPath() const { return ""; }
		virtual unique_ptr<RecordFormView> createFormView() { return nullptr; }

		Wt::Signal<> &submitted() { return _submitted; }

	protected:
		template<typename ...Args>
		SubmittableRecordWidget(Args &&...args)
			: ReloadOnVisibleWidget<Wt::WTemplateFormView>(std::forward<Args>(args)...)
		{ }
		
	private:
		Wt::Signal<> _submitted;
	};

	//VIEWS
	class RecordFormView : public SubmittableRecordWidget
	{
	public:
		typedef const char *ModelKey;

		RecordFormView() = default;
		RecordFormView(const Wt::WString &text) : SubmittableRecordWidget(text) { }

		virtual void load() override;
		virtual void reload() override;
		virtual void render(Wt::WFlags<Wt::RenderFlag> flags) override;
		virtual bool updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual bool updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual Wt::WString templateText() const override;

		void updateView();
		void updateModel();
		bool validateAll();
		void resetValidationAll();

		enum LoadState { NotLoadedState, ValidState, ErrorLoadingState, NotFoundState, PermissionDeniedState };
		LoadState loadState() const { return _loadState; }
		bool isValidLoadState() const { return _loadState == ValidState; }
		bool isWriteMode() const;

		virtual Wt::WString recordName() const { return tr("Unknown"); }

		AbstractRecordFormModel *model() { return _firstModel; }
		Wt::WPushButton *submitBtn() { return _submitBtn; }
		Wt::WPushButton *modifyBtn() { return _editBtn; }
		Wt::WPushButton *reloadBtn() { return _reloadBtn; }

	protected:
		virtual void initView() { }
		virtual void afterSubmitHandler() { }
		virtual void submit();
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;

		bool checkPermissions();
		void updateModelsFromDb();

		typedef pair<ModelKey, unique_ptr<AbstractRecordFormModel>> ModelKeyPair;
		typedef std::vector<ModelKeyPair> FormModelVector;
		FormModelVector &modelVector() { return _modelVector; }

		template<class FormModel, typename ...Args>
		FormModel *newFormModel(ModelKey key, Args && ...a)
		{
			auto formModel = make_unique<FormModel>(std::forward<Args>(a)...);
			FormModel *res = formModel.get();

			if(!_firstModel) _firstModel = res;
			setCondition("m:" + std::string(key), true);
			_modelVector.emplace_back(key, move(formModel));
			
			//Recheck permissions if model is already loaded to make sure m:model condition is set
			if(_loadState != NotLoadedState)
			{
				if(!checkPermissions())
					_loadState = PermissionDeniedState;
			}
			
			return res;
		}

		using Wt::WTemplateFormView::updateView;
		using Wt::WTemplateFormView::updateModel;

	private:
		void _rereadModelDbos();
		void _updateModelsFromDb();
		void handleEditBtn();

		enum ViewFlags { AllTransient, NoViewPermission, NoModifyPermission, NoCreatePermission, AllReadOnly, ViewFlagsCount };
		std::bitset<ViewFlagsCount> _permissionFlags;
		LoadState _loadState = NotLoadedState;

		Wt::WPushButton *_submitBtn = nullptr;
		Wt::WPushButton *_editBtn = nullptr;
		Wt::WPushButton *_reloadBtn = nullptr;

		FormModelVector _modelVector;
		AbstractRecordFormModel *_firstModel = nullptr;
		bool _editModeEnabled = false;
	};

	class RecordViewsContainer : public Wt::WContainerWidget
	{
	public:
		RecordViewsContainer(AbstractMultipleRecordModel *model) : _model(model) { }
		void reset();
		void addRecordView();
		void updateViews();
		void updateModels();
		bool validateAll();

		RecordFormView *viewWidget(int index) const;

	protected:
		AbstractMultipleRecordModel *_model = nullptr;
	};

	class ShowEnabledButton : public Wt::WPushButton
	{
	public:
		ShowEnabledButton() { addStyleClass("hidden-print"); }
		ShowEnabledButton(const Wt::WString &text) : Wt::WPushButton(text) { addStyleClass("hidden-print"); }

	protected:
		virtual void propagateSetEnabled(bool enabled) override;
	};

	//MODELS

	class AbstractRecordFormModel : public Wt::WFormModel
	{
	public:
		virtual void rereadDbos() = 0;
		virtual void updateFromDb() = 0;
		virtual bool saveChanges() { return false; }
		virtual bool isRecordPersisted() const { return false; }
		virtual bool isProxyModel() const { return false; }
		bool validateUpdateField(Wt::WFormModel::Field field);
		RecordFormView *formView() const { return _view; }

		virtual AuthLogin::PermissionResult checkViewPermission() const { return AuthLogin::Permitted; }
		virtual AuthLogin::PermissionResult checkModifyPermission() const { return AuthLogin::Denied; }
		virtual AuthLogin::PermissionResult checkCreatePermission() const { return APP->authLogin().checkRecordCreatePermission(); }
		virtual AuthLogin::PermissionResult checkSubmitPermission() const { return isRecordPersisted() ? checkModifyPermission() : checkCreatePermission(); }

	protected:
		AbstractRecordFormModel(RecordFormView *view) : _view(view) { }
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) { return nullptr; }
		virtual void persistedHandler() { }

		RecordFormView *_view = nullptr;

	private:
		friend class RecordFormView;
		template<class DboType>
		friend class MultipleRecordModel;
	};

	template<class DboType>
	class RecordFormModel : public AbstractRecordFormModel
	{
	public:
		typedef DboType RecordDbo;
		RecordFormModel(RecordFormView *view, Dbo::ptr<RecordDbo> recordPtr)
			: AbstractRecordFormModel(view), _recordPtr(recordPtr)
		{ }

		Dbo::ptr<RecordDbo> recordPtr() const { return _recordPtr; }
		virtual void rereadDbos() override { _recordPtr.reread(); }
		virtual bool isRecordPersisted() const override { return !_recordPtr.isTransient(); }

		virtual AuthLogin::PermissionResult checkViewPermission() const override
		{
			TRANSACTION(APP);
			return APP->authLogin().checkRecordViewPermission(recordPtr().get());
		}
		virtual AuthLogin::PermissionResult checkModifyPermission() const override
		{
			TRANSACTION(APP);
			return APP->authLogin().checkRecordModifyPermission(recordPtr().get());
		}

	protected:
		Dbo::ptr<RecordDbo> _recordPtr;
	};

	template<class DboType>
	class ChildRecordFormModel : public RecordFormModel<DboType>
	{
	public:
		typedef DboType RecordDbo;
		ChildRecordFormModel(AbstractRecordFormModel *parentModel, RecordFormView *view, Dbo::ptr<RecordDbo> recordPtr = nullptr)
			: RecordFormModel<DboType>(view, recordPtr), _parentModel(parentModel) { }

		virtual AuthLogin::PermissionResult checkViewPermission() const override { return _parentModel->checkViewPermission(); }
		virtual AuthLogin::PermissionResult checkModifyPermission() const override { return _parentModel->checkModifyPermission(); }

	protected:
		AbstractRecordFormModel *_parentModel = nullptr;
	};

	class AbstractMultipleRecordModel : public AbstractRecordFormModel
	{
	public:
		virtual void updateContainer(RecordViewsContainer *container) = 0;
		virtual void updateModelValue() = 0;
		virtual void addRecordPtr() = 0;
		virtual bool isProxyModel() const override { return true; }

	protected:
		AbstractMultipleRecordModel(RecordFormView *view) : AbstractRecordFormModel(view) { }
	};

	template<class DboType>
	class MultipleRecordModel : public AbstractMultipleRecordModel
	{
	public:
		typedef DboType RecordDbo;
		typedef std::vector<Dbo::ptr<RecordDbo>> PtrVector;
		typedef Dbo::collection<Dbo::ptr<RecordDbo>> PtrCollection;
		typedef RecordFormModel<RecordDbo> ModelType;
		typedef std::vector<ModelType*> ModelVector;

		static const Field field;

		virtual void rereadDbos() override;
		virtual void updateFromDb() override;
		virtual void addRecordPtr() override { addRecordPtr(nullptr); }
		virtual void addRecordPtr(Dbo::ptr<RecordDbo> ptr);
		virtual void updateContainer(RecordViewsContainer *container) override;
		virtual void updateModelValue() override;
		virtual bool saveChanges() override;
		virtual bool validate() override;

		virtual unique_ptr<Wt::WWidget> createFormWidget(Field f) override { return f == field ? createContainer() : nullptr; }
		unique_ptr<RecordViewsContainer> createContainer() { return make_unique<RecordViewsContainer>(this); }

		const PtrVector &ptrVector() const { return Wt::any_cast<const PtrVector&>(value(field)); }

	protected:
		MultipleRecordModel(RecordFormView *view, PtrCollection collection = PtrCollection());
		tuple<unique_ptr<RecordFormView>, ModelType*> createRecordView() { return createRecordView(nullptr); }
		virtual tuple<unique_ptr<RecordFormView>, ModelType*> createRecordView(Dbo::ptr<RecordDbo> recordPtr) = 0;

		virtual void persistedHandler() override;

		ModelVector _modelVector;
	};

	//TEMPLATE CLASS DEFITIONS
	template<class DboType>
	MultipleRecordModel<DboType>::MultipleRecordModel(RecordFormView *view, PtrCollection collection)
		: AbstractMultipleRecordModel(view)
	{
		TRANSACTION(APP);
		PtrVector result(collection.begin(), collection.end());
		setValue(field, result);
	}

	template<class DboType>
	void MultipleRecordModel<DboType>::addRecordPtr(Dbo::ptr<RecordDbo> ptr)
	{
		PtrVector newVec(ptrVector());
		newVec.push_back(ptr);
		setValue(field, newVec);
	}

	template<class DboType>
	void MultipleRecordModel<DboType>::updateContainer(RecordViewsContainer *container)
	{
		auto ptrs = ptrVector();
		_modelVector.resize(ptrs.size(), nullptr);
		for(int i = container->count(); i < ptrs.size(); ++i)
		{
			auto createRes = createRecordView(ptrs[i]);
			auto w = move(std::get<0>(createRes));
			w->bindInt("index", i + 1);
			container->insertWidget(i, move(w));
			_modelVector[i] = std::get<1>(createRes);
		}
		for(int wCount = container->count(); wCount > ptrs.size(); --wCount)
			container->widget(wCount - 1)->removeFromParent();
	}

	template<class DboType>
	void MultipleRecordModel<DboType>::updateModelValue()
	{
		PtrVector result;
		for(const auto &res : _modelVector)
			result.push_back(res->recordPtr());

		setValue(field, result);
	}

	template<class DboType>
	bool MultipleRecordModel<DboType>::validate()
	{
		bool valid = true;
		for(const auto &res : _modelVector)
		{
			if(!res->validate())
				valid = false;
		}
		setValidation(field, Wt::WValidator::Result(valid ? Wt::ValidationState::Valid : Wt::ValidationState::Invalid));
		return valid;
	}

	template<class DboType>
	bool MultipleRecordModel<DboType>::saveChanges()
	{
		if(!valid())
			return false;

		auto vector = ptrVector();

		WApplication *app = APP;
		TRANSACTION(app);

		bool nothingSaved = true;
		for(const auto &model : _modelVector)
		{
			if(model->isAllReadOnly() || model->checkSubmitPermission() != AuthLogin::Permitted)
				continue;

			if(model->saveChanges())
				nothingSaved = false;
		}

		t.commit();
		updateModelValue();
		return !nothingSaved;
	}

	template<class DboType>
	void MultipleRecordModel<DboType>::persistedHandler()
	{
		for(const auto &model : _modelVector)
		{
			if(model->isRecordPersisted())
				model->persistedHandler();
		}
	}

	template<class DboType>
	void MultipleRecordModel<DboType>::rereadDbos()
	{
		for(const auto &model : _modelVector)
		{
			if(model->isRecordPersisted())
				model->rereadDbos();
		}
	}

	template<class DboType>
	void MultipleRecordModel<DboType>::updateFromDb()
	{
		for(const auto &model : _modelVector)
		{
			if(model->isRecordPersisted())
				model->updateFromDb();
		}
	}
}

#endif