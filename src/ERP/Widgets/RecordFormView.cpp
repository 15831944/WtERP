#include "Widgets/RecordFormView.h"
#include "ModelView/EntityList.h"
#include "ModelView/EntityView.h"
#include "ModelView/AccountMVC.h"
#include "Widgets/ImageUpload.h"

#include <Wt/WDateEdit.h>
#include <Wt/WTimeEdit.h>

namespace ERP
{
	//RECORD FORM VIEW

	void RecordFormView::load()
	{
		if(!loaded())
		{
			_submitBtn = bindNew<ShowEnabledButton>("submitBtn", tr("Submit"));
			_submitBtn->clicked().connect(this, &RecordFormView::submit);

			_editBtn = bindNew<ShowEnabledButton>("editBtn", tr("Edit"));
			_editBtn->clicked().connect(this, &RecordFormView::handleEditBtn);

			_reloadBtn = bindNew<Wt::WPushButton>("reloadBtn", tr("Refresh"));
			_reloadBtn->clicked().connect(this, &RecordFormView::reload);

			_rereadModelDbos();
			initView();
			
			bool permitted = checkPermissions();
			if(!permitted)
			{
				_loadState = PermissionDeniedState;
				throw PermissionDeniedException();
			}
			
			_updateModelsFromDb();
			
			for(const auto &val : _modelVector)
			{
				if(val.second->isRecordPersisted() || val.second->isProxyModel())
					val.second->persistedHandler();
			}

			_loadState = ValidState;
		}

		Wt::WTemplateFormView::load();
	}

	void RecordFormView::reload()
	{
		if(!loaded())
			return;

		if((isValidLoadState() || _loadState == ErrorLoadingState) && !isWriteMode())
		{
			updateModelsFromDb();
			updateView();
		}
	}

	void RecordFormView::updateModelsFromDb()
	{
		try
		{
			TRANSACTION(APP);
			bool permitted = checkPermissions();
			if(!permitted)
			{
				_loadState = PermissionDeniedState;
				return;
			}
			
			_rereadModelDbos();
			_updateModelsFromDb();
			t.commit();

			_loadState = ValidState;
		}
		catch(const Dbo::ObjectNotFoundException &)
		{
			_loadState = NotFoundState;
		}
		catch(const Dbo::Exception &e)
		{
			Wt::log("error") << "RecordFormView::updateModelsFromDb(): (View: " << viewName() << ") Dbo error(" << e.code() << "): " << e.what();
			_loadState = ErrorLoadingState;
		}
	}

	void RecordFormView::_rereadModelDbos()
	{
		for(const auto &val : _modelVector)
		{
			if(val.second->isRecordPersisted() || val.second->isProxyModel())
				val.second->rereadDbos();
		}
	}

	void RecordFormView::_updateModelsFromDb()
	{
		for(const auto &val : _modelVector)
		{
			if(val.second->isRecordPersisted() || val.second->isProxyModel())
				val.second->updateFromDb();
		}
	}

	bool RecordFormView::checkPermissions()
	{
		TRANSACTION(APP);
		_permissionFlags.set();
		for(const auto &res : _modelVector)
		{
			res.second->setAllReadOnly(false);

			bool isPersisted = res.second->isRecordPersisted();
			setCondition("m:" + std::string(res.first) + "-transient", !isPersisted);
			setCondition("m:" + std::string(res.first) + "-persisted", isPersisted);
			if(isPersisted)
				_permissionFlags[AllTransient] = false;

			AuthLogin::PermissionResult viewPermission = res.second->checkViewPermission();
			setCondition("m:" + std::string(res.first) + "-view", viewPermission == AuthLogin::Permitted);
			if(viewPermission == AuthLogin::Permitted)
				_permissionFlags[NoViewPermission] = false;

			AuthLogin::PermissionResult modifyPermission = res.second->checkModifyPermission();
			setCondition("m:" + std::string(res.first) + "-modify", modifyPermission == AuthLogin::Permitted);
			if(modifyPermission == AuthLogin::Permitted)
				_permissionFlags[NoModifyPermission] = false;
			else
			{
				if(isPersisted)
					res.second->setAllReadOnly(true);
			}

			AuthLogin::PermissionResult createPermission = res.second->checkCreatePermission();
			setCondition("m:" + std::string(res.first) + "-create", createPermission == AuthLogin::Permitted);
			if(createPermission == AuthLogin::Permitted)
				_permissionFlags[NoCreatePermission] = false;
			else
			{
				if(isPersisted)
					res.second->setAllReadOnly(true);
			}

			if(!isWriteMode())
				res.second->setAllReadOnly(true);

			if(!res.second->isAllReadOnly())
				_permissionFlags[AllReadOnly] = false;
		}

		_editBtn->setEnabled(!_permissionFlags[NoModifyPermission] && !_permissionFlags[AllTransient]);

		if(_permissionFlags[AllTransient])
		{
			_submitBtn->setDisabled(_permissionFlags[NoCreatePermission] || _permissionFlags[AllReadOnly]);
			if(_permissionFlags[NoCreatePermission])
				return false;
		}
		else
		{
			_submitBtn->setDisabled(_permissionFlags[NoModifyPermission] || _permissionFlags[AllReadOnly]);
			if(_permissionFlags[NoViewPermission])
				return false;
		}
		return true;
	}

