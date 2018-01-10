#ifndef GS_RELOADABLEWIDGET_UTILITY_H
#define GS_RELOADABLEWIDGET_UTILITY_H

#include "Common.h"

namespace GS
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
			scheduleRender();
			Base::propagateSetVisible(visible);
		}

		virtual void render(Wt::WFlags<Wt::RenderFlag> flags) override
		{
			isNotStateless();
			if(canOptimizeUpdates() && !flags.test(Wt::RenderFlag::Full))
			{
				bool visible = isVisible();
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
