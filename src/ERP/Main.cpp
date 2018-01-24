#include <iostream>
#include "Application/WServer.h"
#include "Application/WApplication.h"

//TODO: Exception handling
//TODO: Record deletion
//TODO: Versioned record keeping
//TODO: Find solution to Dbo::ptr circular reference
//TODO: Check for shared_ptr circular references
//TODO: Localize DateTimes
//TODO: Separate FormModel and FormView roles properly

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
			Wt::log("erp-info") << "Shutdown (Signal = " << sig << ")";
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