	void RecordFormView::submit()
	{
		if(!loaded() || _modelVector.empty() || _submitBtn->isDisabled() || !isEnabled() || !isWriteMode() || !isValidLoadState())
			return;

		WApplication *app = APP;
		TRANSACTION(app);

		bool nothingSaved = true;
		try
		{
			bool permitted = checkPermissions();
			if(_permissionFlags[NoViewPermission] || _permissionFlags[AllReadOnly])
				permitted = false;
			if(_permissionFlags[AllTransient])
			{
				if(_permissionFlags[NoCreatePermission])
					permitted = false;
			}
			else
			{
				if(_permissionFlags[NoModifyPermission])
					permitted = false;
			}
			if(!permitted)
			{
				_loadState = PermissionDeniedState;
				updateView();
				return;
			}

			updateModel();
			bool valid = validateAll();
			if(!valid)
			{
				updateView();
				return;
			}

			for(const auto &res : _modelVector)
			{
				if(res.second->isAllReadOnly() || res.second->checkSubmitPermission() != AuthLogin::Permitted)
					continue;

				bool wasPersisted = res.second->isRecordPersisted();
				if(res.second->saveChanges())
				{
					nothingSaved = false;
					if(!wasPersisted)
						res.second->persistedHandler();
				}
			}

			t.commit();
			afterSubmitHandler();
		}
		catch(const Dbo::StaleObjectException &)
		{
			app->dboSession().rereadAll();
			app->showStaleObjectError();
			nothingSaved = true;
		}
		catch(const Dbo::Exception &e)
		{
			Wt::log("error") << "RecordFormView::submit(): (View: " << viewName() << ") Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
			nothingSaved = true;
		}

		if(nothingSaved)
			resetValidationAll();
		else
			_editModeEnabled = false;
		
		if(!checkPermissions())
			_loadState = PermissionDeniedState;

		updateView();
		if(!nothingSaved)
			submitted().emit();
	}

	void RecordFormView::render(Wt::WFlags<Wt::RenderFlag> flags)
	{
		if(canOptimizeUpdates() && flags.test(Wt::RenderFlag::Full))
			updateView();
	}

	void RecordFormView::updateView()
	{
		setCondition("s:valid", isValidLoadState());
		setCondition("s:not-found", _loadState == NotFoundState);
		setCondition("s:error", _loadState == ErrorLoadingState);
		setCondition("s:permission-denied", _loadState == PermissionDeniedState);
		if(!isValidLoadState())
			return;

		for(const auto &res : _modelVector)
		{
			if(conditionValue("m:" + std::string(res.first) + "-view"))
				updateView(res.second.get());
		}
	}

	void RecordFormView::updateModel()
	{
		if(!isValidLoadState())
			return;

		for(const auto &res : _modelVector)
			updateModel(res.second.get());
	}

	bool RecordFormView::validateAll()
	{
		if(!isValidLoadState())
			return false;

		bool valid = true;
		for(const auto &res : _modelVector)
		{
			if(!res.second->validate())
				valid = false;
		}
		return valid;
	}

	void RecordFormView::resetValidationAll()
	{
		for(const auto &res : _modelVector)
			res.second->resetValidation();
	}

	unique_ptr<Wt::WWidget> RecordFormView::createFormWidget(Wt::WFormModel::Field field)
	{
		for(const auto &res : _modelVector)
		{
			if(auto w = res.second->createFormWidget(field))
				return w;
		}
		return nullptr;
	}

	void RecordFormView::handleEditBtn()
	{
		_editModeEnabled = !isWriteMode();
		if(_editModeEnabled)
			_editBtn->setText(tr("Cancel"));
		else
			_editBtn->setText(tr("Edit"));

		updateModelsFromDb();
		updateView();
	}

	Wt::WString RecordFormView::templateText() const
	{
		return tr("ERP.RecordFormView").arg(Wt::WTemplateFormView::templateText());
	}

	bool RecordFormView::isWriteMode() const
	{
		return _editModeEnabled || (_firstModel && !_firstModel->isRecordPersisted());
	}

