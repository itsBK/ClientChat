#include <errno.h>
#include "network.h"

int networkReceive(int fd, Message *buffer)
{
	//TODO: Receive length
	ssize_t bytesReceived = recv(fd, &buffer->len, sizeof(buffer->len), 0);

	if (bytesReceived == 0)
	{
		return bytesReceived;
	}

	else if (bytesReceived < 0)
	{
		errorPrint("111 error happening here mate %d, error code: %d", bytesReceived, errno);
		return bytesReceived;
	}

	//TODO: Convert length byte order
	buffer->len = ntohs(buffer->len);
	
	//TODO: Validate length
	if (buffer->len > MSG_MAX)
		return -1;
	
	//TODO: Receive text
	bytesReceived = recv(fd, &buffer->text, buffer->len, 0);
	if (bytesReceived < 0)
	{
		errorPrint("222 error happening here mate %d, error code: %d", bytesReceived, errno);
	}

	return bytesReceived;
}

int networkSend(int fd, const Message *buffer)
{
	//TODO: Send complete message

	infoPrint("using socket number %d", fd);
	uint16_t len = htons(buffer->len);
	ssize_t status = send(fd, &len, sizeof(len), 0);
	if (status < 0) {
		errnoPrint("1111 error happened here %d", status);
		return status;
	}

	status = send(fd, &buffer->text, buffer->len, 0);
	if (status < 0)
		errnoPrint("2222 error happened here %d", status);

	return status;
}
