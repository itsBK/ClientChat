#include <iostream>
#include <csignal>
#include <cstring>
#include "server.hpp"
#include "util.hpp"
#include "network.hpp"

std::string serverName;
unsigned int serverNameLength;
char* msgQueueName;

void captureSignal(int sig)
{
	std::cout << std::endl;
	if (sig == SIGINT)
		infoPrint("we are being interrupted (Ctrl+C), where is your manners ...");
	else if (sig == SIGTERM)
		infoPrint("we are being politely asked to die. committing Seppuku (honorable death) ?_?");
	else if (sig == SIGTSTP)
		infoPrint("received a shush (Ctrl+Z), but we cannot be shushed. Dying in disappointment ...");
	broadcastAgentCleanup();
	close(listenSock_fd);
	infoPrint("socket closed successfully");
	std::cout << "exiting" << std::endl;
	exit(EXIT_SUCCESS);
}

void setExitSignal()
{
	signal(SIGINT, captureSignal);		// Ctrl + C
	signal(SIGTERM, captureSignal);		// polite killing signal
	signal(SIGTSTP, captureSignal);		// Ctrl + Z
}

int main(int argc, char **argv)
{
	utilInit(argv[0]);
	debugEnable();
	styleEnable();
	infoPrint("Chat server v0.1");
	setExitSignal();

	serverName = SERVER_DEFAULT_NAME;
	serverNameLength = serverName.length();
	in_port_t port = SERVER_DEFAULT_PORT;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			infoPrint("Usage: %s [-h] [-n SERVERNAME] [-p PORT]", getProgName());
			infoPrint("\t-h\t\tprint this usage");
			infoPrint("\t-n SERVERNAME\tset server name (default: %s)", SERVER_DEFAULT_NAME.c_str());
			infoPrint("\t-p PORT\t\tset TCP port (default: %u)", SERVER_DEFAULT_PORT);
			return EXIT_SUCCESS;

		} else if (strcmp(argv[i], "-n") == 0 && i+1 < argc)
		{
			serverNameLength = strlen(argv[++i]);
			if (serverNameLength > NAME_MAX_LENGTH)
			{
				errorPrint("server name is too long, maximum length allowed is %d", NAME_MAX_LENGTH);
				return EXIT_FAILURE;
			}
			serverName = argv[i];

		} else if (strcmp(argv[i], "-p") == 0 && i+1 < argc)
		{

			char* givenPort = argv[++i];
			unsigned int j = 0;
			while (j < strlen(givenPort))
			{
				if (isdigit(givenPort[j]) != 0)
					j++;
				else
				{
					errorPrint("port number must be all digits and a positive number");
					return EXIT_FAILURE;
				}
			}

			port = atoi(givenPort);

		}
	}
	
	debugPrint("server name is '%s'", serverName.c_str());

	msgQueueName = reinterpret_cast<char*>(malloc(serverNameLength + 2));
	msgQueueName[0] = '/';
	strcpy(&msgQueueName[1], serverName.c_str());
	int result = broadcastAgentInit();
	if (result < 0)
		return EXIT_FAILURE;

	debugPrint("using port %u", port);
	result = connectionHandler(port);

	broadcastAgentCleanup();
	free(msgQueueName);

	//TODO: perform cleanup, if required by your implementation
	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
