#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WDialog.h>

using namespace Wt;

class BugReportApp : public WApplication
{
public:
	BugReportApp(const WEnvironment& env);
};

BugReportApp::BugReportApp(const WEnvironment& env)
		: WApplication(env)
{
	setTitle("BUG REPORT");

	//DIALOG//
	WDialog *dialog1 = addChild(std::make_unique<Wt::WDialog>());
	dialog1->contents()->addNew<Wt::WText>("DIALOG ONE");
	dialog1->setTitleBarEnabled(true);
	dialog1->setTransient(true);
	dialog1->setClosable(true);
	dialog1->show();

	WDialog *dialog2 = addChild(std::make_unique<Wt::WDialog>());
	dialog2->contents()->addNew<Wt::WText>("DIALOG TWO");
	dialog2->setTitleBarEnabled(true);
	dialog2->setTransient(true);
	dialog2->setClosable(true);
	dialog2->show();
}

int main(int argc, char **argv)
{
	return WRun(argc, argv, [](const WEnvironment& env)->auto{ return std::make_unique<BugReportApp>(env); });
}

