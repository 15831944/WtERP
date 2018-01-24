#ifndef ERP_RELOADABLEWIDGET_UTILITY_H
#define ERP_RELOADABLEWIDGET_UTILITY_H

#include "Common.h"

namespace ERP
{
	class Reloadable
	{
	public:
		Reloadable() { }
		virtual void reload() = 0;
	};

	template<class Base>
	class ReloadOnVisibleWidget : public Base, public Reloadable
	{
	public:
		ReloadOnVisibleWidget() { }
		
	protected:
		virtual void propagateSetVisible(bool visible) override
		{
			Base::scheduleRender();
			Base::propagateSetVisible(visible);
		}

		virtual void render(Wt::WFlags<Wt::RenderFlag> flags) override
		{
			Base::isNotStateless();
			if(Base::canOptimizeUpdates() && !flags.test(Wt::RenderFlag::Full))
			{
				bool visible = Base::isVisible();
				if(visible && !_wasVisible)
					reload();

				_wasVisible = visible;
			}
			Base::render(flags);
		}

		bool _wasVisible = true;
	};
}

#endif
