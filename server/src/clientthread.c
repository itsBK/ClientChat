#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "clientthread.h"
#include "util.h"

void *clientthread(void *arg)
{
	User *self = (User *)arg;
	bool isLoggedin = false;

	debugPrint("Client thread started.");

	//TODO: Receive messages and send them to all users, skip self
	void* message = malloc(1024);
	int returnVal;
	while ((returnVal = networkReceive(self->sock, message)) > 0)
	{
		enum MessageType type = ((Message*) message)->type;
		if (type == LOGIN_REQUEST)
		{
			int status = processLoginRequest(self, (LoginRequest*) message);
			if (status < 0)
				break;
			
			isLoggedin = true;
			sendUserAdded(self);

		} else if (type == CLIENT_2_SERVER)
		{
			if (!isLoggedin)
			{
				errorPrint("client is not logged in yet");
				break;
			}

			infoPrint("received client to server message");
		}


		continue;
		User* user = NULL;
		while ((user = iterator(user)) != NULL)
		{
			if (user != self)
			{
				int status = networkSend(user->sock, &message);
				if (status <= 0)
					infoPrint("unknown error occured while sending data, error code: %d", status);
			}
		}
	}

	// TODO: handle kicked from server
	if (returnVal == 0)
		sendUserRemoved(self, CONNECTION_CLOSED);
	else
		sendUserRemoved(self, COMMUNICATION_ERROR);

	debugPrint("Client thread stopping.");
	free(message);
	removeUser(self);
	
	return NULL;
}


int processLoginRequest(User* self, LoginRequest* request)
{
	LoginResponse response;
	response.type = LOGIN_RESPONSE;
	response.len = htons(sizeof(response.magic) + sizeof(response.code) + serverNameLength);
	response.magic = htonl(LOGIN_RESPONSE_MAGIC_VALUE);
	strncpy(&response.sName, serverName, serverNameLength);

	if (request->version != LOGIN_REQUEST_PROTOCOL_VERSION)
	{
		response.code = PROTOCOL_VERSION_MISMATCH;
		networkSend(self->sock, &response);
		return -1;
	}

	int nameLength = request->len - sizeof(request->version) - sizeof(request->magic);
	char name[nameLength + 1];
	strncpy(&name, request->name, nameLength);
	name[nameLength] = '\0';
	
	response.code = checkAndProcessName(self, name);
	networkSend(self->sock, &response);

	if (response.code != SUCCESS)
	{
		debugPrint("invaild name was requested");
		return -1;
	}

	return 0;
}


void sendUserAdded(User* newUser)
{
	int newUserNameLength = strlen(newUser->name);
	UserAdded userAdded;
	userAdded.type = USER_ADDED;
	userAdded.len = htons(sizeof(userAdded.timestamp) + newUserNameLength);
	userAdded.timestamp = hton64u(time(NULL));
	strncpy(&userAdded.name, newUser->name, newUserNameLength);

	User* user = NULL;
	while ((user = iterator(user)) != NULL)
	{
		if (user != newUser)
		{
			int existingUserNameLength = strlen(user->name);
			UserAdded existingUser;
			existingUser.type = USER_ADDED;
			existingUser.len = htons(sizeof(existingUser.timestamp) + existingUserNameLength);
			existingUser.timestamp = 0;
			strncpy(&existingUser.name, user->name, existingUserNameLength);
			networkSend(newUser->sock, &existingUser);
		}
		networkSend(user->sock, &userAdded);
	}
}


void sendUserRemoved(User* removedUser, enum UserRemovedCode code)
{
	int nameLength = strlen(removedUser->name);
	UserRemoved userRemoved;
	userRemoved.type = USER_REMOVED;
	userRemoved.len = htons(sizeof(userRemoved.timestamp) + sizeof(userRemoved.code) + nameLength);
	userRemoved.timestamp = hton64u(time(NULL));
	userRemoved.code = code;
	strncpy(&userRemoved.name, removedUser->name, nameLength);

	User* user = NULL;
	while ((user = iterator(user)) != NULL)
	{
		if (user != removedUser)
			networkSend(user->sock, &userRemoved);
	}
}
