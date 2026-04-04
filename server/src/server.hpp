#pragma once

#include <netinet/in.h>
#include <string>

static const std::string SERVER_DEFAULT_NAME = "reference-server";
#define SERVER_DEFAULT_PORT 8111
#define MAXIMUM_CONNECTIONS_COUNT 20
#define MAXIMUM_QUEUE_SIZE 10

#define MESSAGE_QUEUE_CLOSE_COMMAND "EXIT_QUEUE"


extern std::string serverName;
extern unsigned int serverNameLength;
extern char* msgQueueName;
extern in_port_t port;
extern bool threadRunning;
extern int listenSock_fd;

int connectionHandler(in_port_t port);
int broadcastAgentInit();
void broadcastAgentCleanup();
