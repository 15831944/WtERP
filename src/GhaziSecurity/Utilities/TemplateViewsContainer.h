#ifndef GS_TEMPLATEVIEWSCONTAINER_WIDGET_H
#define GS_TEMPLATEVIEWSCONTAINER_WIDGET_H

#include <Wt/WTemplateFormView>
#include <Wt/WContainerWidget>

namespace GS
{
	class AbstractTemplateViewsContainer : public Wt::WContainerWidget
	{
	protected:
		AbstractTemplateViewsContainer(Wt::WContainerWidget *parent = nullptr) : Wt::WContainerWidget(parent) { }

	public:
		virtual void reset() = 0;
		virtual void updateViews() = 0;
		virtual void updateModels() = 0;
		virtual bool validateAll() = 0;
	};

	template<class View, class ViewFormModel>
	class TemplateViewsContainer : public AbstractTemplateViewsContainer
	{
	public:
		TemplateViewsContainer(Wt::WContainerWidget *parent = nullptr) : AbstractTemplateViewsContainer(parent) { }
		virtual void reset() override
		{
			for(int i = 0; i < count(); ++i)
			{
				auto lw = viewWidget(i);
				lw->model()->reset();
				lw->updateView(lw->model());
				lw->updateModel(lw->model());
			}
		}
		virtual void updateViews() override
		{
			for(int i = 0; i < count(); ++i)
			{
				auto w = viewWidget(i);
				w->updateView(w->model());
			}
		}
		virtual void updateModels() override
		{
			for(int i = 0; i < count(); ++i)
			{
				auto w = viewWidget(i);
				w->updateModel(w->model());
			}
		}
		virtual bool validateAll() override
		{
			bool result = true;
			for(int i = 0; i < count(); ++i)
			{
				auto model = viewModel(i);
				if(!model->validate())
					result = false;
			}

			return result;
		}

		View *viewWidget(int index) const
		{
			return dynamic_cast<View*>(widget(index));
		}
		ViewFormModel *viewModel(int index) const
		{
			if(auto w = viewWidget(index))
				return w->model();
			else
				return nullptr;
		}
	};

}

#endif