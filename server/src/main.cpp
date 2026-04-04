#include <iostream>
#include <csignal>
#include <cstring>

#include "cmdHelper.hpp"
#include "server.hpp"
#include "util.hpp"
#include "network.hpp"

std::string serverName;
unsigned int serverNameLength;
char* msgQueueName;
in_port_t port;


void cleanup()
{
	broadcastAgentCleanup();
	free(msgQueueName);
	close(listenSock_fd);
	infoPrint("socket closed successfully");
	infoPrint("exiting");
}

int main(int argc, char **argv)
{
	utilInit(argv[0]);
	debugEnable();
	styleEnable();
	infoPrint("Chat server v0.1");

	serverName = SERVER_DEFAULT_NAME;
	serverNameLength = serverName.length();
	port = SERVER_DEFAULT_PORT;

	cmdHelper::captureExitSignals(cleanup);
	int result = cmdHelper::parseArgs(argc, argv);
	if (result == -1)
		return EXIT_FAILURE;
	
	debugPrint("server name is '%s'", serverName.c_str());

	msgQueueName = static_cast<char*>(malloc(serverNameLength + 2));
	msgQueueName[0] = '/';
	strcpy(&msgQueueName[1], serverName.c_str());
	result = broadcastAgentInit();
	if (result < 0)
		return EXIT_FAILURE;

	debugPrint("using port %u", port);
	result = connectionHandler(port);

	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
