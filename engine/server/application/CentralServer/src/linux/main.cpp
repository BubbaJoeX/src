#include <signal.h>

#include "sharedFoundation/FirstSharedFoundation.h"

#include "ConfigCentralServer.h"
#include "CentralServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#ifdef ENABLE_PROFILING
extern "C" int __llvm_profile_write_file(void);
#endif

inline void signalHandler(int s){
    printf("PlanetServer terminating, signal %d\n",s);

#ifdef ENABLE_PROFILING
    __llvm_profile_write_file();
#endif

    exit(0);
}

// ======================================================================

int main(int argc, char ** argv)
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine  = ConvertCommandLine(argc,argv);
	SetupSharedFoundation::install(setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedRandom::install(time(0)); //lint !e732

	NetworkHandler::install();
	Os::setProgramName("CentralServer");
	ConfigCentralServer::install();

	std::string cmdLine = setupFoundationData.lpCmdLine;
	size_t firstArg = cmdLine.find(" ", 0);
	size_t lastSlash = 0;
	size_t nextSlash = 0;
	while(nextSlash < firstArg)
	{
		nextSlash = cmdLine.find("/", lastSlash);
		if(nextSlash == cmdLine.npos || nextSlash >= firstArg) //lint !e1705 static class members may be accessed by the scoping operator (huh?)
			break;
		lastSlash = nextSlash + 1;
	}
	cmdLine = cmdLine.substr(lastSlash);
	CentralServer::getInstance().setCommandLine(cmdLine);
	//-- run game
	SetupSharedFoundation::callbackWithExceptionHandling(CentralServer::run);
	NetworkHandler::remove();
	SetupSharedFoundation::remove();

#ifdef ENABLE_PROFILING
	__llvm_profile_write_file();
	exit(0);
#endif	

	return 0;
}
