#include <signal.h>

#include "FirstConnectionServer.h"
#include "ConfigConnectionServer.h"
#include "ConnectionServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#ifdef ENABLE_PROFILING
extern "C" int __llvm_profile_write_file(void);
#endif

inline void signalHandler(int s){
    printf("ConnectionServer terminating, signal %d\n",s);

#ifdef ENABLE_PROFILING
    __llvm_profile_write_file();
#endif

    exit(0);
}

// ======================================================================

void dumpPid(const char * argv)
{
	pid_t p = getpid();
	char fileName[1024];
	sprintf(fileName, "%s.%d", argv, p);
	FILE * f = fopen(fileName, "w+");
	fclose(f);
}

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
	setupFoundationData.lpCmdLine                   = ConvertCommandLine(argc,argv);
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();
	SetupSharedRandom::install(time(nullptr));

	//setup the server
	NetworkHandler::install();
	Os::setProgramName("ConnectionServer");
	ConfigConnectionServer::install();
        
	//-- run game
	ConnectionServer::install();
	SetupSharedFoundation::callbackWithExceptionHandling(ConnectionServer::run);
	ConnectionServer::remove();
  
	ConfigConnectionServer::remove();
	NetworkHandler::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

#ifdef ENABLE_PROFILING
    __llvm_profile_write_file();
#endif
	
	return 0;
}
