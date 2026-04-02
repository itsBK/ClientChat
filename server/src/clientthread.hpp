#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "server.hpp"
#include "user.hpp"
#include "network.hpp"

/**
 * returns:
 * * -1: if login process has failed
 * *  0: if login process was successful
 * the function sends a LoginResponse regardless of the return value
*/
int processLoginRequest(User* self, LoginRequest* request);
void processReceivedMessage(User* sender, Client2Server* receivedMessage);
void processCommand(User* sender, Client2Server* receivedCommand, Server2Client* responseMsg);
void sendMessage(const User* sender, const Client2Server* receivedMsg, Server2Client* responseMsg);

void sendUserAdded(User* newUser);
void sendUserRemoved(User* removedUser, enum UserRemovedCode code);

void *clientthread(void *arg);

