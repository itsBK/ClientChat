#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <stdint.h>

enum { MSG_MAX = 1024 };

typedef struct __attribute__((packed))
{
	uint16_t len;		//network byte order, i.e. big endian, 0 <= len <= MSG_MAX
	char message[MSG_MAX];	//variadic length
} Message;

#endif
