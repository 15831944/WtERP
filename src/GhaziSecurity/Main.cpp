#include <iostream>
#include "Application/WServer.h"
#include "Application/WApplication.h"

int main(int argc, char *argv[])
{
	/* *************************************************************************
	* *************************  Start Web Server *****************************
	* *************************************************************************/
	GS::WServer Server(argc, argv);
	try
	{
		//Initialize Server
		Server.initialize();

		//Configuration
		Server.addEntryPoint(Wt::EntryPointType::Application, GS::WApplication::createApplication);

		//Start Server
		if(Server.start())
		{
			//And wait till a shutdown signal is given
			int sig = GS::WServer::waitForShutdown();
			Wt::log("gs-info") << "Shutdown (Signal = " << sig << ")";
			Server.stop();
		}
	}
	catch(Wt::WServer::Exception &e)
	{
		Wt::log("fatal") << "Error starting the server: " << e.what();
		return EXIT_FAILURE;
	}
	catch(std::exception &e)
	{
		Wt::log("fatal") << "Server exception error: " << e.what();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}