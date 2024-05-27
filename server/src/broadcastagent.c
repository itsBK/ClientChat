#include <pthread.h>
#include <mqueue.h>
#include "broadcastagent.h"
#include "util.h"

static mqd_t messageQueue;
static pthread_t threadId;

static void *broadcastAgent(void *arg)
{
	//TODO: Implement thread function for the broadcast agent here!
	return arg;
}

int broadcastAgentInit(void)
{
	//TODO: create message queue
	//TODO: start thread
	return -1;
}

void broadcastAgentCleanup(void)
{
	//TODO: stop thread
	//TODO: destroy message queue
}
