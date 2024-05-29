#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <stdlib.h>
#include <stdbool.h>
#include "server.h"
#include "user.h"
#include "network.h"

/**
 * returns:
 * * -1: if login process has failed
 * *  0: if login process was successful
 * the function sends a LoginResponse regardless of the return value
*/
int processLoginRequest(User* self, LoginRequest* request);

void *clientthread(void *arg);

#endif
