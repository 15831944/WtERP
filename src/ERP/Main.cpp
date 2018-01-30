#include <iostream>
#include "Application/WServer.h"
#include "Application/WApplication.h"

//TODO: Exception handling
//TODO: Record deletion
//TODO: Find solution to Dbo::ptr circular reference
//TODO: Check for shared_ptr circular references
//TODO: Localize DateTimes
//TODO: Modernize code
//TODO: ViewName() and ViewTitle()
//TODO: Make Location-Entity a ManyToMany relation

//TODO: Follow Multiple Dialogs error report

//TODO: A better way to create indexes on Dbos (create index on AttendaceDeviceV and all record version dbos)
//TODO: Read only view and reloadable RecordFormView
//TODO: Improve RecordFormModel API (and remove unnecessary pointers)
//TODO: Create abstraction for list proxy models
//TODO: Fix LOG_ERROR(Wt::Dbo::ptr) useless errors
//TODO: Remove Employee and Personnel DBOs
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