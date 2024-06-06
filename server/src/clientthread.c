#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>
#include "clientthread.h"
#include "util.h"

mqd_t messageQueue;

void *clientthread(void *arg)
{
	User *self = (User *)arg;
	bool isLoggedin = false;
	messageQueue = mq_open(msgQueueName, O_RDWR);
	if (messageQueue < 0)
	{
		errnoPrint("error accored while opening POSIX message queue for client thread");
		return NULL;
	}
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
				errorPrint("client cannot sent Client2Server messages before logging in");
				returnVal = -1;
				break;
			}

			processReceivedMessage(self, message);
		}
	}

	// TODO: handle kicked from server
	// only inform others if this client was logged in
	if (returnVal == 0 && isLoggedin)
		sendUserRemoved(self, CONNECTION_CLOSED);
	else if (isLoggedin)
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


void processReceivedMessage(User* sender, Client2Server* receivedMessage)
{
	Server2Client server2Client;
	server2Client.type = SERVER_2_CLIENT;
	server2Client.len = sizeof(server2Client.timestamp) + sizeof(server2Client.originalSender) + receivedMessage->len;
	server2Client.timestamp = hton64u(time(NULL));
	memset(&server2Client.originalSender, '\0', sizeof(server2Client.originalSender));

	if (receivedMessage->text[0] == '/')
	{
		processCommand(sender, receivedMessage, &server2Client);
	} else
	{
		sendMessage(sender, receivedMessage, &server2Client);
	}

}

void processCommand(User* sender, Client2Server* receivedCommand, Server2Client* responseMsg)
{
	char text[] = "you send an invalid command. don't be stupid";
	int textLen = strlen(text);
	responseMsg->len = htons(sizeof(responseMsg->timestamp) + sizeof(responseMsg->originalSender) + textLen);
	strncpy(&responseMsg->text, text, textLen);
	networkSend(sender->sock, responseMsg);
}


void sendMessage(User* sender, Client2Server* receivedMsg, Server2Client* responseMsg)
{
	strcpy(&responseMsg->originalSender, sender->name);
	strncpy(&responseMsg->text, receivedMsg->text, receivedMsg->len);
	int size = responseMsg->len + sizeof(responseMsg->type) + sizeof(responseMsg->len);
	responseMsg->len = htons(responseMsg->len);

	if (mq_send(messageQueue, responseMsg, size, 0) == -1)
		errnoPrint("error accored while queuing message");
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
