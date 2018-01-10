#ifndef GS_RECORDFORMVIEW_H
#define GS_RECORDFORMVIEW_H

#include "Common.h"
#include "Utilities/QueryProxyMVC.h"
#include "Application/WApplication.h"

#include <Wt/WTemplateFormView.h>
#include <Wt/WPushButton.h>
#include <Wt/WContainerWidget.h>

namespace GS
{
	class AbstractRecordFormModel;
	class AbstractMultipleRecordModel;
	class RecordFormView;

	class SubmittableRecordWidget
	{
	public:
		virtual Wt::WString viewName() const { return tr("Unknown"); }
		virtual std::string viewInternalPath() const { return ""; }
		virtual unique_ptr<RecordFormView> createFormView() { return nullptr; }

		Wt::Signal<> &submitted() { return _submitted; }

	private:
		Wt::Signal<> _submitted;
	};

	//VIEWS
	class RecordFormView : public Wt::WTemplateFormView, public SubmittableRecordWidget
	{
	public:
		typedef const char *ModelKey;

		RecordFormView() = default;
		RecordFormView(const Wt::WString &text) : Wt::WTemplateFormView(text) { }

		virtual void load() override;
		virtual void render(Wt::WFlags<Wt::RenderFlag> flags) override;
		virtual bool updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual bool updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual Wt::WString templateText() const override;

		void updateView();
		void updateModel();
		bool validateAll();
		void resetValidationAll();

		virtual Wt::WString recordName() const { return tr("Unknown"); }

		shared_ptr<AbstractRecordFormModel> model() { return _firstModel; }
		Wt::WPushButton *submitBtn() { return _submitBtn; }

	protected:
		virtual void initView() { }
		virtual void afterSubmitHandler() { }
		virtual void submit();
		virtual unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;

		typedef pair<ModelKey, shared_ptr<AbstractRecordFormModel>> ModelKeyPair;
		typedef std::vector<ModelKeyPair> FormModelVector;
		void addFormModel(ModelKey key, shared_ptr<AbstractRecordFormModel> model);
		FormModelVector &modelVector() { return _modelVector; }

		using Wt::WTemplateFormView::updateView;
		using Wt::WTemplateFormView::updateModel;

	private:
		void handleSubmitted();

		enum ViewFlags { AllTransient, NoViewPermission, NoModifyPermission, NoCreatePermission, AllReadOnly, FlagsCount };
		std::bitset<FlagsCount> _viewFlags;

		FormModelVector _modelVector;
		shared_ptr<AbstractRecordFormModel> _firstModel = nullptr;
		Wt::WPushButton *_submitBtn = nullptr;
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
		virtual void propagateSetEnabled(bool enabled) override { setHidden(!enabled); }
	};

	//MODELS

	class AbstractRecordFormModel : public Wt::WFormModel
	{
	public:
		virtual bool saveChanges() { return false; }
		virtual bool isRecordPersisted() const { return false; }
		RecordFormView *formView() const { return _view; }
		bool validateUpdateField(Wt::WFormModel::Field field);

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
	};

	template<class DboType>
	class RecordFormModel : public AbstractRecordFormModel
	{
	public:
		typedef typename DboType RecordDbo;
		RecordFormModel(RecordFormView *view, Dbo::ptr<RecordDbo> recordPtr) : AbstractRecordFormModel(view), _recordPtr(recordPtr) { }

		Dbo::ptr<RecordDbo> recordPtr() const { return _recordPtr; }
		virtual bool isRecordPersisted() const override { return !_recordPtr.isTransient(); }

		virtual AuthLogin::PermissionResult checkViewPermission() const override { return APP->authLogin().checkRecordViewPermission(recordPtr().get()); }
		virtual AuthLogin::PermissionResult checkModifyPermission() const override { return APP->authLogin().checkRecordModifyPermission(recordPtr().get()); }

	protected:
		Dbo::ptr<RecordDbo> _recordPtr;
	};

	template<class DboType>
	class ChildRecordFormModel : public RecordFormModel<DboType>
	{
	public:
		ChildRecordFormModel(shared_ptr<AbstractRecordFormModel> parentModel, RecordFormView *view, Dbo::ptr<RecordDbo> recordPtr = nullptr)
			: RecordFormModel(view, recordPtr), _parentModel(parentModel) { }

		virtual AuthLogin::PermissionResult checkViewPermission() const override { return _parentModel->checkViewPermission(); }
		virtual AuthLogin::PermissionResult checkModifyPermission() const override { return _parentModel->checkModifyPermission(); }

	protected:

		shared_ptr<AbstractRecordFormModel> _parentModel = nullptr;
	};

	class AbstractMultipleRecordModel : public AbstractRecordFormModel
	{
	public:
		virtual void updateContainer(RecordViewsContainer *container) = 0;
		virtual void updateModelValue() = 0;
		virtual void addRecordPtr() = 0;

	protected:
		AbstractMultipleRecordModel(RecordFormView *view) : AbstractRecordFormModel(view) { }
	};

	template<class DboType>
	class MultipleRecordModel : public AbstractMultipleRecordModel
	{
	public:
		typedef typename DboType RecordDbo;
		typedef std::vector<Dbo::ptr<RecordDbo>> PtrVector;
		typedef Dbo::collection<Dbo::ptr<RecordDbo>> PtrCollection;
		typedef RecordFormModel<RecordDbo> ModelType;
		typedef std::vector<shared_ptr<ModelType>> ModelVector;

		static const Field field;

		virtual void addRecordPtr() override { addRecordPtr(Dbo::ptr<RecordDbo>()); }
		virtual void addRecordPtr(Dbo::ptr<RecordDbo> ptr);
		virtual void updateContainer(RecordViewsContainer *container) override;
		virtual void updateModelValue() override;
		virtual bool saveChanges() override;
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field f) override { return f == field ? createContainer() : nullptr; }
		unique_ptr<RecordViewsContainer> createContainer() { return make_unique<RecordViewsContainer>(this); }
		virtual bool validate() override;

		const PtrVector &ptrVector() const { return Wt::any_cast<const PtrVector&>(value(field)); }

	protected:
		MultipleRecordModel(RecordFormView *view, PtrCollection collection = PtrCollection());
		tuple<unique_ptr<RecordFormView>, shared_ptr<ModelType>> createRecordView() { return createRecordView(Dbo::ptr<RecordDbo>()); }
		virtual tuple<unique_ptr<RecordFormView>, shared_ptr<ModelType>> createRecordView(Dbo::ptr<RecordDbo> recordPtr) = 0;

		ModelVector _modelVector;
	};

	//TEMPLATE CLASS DEFITIONS
	template<class DboType>
	MultipleRecordModel<DboType>::MultipleRecordModel(RecordFormView *view, PtrCollection collection /*= PtrCollection()*/)
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
		for(size_t i = container->count(); i < ptrs.size(); ++i)
		{
			auto createRes = createRecordView(ptrs[i]);
			auto w = std::move(std::get<0>(createRes));
			w->bindInt("index", i + 1);
			container->insertWidget(i, std::move(w));
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
}

#endif