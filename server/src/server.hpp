#ifndef SERVER_H
#define SERVER_H

#include <string>

static const std::string SERVER_DEFAULT_NAME = "reference-server";
#define SERVER_DEFAULT_PORT 8111

extern std::string serverName;
extern unsigned int serverNameLength;
extern char* msgQueueName;

#endif