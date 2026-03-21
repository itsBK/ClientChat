#include <errno.h>
#include "connectionhandler.hpp"
#include "user.hpp"
#include "util.hpp"

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
		int socketFd = accept(fd, NULL, NULL);
		User::add(socketFd);
	}

	return 0;	//never reached
}
