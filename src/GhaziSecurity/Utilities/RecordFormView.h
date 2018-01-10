#ifndef GS_RECORDFORMVIEW_H
#define GS_RECORDFORMVIEW_H

#include "Utilities/QueryProxyMVC.h"
#include "Application/WApplication.h"

#include <vector>
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
		virtual std::unique_ptr<RecordFormView> createFormView() { return nullptr; }

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

		std::shared_ptr<AbstractRecordFormModel> model() { return _firstModel; }
		Wt::WPushButton *submitBtn() { return _submitBtn; }

	protected:
		virtual void initView() { }
		virtual void afterSubmitHandler() { }
		virtual void submit();
		virtual std::unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override;

		typedef std::pair<ModelKey, std::shared_ptr<AbstractRecordFormModel>> ModelKeyPair;
		typedef std::vector<ModelKeyPair> FormModelVector;
		void addFormModel(ModelKey key, std::shared_ptr<AbstractRecordFormModel> model);
		FormModelVector &modelVector() { return _modelVector; }

		using Wt::WTemplateFormView::updateView;
		using Wt::WTemplateFormView::updateModel;

	private:
		void handleSubmitted();

		enum ViewFlags { AllTransient, NoViewPermission, NoModifyPermission, NoCreatePermission, AllReadOnly, FlagsCount };
		std::bitset<FlagsCount> _viewFlags;

		FormModelVector _modelVector;
		std::shared_ptr<AbstractRecordFormModel> _firstModel = nullptr;
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
		virtual std::unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) { return nullptr; }
		virtual void persistedHandler() { }

		RecordFormView *_view = nullptr;

	private:
		friend class RecordFormView;
	};

	template<class T>
	class RecordFormModel : public AbstractRecordFormModel
	{
	public:
		typedef typename T Dbo;
		RecordFormModel(RecordFormView *view, Wt::Dbo::ptr<Dbo> recordPtr) : AbstractRecordFormModel(view), _recordPtr(recordPtr) { }

		Wt::Dbo::ptr<Dbo> recordPtr() const { return _recordPtr; }
		virtual bool isRecordPersisted() const override { return !_recordPtr.isTransient(); }

		virtual AuthLogin::PermissionResult checkViewPermission() const override { return APP->authLogin().checkRecordViewPermission(recordPtr().get()); }
		virtual AuthLogin::PermissionResult checkModifyPermission() const override { return APP->authLogin().checkRecordModifyPermission(recordPtr().get()); }

	protected:
		Wt::Dbo::ptr<Dbo> _recordPtr;
	};

	template<class T>
	class ChildRecordFormModel : public RecordFormModel<T>
	{
	public:
		ChildRecordFormModel(std::shared_ptr<AbstractRecordFormModel> parentModel, RecordFormView *view, Wt::Dbo::ptr<Dbo> recordPtr = nullptr)
			: RecordFormModel(view, recordPtr), _parentModel(parentModel) { }

		virtual AuthLogin::PermissionResult checkViewPermission() const override { return _parentModel->checkViewPermission(); }
		virtual AuthLogin::PermissionResult checkModifyPermission() const override { return _parentModel->checkModifyPermission(); }

	protected:

		std::shared_ptr<AbstractRecordFormModel> _parentModel = nullptr;
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

	template<class RecordDbo>
	class MultipleRecordModel : public AbstractMultipleRecordModel
	{
	public:
		typedef typename RecordDbo Dbo;
		typedef std::vector<Wt::Dbo::ptr<Dbo>> PtrVector;
		typedef Wt::Dbo::collection<Wt::Dbo::ptr<Dbo>> PtrCollection;
		typedef RecordFormModel<Dbo> ModelType;
		typedef std::vector<std::shared_ptr<ModelType>> ModelVector;

		static const Field field;

		virtual void addRecordPtr() override { addRecordPtr(Wt::Dbo::ptr<Dbo>()); }
		virtual void addRecordPtr(Wt::Dbo::ptr<Dbo> ptr);
		virtual void updateContainer(RecordViewsContainer *container) override;
		virtual void updateModelValue() override;
		virtual bool saveChanges() override;
		virtual std::unique_ptr<Wt::WWidget> createFormWidget(Field f) override { return f == field ? createContainer() : nullptr; }
		std::unique_ptr<RecordViewsContainer> createContainer() { return std::make_unique<RecordViewsContainer>(this); }
		virtual bool validate() override;

		const PtrVector &ptrVector() const { return Wt::any_cast<const PtrVector&>(value(field)); }

	protected:
		MultipleRecordModel(RecordFormView *view, PtrCollection collection = PtrCollection());
		std::tuple<std::unique_ptr<RecordFormView>, std::shared_ptr<ModelType>> createRecordView() { return createRecordView(Wt::Dbo::ptr<Dbo>()); }
		virtual std::tuple<std::unique_ptr<RecordFormView>, std::shared_ptr<ModelType>> createRecordView(Wt::Dbo::ptr<Dbo> recordPtr) = 0;

		ModelVector _modelVector;
	};

	//TEMPLATE CLASS DEFITIONS
	template<class RecordDbo>
	MultipleRecordModel<RecordDbo>::MultipleRecordModel(RecordFormView *view, PtrCollection collection /*= PtrCollection()*/)
		: AbstractMultipleRecordModel(view)
	{
		TRANSACTION(APP);
		PtrVector result(collection.begin(), collection.end());
		setValue(field, result);
	}

	template<class RecordDbo>
	void MultipleRecordModel<RecordDbo>::addRecordPtr(Wt::Dbo::ptr<Dbo> ptr)
	{
		PtrVector newVec(ptrVector());
		newVec.push_back(ptr);
		setValue(field, newVec);
	}

	template<class RecordDbo>
	void MultipleRecordModel<RecordDbo>::updateContainer(RecordViewsContainer *container)
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

	template<class RecordDbo>
	void MultipleRecordModel<RecordDbo>::updateModelValue()
	{
		PtrVector result;
		for(const auto &res : _modelVector)
			result.push_back(res->recordPtr());

		setValue(field, result);
	}

	template<class RecordDbo>
	bool MultipleRecordModel<RecordDbo>::validate()
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

	template<class RecordDbo>
	bool MultipleRecordModel<RecordDbo>::saveChanges()
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