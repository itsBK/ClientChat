#include <pthread.h>
#include <mqueue.h>
#include <stdbool.h>

#include "broadcastagent.hpp"
#include "util.hpp"
#include "network.hpp"
#include "server.hpp"
#include "user.hpp"

static mqd_t messageQueue;
static pthread_t threadId;
static bool threadRunning;

static void *broadcastAgent(void*)
{
	unsigned char buf[sizeof(Server2Client)];
	while(threadRunning)
	{
		if (mq_receive(messageQueue, (char*) &buf, sizeof(Server2Client), NULL) == -1)
		{
			errnoPrint("error accored while dequeuing message");
			continue;
		}

		User* user = NULL;
		while ((user = User::iterator(user)) != NULL)
		{
			if (isUserLoggedIn(user))
			{
				int status = networkSend(user->sock, (Message*) &buf);
				if (status <= 0)
					infoPrint("unknown error occured while sending data, error code: %d", status);
			}
		}
	}
	
	return NULL;
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
    int result = pthread_create(&threadId, NULL, broadcastAgent, NULL);
	if (result < 0)
	{
		errnoPrint("error while creating broadcast thread");
	}

	return result;
}

void broadcastAgentCleanup()
{
	threadRunning = false;
    pthread_join(threadId, NULL);

	int status = mq_close(messageQueue) | mq_unlink(msgQueueName);
	if (status < 0)
		errnoPrint("error while closing messageQueue");
	else
		infoPrint("messageQueue closed successfully");
}
