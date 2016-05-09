#include <Wt/WApplication>
#include <Wt/WPushButton>
#include <Wt/WTimeEdit>

using namespace Wt;

class HelloApplication : public WApplication
{
public:
	HelloApplication(const WEnvironment& env);
};

WApplication *createApplication(const WEnvironment& env) { return new HelloApplication(env); }
int main(int argc, char **argv)
{
	return WRun(argc, argv, &createApplication);
}

//MAIN LOGIC
HelloApplication::HelloApplication(const WEnvironment& env)
  : WApplication(env)
{
	setTitle("doJavascript and setJavascriptMember output order bug");

	auto btn = new WPushButton("Push", root());
	btn->clicked().connect(std::bind([this]() {
		auto timeEdit = new WTimeEdit(); //initialized but not in widget hierarchy
		refresh(); //also calls timeEdit->refresh() which calls doJavascript() before setJavaScriptMember() is called
		root()->addWidget(timeEdit); //Now setJavaScriptMember() is called
	}));

	//The output shows Obj(timePickerSpinBox->jsRef()).setLocale(...) is called before the Obj() is defined
}
