#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <netinet/in.h>
#include "user.h"

#define MAXIMUM_CONNECTIONS_COUNT 10

int connectionHandler(in_port_t port);

#endif
