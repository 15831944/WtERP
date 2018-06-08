#include <iostream>
#include "Application/WServer.h"
#include "Application/WApplication.h"

//TODO: Create indexes
//TODO: Check exception handling
//TODO: Check for shared_ptr and Dbo::ptr cyclical circular references
//TODO: Upgrade Font Awesome

//TODO: Record deletion
//TODO: Make Location-Entity a ManyToMany relation
//TODO: Fix LOG_ERROR(Wt::Dbo::ptr) useless errors
//TODO: Look into QueryModel exception handling

//TODO: Follow Multiple Dialogs error report
//TODO: Follow MySQL backend error report
//TODO: Follow WMenuItem::setContents() error report

//TODO: Account Entry value must be > 0
//TODO: Localize DateTimes
//TODO: viewName() and viewTitle()
//TODO: viewName() for record not found
//TODO: Create abstraction for list proxy models

int main(int argc, char *argv[])
{
	/* *************************************************************************
	* *************************  Start Web Server *****************************
	* *************************************************************************/
	ERP::WServer Server(argc, argv);
	try
	{
		//Initialize Server
		Server.initialize();

		//Configuration
		Server.addEntryPoint(Wt::EntryPointType::Application, ERP::WApplication::createApplication);

		//Start Server
		if(Server.start())
		{
			//And wait till a shutdown signal is given
			int sig = ERP::WServer::waitForShutdown();
			Wt::log("erp-info") << "SHUTDOWN SIGNAL (" << sig << ")";
			Server.stop();
		}
	}
	catch(const Wt::WServer::Exception &e)
	{
		Wt::log("fatal") << "Error starting the server: " << e.what();
		return EXIT_FAILURE;
	}
	catch(const std::exception &e)
	{
		Wt::log("fatal") << "Server exception error: " << e.what();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}