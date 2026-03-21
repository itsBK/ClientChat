#ifndef USER_H
#define USER_H

#include <pthread.h>
#include "network.hpp"

class User
{
public:
	User *prev;
	User *next;
	pthread_t thread;	//thread ID of the client thread
	int sock;			//socket for client
	char* name;

	static void add(int socketFd);
	static void remove(User* userToRemove);
	/**
	 * returns first user if the passed argument is null
	 * otherwise returns the next user in the list.
	 * WARNING: it returns null after iterating through all the list
	*/
	static User* iterator(User* currentUser);
};



/**
 * NOTE: name must be null terminated!!
 * @returns one of the following:<br>
 * * SUCCESS<br>
 * * NAME_ALREADY_IN_USE<br>
 * * NAME_INVALID<br>
 * if the return code is SUCCESS the name will be also allocated in User struct
*/
enum LoginResponseCode checkAndProcessName(User* user, char* name);
bool isUserLoggedIn(User* user);

#endif
