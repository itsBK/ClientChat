#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <netinet/in.h>

#define MAXIMUM_CONNECTIONS_COUNT 20

int connectionHandler(in_port_t port);

#endif
