#include <errno.h>
#include "network.h"

int networkReceive(int fd, Message *buffer)
{
	//TODO: Receive length
	//TODO: Convert length byte order
	//TODO: Validate length
	//TODO: Receive text

	errno = ENOSYS;
	return -1;
}

int networkSend(int fd, const Message *buffer)
{
	//TODO: Send complete message

	errno = ENOSYS;
	return -1;
}
