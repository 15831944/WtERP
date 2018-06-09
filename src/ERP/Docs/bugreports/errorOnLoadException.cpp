#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>
#include <Wt/WStackedWidget.h>

class HelloApplication : public Wt::WApplication
{
public:
	HelloApplication(const Wt::WEnvironment& env);
};

class TestContainer : public Wt::WContainerWidget
{
public:
	virtual void load() override
	{
		if(!loaded())
			throw std::runtime_error("When this exception is caught, the e.what() message should be available.");
		//But Wt prints "Wt: fatal error: std::exception"
		
		Wt::WContainerWidget::load();
	}
};

HelloApplication::HelloApplication(const Wt::WEnvironment& env)
	: WApplication(env)
{
	setTitle("Exception e.what() caught from load() bug report");
	
	auto button = root()->addNew<Wt::WPushButton>("Click me (to create a widget that throws an exception on load())");
	
	button->clicked().connect([this](){
		root()->addNew<TestContainer>();
	});
}

int main(int argc, char **argv)
{
	return Wt::WRun(argc, argv, [](const Wt::WEnvironment &env) {
		return Wt::cpp14::make_unique<HelloApplication>(env);
	});
}
