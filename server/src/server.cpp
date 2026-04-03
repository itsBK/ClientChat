#include "server.hpp"
#include "user.hpp"
#include "util.hpp"

#include <pthread.h>
#include <mqueue.h>

#include "network.hpp"

static mqd_t messageQueue;
static pthread_t threadId;
static bool threadRunning;

static int createPassiveSocket(in_port_t port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*) 1, sizeof(int));
	struct sockaddr_in serverSocket;
	serverSocket.sin_family = AF_INET;
	serverSocket.sin_port = htons(port);
	serverSocket.sin_addr.s_addr = INADDR_ANY;

	bind(fd, (struct sockaddr*) &serverSocket, sizeof(serverSocket));
	listen(fd, MAXIMUM_CONNECTIONS_COUNT);

	return fd;
}

int connectionHandler(in_port_t port)
{
	const int fd = createPassiveSocket(port);
	if(fd == -1)
	{
		errnoPrint("Unable to create server socket");
		return -1;
	}

	for(;;)
	{
		//TODO: accept() incoming connection
		//TODO: add connection to user list and start client thread
		int socketFd = accept(fd, nullptr, nullptr);
		User::add(socketFd);
	}

	return 0;	//never reached
}


static void *broadcastAgent(void*)
{
	unsigned char buf[sizeof(Server2Client)];
	while(threadRunning)
	{
		if (mq_receive(messageQueue, (char*) &buf, sizeof(Server2Client), nullptr) == -1)
		{
			errnoPrint("error accored while dequeuing message");
			continue;
		}

		for (auto user : UserIterator::users)
		{
			if (isUserLoggedIn(user))
			{
				int status = networkSend(user->sock, (Message*) &buf);
				if (status <= 0)
					infoPrint("unknown error occured while sending data, error code: %d", status);
			}
		}
	}

	return nullptr;
}

int broadcastAgentInit()
{
	struct mq_attr attr;
	attr.mq_maxmsg = MAXIMUM_QUEUE_SIZE;
	attr.mq_msgsize = sizeof(Server2Client);
	messageQueue = mq_open(msgQueueName, O_RDONLY | O_CREAT, 0664, &attr);
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
	pthread_join(threadId, nullptr);

	int status = mq_close(messageQueue) | mq_unlink(msgQueueName);
	if (status < 0)
		errnoPrint("error while closing messageQueue");
	else
		infoPrint("messageQueue closed successfully");
}