	bool RecordFormView::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit)
	{
		if(Wt::WTemplateFormView::updateViewValue(model, field, edit))
			return true;

		if(auto dateEdit = dynamic_cast<Wt::WDateEdit*>(edit))
		{
			const Wt::any &v = model->value(field);
			if(v.empty())
				dateEdit->setDate(Wt::WDate());
			else
				dateEdit->setDate(Wt::any_cast<Wt::WDate>(v));
			return true;
		}
		if(auto timeEdit = dynamic_cast<Wt::WTimeEdit*>(edit))
		{
			const Wt::any &v = model->value(field);
			if(v.empty())
				timeEdit->setTime(Wt::WTime());
			else
				timeEdit->setTime(Wt::any_cast<Wt::WTime>(v));
			return true;
		}
		if(auto proxyModelComboBox = dynamic_cast<AbstractQueryProxyModelCB*>(edit))
		{
			proxyModelComboBox->setViewValue(model->value(field));
			return true;
		}
		else if(auto comboBox = dynamic_cast<Wt::WComboBox*>(edit))
		{
			const Wt::any &v = model->value(field);
			if(!v.empty())
				comboBox->setCurrentIndex(Wt::any_cast<int>(v));
			return true;
		}
		if(auto findRecordEdit = dynamic_cast<AbstractFindRecordEdit*>(edit))
		{
			const Wt::any &v = model->value(field);
			findRecordEdit->setValuePtr(v);
			return true;
		}
		if(auto viewsContainer = dynamic_cast<RecordViewsContainer*>(edit))
		{
			const Wt::any &v = model->value(field);

			if(v.empty())
				viewsContainer->reset();
			else
				viewsContainer->updateViews();

			return true;
		}
		if(auto imageUpload = dynamic_cast<ImageUpload*>(edit))
		{
			const Wt::any &v = model->value(field);
			if(!v.empty())
				imageUpload->setImageInfo(Wt::any_cast<UploadedImage>(v));

			return true;
		}
		if(auto heightEdit = dynamic_cast<HeightEdit*>(edit))
		{
			const Wt::any &v = model->value(field);
			if(v.empty())
				heightEdit->setValueInCm(-1);
			else
				heightEdit->setValueInCm(Wt::any_cast<float>(v));
			return true;
		}
		return false;
	}

	bool RecordFormView::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit)
	{
		if(Wt::WTemplateFormView::updateModelValue(model, field, edit))
			return true;

		if(auto proxyModelComboBox = dynamic_cast<AbstractQueryProxyModelCB*>(edit))
		{
			model->setValue(field, proxyModelComboBox->modelValue());
			return true;
		}
		else if(auto comboBox = dynamic_cast<Wt::WComboBox*>(edit))
		{
			model->setValue(field, comboBox->currentIndex());
			return true;
		}
		if(auto dateEdit = dynamic_cast<Wt::WDateEdit*>(edit))
		{
			model->setValue(field, dateEdit->date());
			return true;
		}
		if(auto timeEdit = dynamic_cast<Wt::WTimeEdit*>(edit))
		{
			model->setValue(field, timeEdit->time());
			return true;
		}
		if(auto findRecordEdit = dynamic_cast<AbstractFindRecordEdit*>(edit))
		{
			model->setValue(field, findRecordEdit->valueAny());
			return true;
		}
		if(auto imageUpload = dynamic_cast<ImageUpload*>(edit))
		{
			model->setValue(field, imageUpload->imageInfo());
			return true;
		}
		if(auto heightEdit = dynamic_cast<HeightEdit*>(edit))
		{
			model->setValue(field, heightEdit->valueInCm());
			return true;
		}
		if(auto viewsContainer = dynamic_cast<RecordViewsContainer*>(edit))
		{
			viewsContainer->updateModels();
			return true;
		}
		return false;
	}

	//OTHERS

	RecordFormView *RecordViewsContainer::viewWidget(int index) const
	{
		return dynamic_cast<RecordFormView*>(widget(index));
	}

	bool RecordViewsContainer::validateAll()
	{
		bool result = true;
		for(int i = 0; i < count(); ++i)
		{
			auto view = viewWidget(i);
			if(!view->validateAll())
				result = false;
		}

		return result;
	}

	void RecordViewsContainer::updateModels()
	{
		for(int i = 0; i < count(); ++i)
		{
			RecordFormView *view = viewWidget(i);
			view->updateModel();
		}
		_model->updateModelValue();
	}

	void RecordViewsContainer::updateViews()
	{
		for(int i = 0; i < count(); ++i)
		{
			RecordFormView *view = viewWidget(i);
			view->updateView();
		}
		_model->updateContainer(this);
	}

	void RecordViewsContainer::reset()
	{
		for(int i = 0; i < count(); ++i)
		{
			RecordFormView *view = viewWidget(i);
			_model->reset();
			view->updateView();
			view->updateModel();
		}
	}

	void RecordViewsContainer::addRecordView()
	{
		_model->updateModelValue();
		_model->addRecordPtr();
		_model->updateContainer(this);
	}

	bool AbstractRecordFormModel::validateUpdateField(Wt::WFormModel::Field field)
	{
		_view->updateModelField(this, field);
		bool res = validateField(field);
		_view->updateViewField(this, field);
		return res;
	}

	void ShowEnabledButton::propagateSetEnabled(bool enabled)
	{
		setHidden(!enabled);
		Wt::WPushButton::propagateSetEnabled(enabled);
	}
}