#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WProgressBar.h>
#include <Wt/WTemplate.h>
#include <Wt/WMenu.h>
#include <Wt/WStackedWidget.h>

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

class ReloadableProgress : public WProgressBar, public Reloadable
{
public:
	ReloadableProgress(WContainerWidget *parent = nullptr) : WProgressBar(parent) { }
	virtual void reload() override;
};

class ReloadableProgressTemplate : public Wt::WTemplate, public Reloadable
{
public:
	ReloadableProgressTemplate(WContainerWidget *parent);
	virtual void reload() override;

protected:
	Wt::WText *_text = nullptr;
};

class ReloadableContainer : public ReloadOnVisibleWidget<Wt::WContainerWidget>
{
public:
	virtual void reload() override
	{
		//isNotStateless(); doesnt work either; already called previously on the call stack in ReloadOnVisibleWidget::render
		for(auto w : children())
		{
			if(auto rW = dynamic_cast<Reloadable*>(w))
				rW->reload();
		}
	}
};

class HelloApplication : public WApplication
{
public:
	HelloApplication(const WEnvironment& env);
};

//APPLICATION LOGIC
HelloApplication::HelloApplication(const WEnvironment& env)
	: WApplication(env)
{
	setTitle("Bug when reloading within render() during a learned slot event");

	//Irrelevant
	auto theme = new WBootstrapTheme();
	theme->setVersion(WBootstrapTheme::Version3);
	setTheme(theme);

	//WMenu with stacked widget change currentWidget using a learned slot
	Wt::WStackedWidget *stack = new Wt::WStackedWidget();
	auto menu = new WMenu(stack, root());
	root()->addWidget(stack);

	//Every Reloadable object's reload() is called when it becomes visible again
	auto reloadableContainer = new ReloadableContainer();
	menu->addItem("ReloadableContainer", reloadableContainer);
	(new ReloadableProgress(reloadableContainer))->setInline(false);
	new ReloadableProgressTemplate(reloadableContainer);

	//Just to hide reloadableContainer
	menu->addItem("Empty", new WContainerWidget());
}

//MAIN
WApplication *createApplication(const WEnvironment& env) { return new HelloApplication(env); }
int main(int argc, char **argv)
{
	return WRun(argc, argv, &createApplication);
}

//FUCNTION DEFINITIONS
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
		{
			isNotStateless();
			reload();
		}

		_wasVisible = visible;
	}

	Base::render(flags);
}

ReloadableProgressTemplate::ReloadableProgressTemplate(WContainerWidget *parent)
	: WTemplate(parent)
{
	setTemplateText(
		"<div class=\"progress\">"
		"${progress-bar class=\"progress-bar\"}"
		"</div>"
		);

	auto container = new Wt::WContainerWidget();
	container->setWidth(Wt::WLength(0, Wt::WLength::Percentage));
	_text = new Wt::WText("0%", container);
	bindWidget("progress-bar", container);
}

void ReloadableProgressTemplate::reload()
{
	auto container = resolveWidget("progress-bar");
	container->setWidth(Wt::WLength(container->width().value() + 5, WLength::Percentage));
	_text->setText(container->width().cssText());
	Wt::log("warn") << "ReloadableProgressTemplate::reload(): " << _text->text();
}

void ReloadableProgress::reload()
{
	setValue(value() + 5);
	Wt::log("warn") << "ReloadableProgress::reload(): " << value() << "%";
}