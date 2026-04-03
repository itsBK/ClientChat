#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <sys/socket.h>
#include "util.hpp"

/* TODO: When implementing the fully-featured network protocol (including
 * login), replace this with message structures derived from the network
 * protocol (RFC) as found in the moodle course. */

#define MESSAGE_TYPE_MAXIMAM_VALUE 5

#define LOGIN_REQUEST_MAGIC_VALUE 0x0badf00d
#define LOGIN_REQUEST_PROTOCOL_VERSION 0

#define LOGIN_RESPONSE_MAGIC_VALUE 0xc001c001

/**
 * NULL terminator excluded
*/
enum { NAME_MAX_LENGTH = 31 };
/**
 * NULL terminator excluded
*/
enum { TEXT_MAX_LENGTH = 512 };

enum LoginResponseCode
{
	SUCCESS = 0,
	NAME_ALREADY_IN_USE = 1,
	NAME_INVALID = 2,
	PROTOCOL_VERSION_MISMATCH = 3,
	SERVER_ERROR = 255
};

enum UserRemovedCode
{
	CONNECTION_CLOSED = 0,
	KICKED_FROM_SERVER = 1,
	COMMUNICATION_ERROR = 2
};

enum MessageType
{
	LOGIN_REQUEST = 0,
	LOGIN_RESPONSE = 1,
	CLIENT_2_SERVER = 2,
	SERVER_2_CLIENT = 3,
	USER_ADDED = 4,
	USER_REMOVED = 5,
};

typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint16_t len;		//real length of the text (big endian, len <= MSG_MAX)
	void* data;	//text message
} Message;


typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint16_t len;
	uint32_t magic;
	uint8_t version;
	char name[NAME_MAX_LENGTH];
} LoginRequest;

typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint16_t len;
	uint32_t magic;
	uint8_t code;
	char sName[NAME_MAX_LENGTH];
} LoginResponse;



typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint16_t len;
	char text[TEXT_MAX_LENGTH];
} Client2Server;

typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint16_t len;
	uint64_t timestamp;
	char originalSender[NAME_MAX_LENGTH + 1];
	char text[TEXT_MAX_LENGTH];
} Server2Client;



typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint16_t len;
	uint64_t timestamp;
	char name[NAME_MAX_LENGTH];
} UserAdded;

typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint16_t len;
	uint64_t timestamp;
	u_int8_t code;
	char name[NAME_MAX_LENGTH];
} UserRemoved;


enum { MESSAGE_MAX_LENGTH = sizeof(Server2Client) };

/**
 * returns
 * * -1: error
 * *  0: when connection is closed
 * * >0: message received successfully
*/
int networkReceive(int fd, Message *buffer);
int networkSend(int fd, const Message *buffer);

#endif
