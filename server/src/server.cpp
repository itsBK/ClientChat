#include <cstring>
#include <pthread.h>
#include <mqueue.h>

#include "server.hpp"
#include "user.hpp"
#include "util.hpp"

#include "network.hpp"

static mqd_t messageQueue;
static pthread_t threadId;
bool threadRunning;
int listenSock_fd;

static int createPassiveSocket(in_port_t port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*) 1, sizeof(int));
	sockaddr_in serverSocket;
	serverSocket.sin_family = AF_INET;
	serverSocket.sin_port = htons(port);
	serverSocket.sin_addr.s_addr = INADDR_ANY;

	bind(fd, (sockaddr*) &serverSocket, sizeof(serverSocket));
	listen(fd, MAXIMUM_CONNECTIONS_COUNT);

	return fd;
}

int connectionHandler(in_port_t port)
{
	listenSock_fd = createPassiveSocket(port);
	if(listenSock_fd == -1)
	{
		errnoPrint("Unable to create server socket");
		return -1;
	}

	while (threadRunning)
	{
		int socketFd = accept(listenSock_fd, nullptr, nullptr);
		if (socketFd != -1)
			User::add(socketFd);
	}

	return 0;
}


static void *broadcastAgent(void*)
{
	unsigned char buf[sizeof(Server2Client)];
	while(true)
	{
		ssize_t bytesReceived = mq_receive(messageQueue, (char*) &buf, MESSAGE_MAX_LENGTH, nullptr);
		if (bytesReceived < 0)
		{
			errnoPrint("error occurred while dequeuing message");
			continue;
		}

		if (strcmp(reinterpret_cast<const char*>(&buf), MESSAGE_QUEUE_CLOSE_COMMAND) == 0)
			break;

		for (auto user : UserIterator::users)
		{
			if (isUserLoggedIn(user))
			{
				int status = networkSend(user->sock, (Message*) &buf);
				if (status <= 0)
					infoPrint("unknown error occurred while sending data, error code: %d", status);
			}
		}
	}

	return nullptr;
}

int broadcastAgentInit()
{
	mq_attr attr;
	attr.mq_maxmsg = MAXIMUM_QUEUE_SIZE;
	attr.mq_msgsize = sizeof(Server2Client);
	messageQueue = mq_open(msgQueueName, O_RDWR | O_CREAT, 0644, &attr);
	if (messageQueue < 0)
	{
		errnoPrint("error accored while creating POSIX message queue");
		return -1;
	}

	debugPrint("opening POSIX Message Queue named '%s'", msgQueueName);
	debugPrint("maximum queued messages count is %ld, maximum buffer size per msg is %ld bytes", attr.mq_maxmsg, attr.mq_msgsize);

	threadRunning = true;
	int result = pthread_create(&threadId, nullptr, broadcastAgent, nullptr);
	if (result < 0)
	{
		errnoPrint("error while creating broadcast thread");
	}

	return result;
}

void broadcastAgentCleanup()
{
	threadRunning = false;
	auto value = mq_send(messageQueue, MESSAGE_QUEUE_CLOSE_COMMAND, strlen(MESSAGE_QUEUE_CLOSE_COMMAND) + 1, 0);
	if (value < 0)
		errnoPrint("command to close the message queue could not be sent successfully");
	pthread_join(threadId, nullptr);

	int status = mq_close(messageQueue) | mq_unlink(msgQueueName);
	if (status < 0)
		errnoPrint("error while closing messageQueue");
	else
		infoPrint("messageQueue closed successfully");
}