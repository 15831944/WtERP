#include "Utilities/MyFormView.h"
#include "Utilities/TemplateViewsContainer.h"
#include "Widgets/EntityList.h"
#include "Widgets/EntityView.h"
#include "Widgets/ImageUpload.h"
#include "Widgets/AccountMVC.h"

#include <Wt/WDateEdit>
#include <Wt/WComboBox>

namespace GS
{

	bool MyTemplateFormView::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit)
	{
		if(Wt::WTemplateFormView::updateViewValue(model, field, edit))
			return true;

		if(auto dateEdit = dynamic_cast<Wt::WDateEdit*>(edit))
		{
			const boost::any &v = model->value(field);
			if(v.empty())
				dateEdit->setDate(Wt::WDate());
			else
				dateEdit->setDate(boost::any_cast<Wt::WDate>(v));
			return true;
		}
		if(auto proxyModelComboBox = dynamic_cast<AbstractProxyModelComboBox*>(edit))
		{
			proxyModelComboBox->setViewValue(model->value(field));
			return true;
		}
		else if(auto comboBox = dynamic_cast<Wt::WComboBox*>(edit))
		{
			const boost::any &v = model->value(field);
			if(!v.empty())
				comboBox->setCurrentIndex(boost::any_cast<int>(v));
			return true;
		}
		if(auto findEntity = dynamic_cast<FindEntityEdit*>(edit))
		{
			const boost::any &v = model->value(field);
			if(v.empty())
				findEntity->setEntityPtr(Wt::Dbo::ptr<Entity>());
			else
				findEntity->setEntityPtr(boost::any_cast<Wt::Dbo::ptr<Entity>>(v));
			return true;
		}
		if(auto findAccount = dynamic_cast<FindAccountEdit*>(edit))
		{
			const boost::any &v = model->value(field);
			if(v.empty())
				findAccount->setAccountPtr(Wt::Dbo::ptr<Account>());
			else
				findAccount->setAccountPtr(boost::any_cast<Wt::Dbo::ptr<Account>>(v));
			return true;
		}
		if(auto viewsContainer = dynamic_cast<AbstractTemplateViewsContainer*>(edit))
		{
			const boost::any &v = model->value(field);

			if(v.empty())
				viewsContainer->reset();
			else
				viewsContainer->updateViews();

			return true;
		}
		if(auto imageUpload = dynamic_cast<ImageUpload*>(edit))
		{
			const boost::any &v = model->value(field);
			if(!v.empty())
				imageUpload->setImageInfo(boost::any_cast<UploadedImage>(v));

			return true;
		}
		if(auto heightEdit = dynamic_cast<HeightEdit*>(edit))
		{
			const boost::any &v = model->value(field);
			if(v.empty())
				heightEdit->setValueInCm(-1);
			else
				heightEdit->setValueInCm(boost::any_cast<float>(v));
			return true;
		}
		return false;
	}

	bool MyTemplateFormView::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit)
	{
		if(Wt::WTemplateFormView::updateModelValue(model, field, edit))
			return true;

		if(auto dateEdit = dynamic_cast<Wt::WDateEdit*>(edit))
		{
			model->setValue(field, dateEdit->date());
			return true;
		}
		if(auto proxyModelComboBox = dynamic_cast<AbstractProxyModelComboBox*>(edit))
		{
			model->setValue(field, proxyModelComboBox->modelValue());
			return true;
		}
		else if(auto comboBox = dynamic_cast<Wt::WComboBox*>(edit))
		{
			model->setValue(field, comboBox->currentIndex());
			return true;
		}
		if(auto findEntity = dynamic_cast<FindEntityEdit*>(edit))
		{
			model->setValue(field, findEntity->entityPtr());
			return true;
		}
		if(auto findAccount = dynamic_cast<FindAccountEdit*>(edit))
		{
			model->setValue(field, findAccount->accountPtr());
			return true;
		}
		if(auto viewsContainer = dynamic_cast<AbstractTemplateViewsContainer*>(edit))
		{
			viewsContainer->updateModels();
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
		return false;
	}

}