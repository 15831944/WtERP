#ifndef GS_RECORDFORMVIEW_H
#define GS_RECORDFORMVIEW_H

#include "Utilities/QueryProxyMVC.h"
#include "Application/WApplication.h"

#include <vector>
#include <Wt/WTemplateFormView>
#include <Wt/WPushButton>
#include <Wt/WContainerWidget>

namespace GS
{
	class AbstractRecordFormModel;
	class AbstractMultipleRecordModel;

	class SubmittableRecordWidget
	{
	public:
		SubmittableRecordWidget(Wt::WObject *parent = nullptr) : _submitted(parent) { }
		virtual Wt::WString viewName() const { return Wt::WString::tr("Unknown"); }
		virtual std::string viewInternalPath() const { return ""; }
		virtual SubmittableRecordWidget *createFormView() { return nullptr; }

		Wt::Signal<void> &submitted() { return _submitted; }

	private:
		Wt::Signal<void> _submitted;
	};

	//VIEWS
	class RecordFormView : public Wt::WTemplateFormView, public SubmittableRecordWidget
	{
	public:
		typedef const char *ModelKey;

		RecordFormView() : Wt::WTemplateFormView(), SubmittableRecordWidget(this) { addFunction("fwId", &Wt::WTemplate::Functions::fwId); }
		RecordFormView(const Wt::WString &text) : Wt::WTemplateFormView(text), SubmittableRecordWidget(this) { addFunction("fwId", &Wt::WTemplate::Functions::fwId); }

		virtual void load() override;
		virtual bool updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual bool updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual Wt::WString templateText() const override;

		void updateView();
		void updateModel();
		bool validateAll();
		void resetValidationAll();

		virtual Wt::WString recordName() const { return tr("Unknown"); }

		AbstractRecordFormModel *model() { return _firstModel; }
		Wt::WPushButton *submitBtn() { return _submitBtn; }

	protected:
		virtual void init() { }
		//virtual void initAfterUpdateView() { }
		virtual void afterSubmitHandler() { }
		virtual void submit();
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;

		typedef std::pair<ModelKey, AbstractRecordFormModel*> ModelKeyPair;
		typedef std::vector<ModelKeyPair> FormModelVector;
		void addFormModel(ModelKey key, AbstractRecordFormModel *model);
		FormModelVector &modelVector() { return _modelVector; }

		using Wt::WTemplateFormView::updateView;
		using Wt::WTemplateFormView::updateModel;

	private:
		void handleSubmitted();

		enum ViewFlags { AllTransient, NoViewPermission, NoModifyPermission, NoCreatePermission, AllReadOnly, FlagsCount };
		std::bitset<FlagsCount> _viewFlags;

		FormModelVector _modelVector;
		AbstractRecordFormModel *_firstModel = nullptr;
		Wt::WPushButton *_submitBtn = nullptr;
	};

	class RecordViewsContainer : public Wt::WContainerWidget
	{
	public:
		RecordViewsContainer(AbstractMultipleRecordModel *model, Wt::WContainerWidget *parent = nullptr) : Wt::WContainerWidget(parent), _model(model) { }
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
		ShowEnabledButton(Wt::WContainerWidget *parent = nullptr) : Wt::WPushButton(parent) { addStyleClass("hidden-print"); }
		ShowEnabledButton(const Wt::WString &text, Wt::WContainerWidget *parent = nullptr) : Wt::WPushButton(text, parent) { addStyleClass("hidden-print"); }

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
		AbstractRecordFormModel(RecordFormView *view) : Wt::WFormModel(view), _view(view) { }
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) { return nullptr; }

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
		ChildRecordFormModel(AbstractRecordFormModel *parentModel, RecordFormView *view, Wt::Dbo::ptr<Dbo> recordPtr = Wt::Dbo::ptr<Dbo>())
			: RecordFormModel(view, recordPtr), _parentModel(parentModel) { }

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
		typedef std::vector<ModelType*> ModelVector;

		static const Field field;

		virtual void addRecordPtr() override { addRecordPtr(Wt::Dbo::ptr<Dbo>()); }
		virtual void addRecordPtr(Wt::Dbo::ptr<Dbo> ptr);
		virtual void updateContainer(RecordViewsContainer *container) override;
		virtual void updateModelValue() override;
		virtual bool saveChanges() override;

		virtual RecordViewsContainer *createFormWidget(Field f) override { return f == field ? new RecordViewsContainer(this) : nullptr; }
		virtual bool validate() override;

		const PtrVector &ptrVector() const { return boost::any_cast<const PtrVector&>(value(field)); }

	protected:
		MultipleRecordModel(RecordFormView *view, PtrCollection collection = PtrCollection());
		std::tuple<RecordFormView*, ModelType*> createRecordView() { return createRecordView(Wt::Dbo::ptr<Dbo>()); }
		virtual std::tuple<RecordFormView*, ModelType*> createRecordView(Wt::Dbo::ptr<Dbo> recordPtr) = 0;

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
			auto w = std::get<0>(createRes);
			w->bindInt("index", i + 1);
			container->insertWidget(i, w);
			_modelVector[i] = std::get<1>(createRes);
		}
		for(int wCount = container->count(); wCount > ptrs.size(); --wCount)
			delete container->widget(wCount - 1);
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
		setValidation(field, Wt::WValidator::Result(valid ? Wt::WValidator::Valid : Wt::WValidator::Invalid));
		return valid;
	}

	template<class RecordDbo>
	bool MultipleRecordModel<RecordDbo>::saveChanges()
	{
		if(!valid())
			return false;

		RecordViewsContainer *container = _view->resolve<RecordViewsContainer*>(field);
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