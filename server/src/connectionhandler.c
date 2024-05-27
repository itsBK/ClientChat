#include <errno.h>
#include "connectionhandler.h"
#include "util.h"

static int createPassiveSocket(in_port_t port)
{
	int fd = -1;
	//TODO: socket()
	//TODO: bind() to port
	//TODO: listen()

	errno = ENOSYS;
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
	}

	return 0;	//never reached
}
