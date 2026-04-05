#include "cmdHelper.hpp"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "network.hpp"
#include "server.hpp"
#include "util.hpp"

void(*cmdHelper::cleanupCallback)() = nullptr;

int cmdHelper::parseArgs(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            infoPrint("Usage: %s [-h] [-n SERVERNAME] [-p PORT]", getProgName());
            infoPrint("\t-h\t\tprint this usage");
            infoPrint("\t-n SERVERNAME\tset server name (default: %s)", Server::DEFAULT_NAME);
            infoPrint("\t-p PORT\t\tset TCP port (default: %u)", Server::DEFAULT_PORT);
            return 0;

        }
        if (strcmp(argv[i], "-n") == 0 && i+1 < argc)
        {
            Server::instance.serverName = argv[++i];
            if (Server::instance.serverName.length() > NAME_MAX_LENGTH)
            {
                errorPrint("server name is too long, maximum length allowed is %d", NAME_MAX_LENGTH);
                return -1;
            }

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
                    return -1;
                }
            }

            Server::instance.port = atoi(givenPort);
        }
    }

    return 0;
}

void cmdHelper::captureExitSignals(void(*cleanup)())
{
    signal(SIGINT, captureSignal);		// Ctrl + C
    signal(SIGTERM, captureSignal);		// polite killing signal
    signal(SIGTSTP, captureSignal);		// Ctrl + Z

    cleanupCallback = cleanup;
}

void cmdHelper::captureSignal(int sig)
{
    std::cout << std::endl;
    if (sig == SIGINT)
        infoPrint("we are being interrupted (Ctrl+C), where are your manners ...");
    else if (sig == SIGTERM)
        infoPrint("we are being politely asked to die. committing Seppuku (honorable death) ?_?");
    else if (sig == SIGTSTP)
        infoPrint("received a shush (Ctrl+Z), but we cannot be shushed. Dying in disappointment ...");

    if (cleanupCallback)
        cleanupCallback();
}
