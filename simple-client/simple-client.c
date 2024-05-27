#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "message.h"

typedef union __attribute__((packed))
{
	Message asMessage;
	char asChars[sizeof(Message)];
} MessageBuffer;

static int connectTo(const char *hostname, const char *portstr)
{
	/* prepare hints for DNS name resolution via getaddrinfo */
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_NUMERICSERV;	//port is given as a number, don't perform service resolution
	hints.ai_family = AF_INET;		//only return IPv4 addresses
	hints.ai_socktype = SOCK_STREAM;	//only stream sockets
	hints.ai_protocol = 0;			//same as the protocol parameter of socket (0=default)

	/* resolve hostname into IP addresses */
	struct addrinfo *gai_res;
	const int gai_status = getaddrinfo(hostname, portstr, &hints, &gai_res);
	if(gai_status != 0)
	{
		fprintf(stderr, "Could not resolve hostname: %s\n", gai_strerror(gai_status));
		return -1;
	}

	/* getaddrinfo might return more than one candidate. Try them one by one,
	 * until we successfully establish a connection.
	 */
	int sock = -1;
	for(const struct addrinfo *it=gai_res; it!=NULL && sock==-1; it=it->ai_next)
	{
		sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
		if(sock >= 0)
		{
			if(connect(sock, it->ai_addr, it->ai_addrlen) != -1)
				fprintf(stderr, "Connected.\n");
			else
			{
				close(sock);
				sock = -1;
			}
		}
	}
	freeaddrinfo(gai_res);

	if(sock == -1)
		fprintf(stderr, "Cannot connect to %s:%s!\n", hostname, portstr);

	return sock;
}

static int mainLoop(int sock)
{
	MessageBuffer buffer;
	char *bufferPos = buffer.asChars;

	struct pollfd fds[2] = {
		{
			.fd = STDIN_FILENO,
			.events = POLLIN | POLLERR | POLLHUP,
			.revents = 0
		},
		{
			.fd = sock,
			.events = POLLIN | POLLERR | POLLHUP,
			.revents = 0
		}
	};

	for(;;)
	{
		if(poll(fds, sizeof(fds)/sizeof(struct pollfd), -1) == -1)
		{
			perror("Error in poll");
			return -1;
		}

		if(fds[0].revents)	//activity on stdin
		{
			Message msg;
			const ssize_t bytesRead = read(STDIN_FILENO, msg.message, sizeof(msg.message));
			if(bytesRead < 0)
			{
				perror("Error reading stdin");
				return -1;
			}
			else if(bytesRead == 0)
			{
				fputs("Quit.\n", stderr);
				break;
			}

			msg.len = htons(bytesRead);
			if(send(sock, &msg, sizeof(msg.len) + (size_t)bytesRead, 0) <
					(ssize_t)sizeof(msg.len) + bytesRead)
			{
				perror("Error sending data via socket");
				return -1;
			}
		}

		if(fds[1].revents)	//activity on socket
		{
			size_t toRead;
			if(bufferPos < buffer.asMessage.message)	//still on length field
				toRead = buffer.asMessage.message - bufferPos;
			else						//on the message text
			{
				toRead = ntohs(buffer.asMessage.len) - (bufferPos - buffer.asMessage.message);
				if(toRead > MSG_MAX)
				{
					fprintf(stderr, "Message too big!\n");
					return -1;
				}
				if(toRead == 0)
					continue;
			}

			const ssize_t bytesRead = recv(sock, bufferPos, toRead, MSG_DONTWAIT);
			if(bytesRead == -1)
			{
				if(errno != EAGAIN && errno != EWOULDBLOCK)
				{
					perror("Error receiving data via socket");
					return -1;
				}
			}
			else if(bytesRead == 0)
			{
				fputs("Connection closed.\n", stderr);
				break;
			}
			else if(bufferPos >= buffer.asMessage.message &&
					bufferPos + bytesRead == buffer.asMessage.message + ntohs(buffer.asMessage.len))
			{
				//full message has been read, print it
				if(write(STDOUT_FILENO, buffer.asMessage.message, ntohs(buffer.asMessage.len)) <
						ntohs(buffer.asMessage.len))
				{
					perror("Error printing the message");
					return -1;
				}
				bufferPos = buffer.asChars;
			}
			else
				bufferPos += bytesRead;
		}
	}

	return 0;
}

static void usage(const char *argv0)
{
	fprintf(stderr, "usage: %s [HOSTNAME [PORT]]\n", argv0);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	const char *host = "localhost";
	const char *port = "8111";

	if(argc == 2 || argc == 3)
	{
		if(!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
			usage(argv[0]);

		host = argv[1];
		if(argc == 3)
			port = argv[2];
	}
	else if(argc > 3)
		usage(argv[0]);

	const int sock = connectTo(host, port);
	if(sock == -1)
		return EXIT_FAILURE;

	const int exitStatus = (mainLoop(sock) == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
	close(sock);
	return exitStatus;
}
