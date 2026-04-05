#include "application.hpp"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "network.hpp"
#include "server.hpp"
#include "util.hpp"


int Application::parseArgs(int argc, char **argv)
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

void Application::captureExitSignals()
{
    signal(SIGINT, captureSignal);		// Ctrl + C
    signal(SIGTERM, captureSignal);		// polite killing signal
    signal(SIGTSTP, captureSignal);		// Ctrl + Z
}

void Application::captureSignal(int sig)
{
    std::cout << std::endl;
    if (sig == SIGINT)
        infoPrint("we are being interrupted (Ctrl+C), where are your manners ...");
    else if (sig == SIGTERM)
        infoPrint("we are being politely asked to die. committing Seppuku (honorable death) ?_?");
    else if (sig == SIGTSTP)
        infoPrint("received a shush (Ctrl+Z), but we cannot be shushed. Dying in disappointment ...");

    cleanup();
}



void Application::cleanup()
{
    Server::instance.cleanup();
    infoPrint("socket closed successfully");
    infoPrint("exiting");
}

int main(int argc, char **argv)
{
    utilInit(argv[0]);
    debugEnable();
    styleEnable();
    infoPrint("Chat server v0.1");

    Application::captureExitSignals();
    int result = Application::parseArgs(argc, argv);
    if (result == -1)
        return EXIT_FAILURE;

    debugPrint("server name is '%s'", Server::instance.serverName.c_str());

    Server::instance.msgQueueName = static_cast<char*>(malloc(Server::instance.serverName.length() + 2));
    Server::instance.msgQueueName[0] = '/';
    strcpy(&Server::instance.msgQueueName[1], Server::instance.serverName.c_str());
    result = Server::instance.broadcastAgentInit();
    if (result < 0)
        return EXIT_FAILURE;

    result = Server::instance.connectionHandler();

    return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

