#include <iostream>
#include "Application/WServer.h"
#include "Application/WApplication.h"

//TODO: Create indexes

//TODO: Exception handling
//TODO: Record deletion
//TODO: Check for shared_ptr and Dbo::ptr circular circular references
//TODO: Localize DateTimes
//TODO: Modernize code
//TODO: Make Location-Entity a ManyToMany relation
//TODO: Fix LOG_ERROR(Wt::Dbo::ptr) useless errors
//TODO: Upgrade Font Awesome
//TODO: Look into QueryModel exception handling

//TODO: Follow Multiple Dialogs error report

//TODO: viewName() and viewTitle()
//TODO: viewName() for record not found
//TODO: delete menu items with same internal path on submit handler
//TODO: WApplication::initDboQueryModel exception safety
//TODO: Improve RecordFormModel API (and remove unnecessary pointers)
//TODO: Create abstraction for list proxy models
//TODO: Versioned record keeping for all Record Dbos

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