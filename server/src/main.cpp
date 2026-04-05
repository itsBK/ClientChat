#include <iostream>
#include <csignal>
#include <cstring>

#include "cmdHelper.hpp"
#include "server.hpp"
#include "util.hpp"
#include "network.hpp"

void cleanup()
{
	Server::instance.broadcastAgentCleanup();
	free(Server::instance.msgQueueName);
	close(Server::instance.listenSock_fd);
	infoPrint("socket closed successfully");
	infoPrint("exiting");
}

int main(int argc, char **argv)
{
	utilInit(argv[0]);
	debugEnable();
	styleEnable();
	infoPrint("Chat server v0.1");

	cmdHelper::captureExitSignals(cleanup);
	int result = cmdHelper::parseArgs(argc, argv);
	if (result == -1)
		return EXIT_FAILURE;
	
	debugPrint("server name is '%s'", Server::instance.serverName.c_str());

	Server::instance.msgQueueName = static_cast<char*>(malloc(Server::instance.serverName.length() + 2));
	Server::instance.msgQueueName[0] = '/';
	strcpy(&Server::instance.msgQueueName[1], Server::instance.serverName.c_str());
	result = Server::instance.broadcastAgentInit();
	if (result < 0)
		return EXIT_FAILURE;

	debugPrint("using port %u", Server::instance.port);
	result = Server::instance.connectionHandler(Server::instance.port);

	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
