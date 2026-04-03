#include <errno.h>
#include <netinet/in.h>
#include "network.hpp"


int receiveLoginRequest(int fd, LoginRequest* buffer)
{
	uint16_t maxLengthAllowed = sizeof(buffer->magic) + sizeof(buffer->version) + NAME_MAX_LENGTH;
	if (buffer->len > maxLengthAllowed)
	{
		errorPrint("message length(%u) reached maximum(%u) allowed", buffer->len, maxLengthAllowed);
		return -1;
	}
	
	ssize_t bytesReceived = recv(fd, &buffer->magic, buffer->len, 0);
	if (bytesReceived < 0)
	{
		errnoPrint("error occured while receiving login request");
	}
	
	buffer->magic = ntohl(buffer->magic);
	if (buffer->magic != LOGIN_REQUEST_MAGIC_VALUE)
	{
		errorPrint("wrong magic number sent. closing thread");
		return -1;
	}

	return bytesReceived;
}

int receiveClient2Server(int fd, Client2Server* buffer)
{
	if (buffer->len > TEXT_MAX_LENGTH)
		return -1;
	
	ssize_t bytesReceived = recv(fd, &buffer->text, buffer->len, 0);
	if (bytesReceived < 0)
	{
		errnoPrint("error occured while receiving (Client2Server) message");
	}
	return bytesReceived;
}


int networkReceive(int fd, Message *buffer)
{
	ssize_t bytesReceived = recv(fd, &buffer->type, sizeof(buffer->type) + sizeof(buffer->len), 0);

	if (bytesReceived == 0)
	{
		return bytesReceived;
	}

	else if (bytesReceived < 0)
	{
		errnoPrint("error occurred while receiving message");
		return bytesReceived;
	}

	auto type = static_cast<MessageType>(buffer->type);
	buffer->len = ntohs(buffer->len);
	switch (type)
	{
		case LOGIN_REQUEST:
			return receiveLoginRequest(fd, (LoginRequest*) buffer);
		case CLIENT_2_SERVER:
			return receiveClient2Server(fd, (Client2Server*) buffer);

		case SERVER_2_CLIENT:
		case USER_ADDED:
		case USER_REMOVED:
		case LOGIN_RESPONSE:
			errorPrint("client is not supposed to send this message type (%u)", buffer->type);
			return -1;
		default:
			errorPrint("wrong message type (%u) received", buffer->type);
			return -1;
	}
}

int networkSend(int fd, const Message *buffer)
{
	ssize_t status = send(fd, &buffer->type, sizeof(buffer->type) + sizeof(buffer->len), 0);
	if (status < 0)
	{
		errnoPrint("1111 error happened here");
		return status;
	}

	status = send(fd, &buffer->data, ntohs(buffer->len), 0);
	if (status < 0)
		errnoPrint("2222 error happened here");

	return status;
}
