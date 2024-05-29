#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "connectionhandler.h"
#include "util.h"
#include "network.h"

int main(int argc, char **argv)
{
	utilInit(argv[0]);
	debugEnable();
	styleEnable();
	infoPrint("Chat server, group xy");	//TODO: Add your group number!

	serverName = malloc(32);
	serverName = SERVER_DEFAULT_NAME;
	serverNameLength = strlen(serverName);
	in_port_t port = SERVER_DEFAULT_PORT;

	//TODO: evaluate command line arguments
	//TODO: perform initialization


	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			infoPrint("Usage: %s [-h] [-n SERVERNAME] [-p PORT]", getProgName());
			infoPrint("\t-h\t\tprint this usage");
			infoPrint("\t-n SERVERNAME\tset server name (default: %s)", SERVER_DEFAULT_NAME);
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
			int j = 0;
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
	
	debugPrint("server name is '%s'", serverName);
	debugPrint("using port %u", port);
	const int result = connectionHandler(port);
	free(serverName);

	//TODO: perform cleanup, if required by your implementation
	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
