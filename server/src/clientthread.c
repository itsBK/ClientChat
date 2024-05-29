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
	while ((returnVal = networkReceive(self->sock, message)) != 0)
	{
		if (returnVal < 0)
			break;
		
		enum MessageType type = ((Message*) message)->type;
		if (type == LOGIN_REQUEST)
		{
			int status = processLoginRequest(self, (LoginRequest*) message);
			if (status < 0)
				break;
			
			isLoggedin = true;

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

	//check if only allowed keys are used
	//TODO: maybe replace with regex
	int nameLength = request->len - sizeof(request->version) - sizeof(request->magic);
	char name[nameLength + 1];
	strncpy(&name, request->name, nameLength);
	name[nameLength] = '\0';

	for (int i = 0; i < nameLength; i++)
	{
		char c = name[i];
		if (!(c >= 33 && c <= 126 && !(c == '\'' || c == '\"' || c == '`')))
		{
			response.code = NAME_INVALID;
			networkSend(self->sock, &response);
			debugPrint("invaild name was requested");
			return -1;
		}
	}

	//TODO: check name availability
	User* user = NULL;
	while ((user = iterator(user)) != NULL)
	{
		if (user != self && strcmp(user->name, name) == 0)
		{
			response.code = NAME_ALREADY_IN_USE;
			networkSend(self->sock, &response);
			debugPrint("invaild name was requested");
			return -1;
		}
	}

	// all is good now. save name
	self->name = malloc(nameLength);
	strcpy(self->name, name);

	response.code = SUCCESS;
	networkSend(self->sock, &response);



	UserAdded userAdded;
	userAdded.type = USER_ADDED;
	userAdded.len = htons(sizeof(userAdded.timestamp) + nameLength);
	time_t time1 = time(NULL);
	infoPrint("time normal %d, and flipped %d", time1, hton64u(time1));
	userAdded.timestamp = hton64u(time1);
	strncpy(&userAdded.name, name, nameLength);

	//TODO: notifiy this user about existing users

	user = NULL;
	while ((user = iterator(user)) != NULL)
	{
		int status = networkSend(user->sock, &userAdded);
		if (status <= 0)
			errnoPrint("unknown error occured while sending data, error code:");
	}

	return 0;
}
