#include <stdio.h>
#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"

void *clientthread(void *arg)
{
	User *self = (User *)arg;

	debugPrint("Client thread started.");

	//TODO: Receive messages and send them to all users, skip self
	Message message = {};
	int returnVal;
	while ((returnVal = networkReceive(self->sock, &message)) != 0)
	{
		if (returnVal < 0)
			errorPrint("i have no idea, error code is %d, %u", returnVal, self->thread);

		User* user = NULL;
		while ((user = iterator(user)) != NULL)
		{
			if (user != self)
			{
				int status = networkSend(user->sock, &message);
				if (status <= 0)
					infoPrint("unknown error accored while sending data, error code: %d", status);
			}
		}
	}

	debugPrint("Client thread stopping.");
	removeUser(self);
	
	return NULL;
}
