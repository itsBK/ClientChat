#pragma once

#include <netinet/in.h>
#include <string>

static const std::string SERVER_DEFAULT_NAME = "reference-server";
#define SERVER_DEFAULT_PORT 8111
#define MAXIMUM_CONNECTIONS_COUNT 20

extern std::string serverName;
extern unsigned int serverNameLength;
extern char* msgQueueName;

int connectionHandler(in_port_t port);
