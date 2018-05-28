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

HelloApplication::HelloApplication(const Wt::WEnvironment& env)
		: WApplication(env)
{
	setTitle("WMenuItem::setContents() bug report");
	
	auto stack = root()->addNew<Wt::WStackedWidget>();
	auto menu = root()->addNew<Wt::WMenu>(stack);
	menu->setInternalPathEnabled("/");
	
	auto uButton = std::make_unique<Wt::WPushButton>("First Button");
	auto button = uButton.get();
	
	auto uMenuItem = std::make_unique<Wt::WMenuItem>("First", move(uButton));
	uMenuItem->setPathComponent("");
	auto menuItem = menu->addItem(move(uMenuItem));
	menuItem->select();
	
	button->clicked().connect([menu, menuItem](){
		auto submittedItem = menuItem;
		auto submittedButton = dynamic_cast<Wt::WPushButton*>(submittedItem->contents());
		assert(submittedButton);
		
		auto uNewItem = std::make_unique<Wt::WMenuItem>("Second", submittedItem->removeContents());
		auto newItem = menu->addItem(move(uNewItem));
		
		auto uNewButton = std::make_unique<Wt::WPushButton>("Second Button");
		
		//UNCOMMENT THE LINES BELOW TO SEE WHAT SHOULD ACTUALLY HAPPEN
		//int index = menu->indexOf(submittedItem);
		//auto uSubmittedItem = menu->removeItem(submittedItem);
		submittedItem->setContents(move(uNewButton), Wt::ContentLoading::Lazy);
		//menu->insertItem(index, move(uSubmittedItem));
	});
}

int main(int argc, char **argv)
{
	return Wt::WRun(argc, argv, [](const Wt::WEnvironment &env) {
		return Wt::cpp14::make_unique<HelloApplication>(env);
	});
}
