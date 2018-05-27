#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WColor.h>
#include <Wt/WCssDecorationStyle.h>

using namespace Wt;

//My Classes
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
	virtual void propagateSetVisible(bool visible) override;
	virtual void render(Wt::WFlags<Wt::RenderFlag> flags) override;
	bool _wasVisible = true;
};

class ReloadableContainer : public ReloadOnVisibleWidget<Wt::WContainerWidget>
{
protected:
	virtual void reload() override
	{
		decorationStyle().setBackgroundColor(WColor(rand() % 256, rand() % 256, rand() % 256));
		for(auto w : children())
		{
			if(auto rW = dynamic_cast<Reloadable*>(w))
				rW->reload();
		}
	}
};

class ReloadableText : public WText, public Reloadable
{
public:
	ReloadableText(const WString &text, WContainerWidget *parent = nullptr) : WText(text, parent) { }

protected:
	virtual void reload() override { decorationStyle().setForegroundColor(WColor(rand() % 256, rand() % 256, rand() % 256)); }
};

class HelloApplication : public WApplication
{
public:
	HelloApplication(const WEnvironment& env);
};

//MAIN LOGIC
HelloApplication::HelloApplication(const WEnvironment& env)
  : WApplication(env)
{
	setTitle("HELP ME :'(");

	auto container = new ReloadableContainer();
	root()->addWidget(container);
	new ReloadableText("Text 1", container);
	new ReloadableText("Text 2", container);
	new ReloadableText("Text 3", container);
	new ReloadableText("Text 4", container);
	new ReloadableText("Text 5", container);
	new ReloadableText("Text 6", container);

	auto btn = new WPushButton("Show/Hide", root());
	btn->clicked().connect(std::bind([container]() {
		container->setHidden(!container->isHidden());
	}));
}

//MAIN
WApplication *createApplication(const WEnvironment& env) { return new HelloApplication(env); }
int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

//TEMPLATE CLASS DEFINITIONS
template<class Base>
void ReloadOnVisibleWidget<Base>::propagateSetVisible(bool visible)
{
	scheduleRender();
	Base::propagateSetVisible(visible);
}

template<class Base>
void ReloadOnVisibleWidget<Base>::render(Wt::WFlags<Wt::RenderFlag> flags)
{
	if(canOptimizeUpdates() && (flags & Wt::RenderFull) == 0)
	{
		bool visible = isVisible();
		if(visible && !_wasVisible)
			reload();

		_wasVisible = visible;
	}

	Base::render(flags);
}
