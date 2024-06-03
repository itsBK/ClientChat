#ifndef USER_H
#define USER_H

#include <pthread.h>
#include <stdbool.h>
#include "network.h"

typedef struct User
{
	struct User *prev;
	struct User *next;
	pthread_t thread;	//thread ID of the client thread
	int sock;			//socket for client
	char* name;
} User;

User* addUser(int socketFd);
void removeUser(User* userToRemove);

/**
 * returns first user if the passed argument is null
 * otherwise returns the next user in the list.
 * WARNING: it returns null after iterating through all the list
*/
User* iterator(User* currentUser);

/**
 * NOTE: name must be null terminated!!
 * returns one of the following:
 * * SUCCESS
 * * NAME_ALREADY_IN_USE
 * * NAME_INVALID
 * if the return code is SUCCESS the name will be also allocated in User struct
*/
enum LoginResponseCode checkAndProcessName(User* user, char* name);
bool isUserLoggedIn(User* user);

#endif
