#ifndef USER_H
#define USER_H

#include <pthread.h>
#include "network.hpp"

class User;

class UserIterator
{
	UserIterator();
	UserIterator(User* user);
	User* current;

public:
	static UserIterator users;
	~UserIterator() = default;

	static UserIterator begin();
	static UserIterator end();
	UserIterator& operator++();
	User* operator*() const;
	bool operator!=(const UserIterator& other) const;
};

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
	 * NOTE: name must be null terminated!!
	 * @returns one of the following:<br>
	 * * SUCCESS<br>
	 * * NAME_ALREADY_IN_USE<br>
	 * * NAME_INVALID<br>
	 * if the return code is SUCCESS the name will be also allocated in User struct
	*/
	LoginResponseCode CheckAndProcessName(char* name);
};



bool isUserLoggedIn(User* user);

#endif
