#include <pthread.h>
#include "util.hpp"
#include "user.hpp"

#include <cstring>

#include "clientthread.hpp"

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = NULL;          // last added user (aka always at front of list)
static User *userBack = NULL;           // first added user (First-In-Last-Out)


void User::add(int socketFd)
{
    User* newUser = reinterpret_cast<User*>(malloc(sizeof(User)));
    pthread_t threadId = 0;
    pthread_create(&threadId, NULL, clientthread, newUser);

    newUser->thread = threadId;
    newUser->sock = socketFd;
    newUser->next = NULL;
    newUser->name = NULL;

    pthread_mutex_lock(&userLock);
    if (userBack == NULL)
    {
        newUser->prev = NULL;
        userFront = newUser;
        userBack = newUser;
    } else
    {
        userFront->next = newUser;
        newUser->prev = userFront;
        userFront = newUser;
    }
    pthread_mutex_unlock(&userLock);
}


void User::remove(User* userToRemove)
{
    User* current = NULL;
    while ((current = iterator(current)) != NULL)
    {
        if (current != userToRemove)
            continue;

        pthread_mutex_lock(&userLock);
        //only one user is left
        if (userBack == userFront)
        {
            userBack = userFront = NULL;
        }
        // first user
        else if (current == userBack)
        {
            userBack = current->next;
            userBack->prev = NULL;
        }
        // last user
        else if (current == userFront)
        {
            userFront = current->prev;
            userFront->next = NULL;
        }
        // a nobody
        else
        {
            current->next->prev = current->prev;
            current->prev->next = current->next;
        }

        pthread_join(userToRemove->thread, NULL);
        if (userToRemove->name != NULL)
            free(userToRemove->name);
        free(userToRemove);
        
        pthread_mutex_unlock(&userLock);
        break;
    }
}


User* User::iterator(User* currentUser)
{
    User* result;
    pthread_mutex_lock(&userLock);

    if (currentUser == NULL)
        result = userBack;
    else
        result = currentUser->next;

    pthread_mutex_unlock(&userLock);
    return result;
}


enum LoginResponseCode checkAndProcessName(User* user, char* name)
{
	// check if only allowed keys are used
	// TODO: maybe replace with regex  "([!-~](?!\'|\"|`))*"
	for (unsigned int i = 0; i < strlen(name); i++)
	{
		char c = name[i];
		if (!(c >= 33 && c <= 126 && !(c == '\'' || c == '\"' || c == '`')))
		{
			return NAME_INVALID;
		}
	}

	// check name availability
	User* current = NULL;
	while ((current = User::iterator(current)) != NULL)
	{
		if (current->name != NULL && strcmp(current->name, name) == 0)
		{
			return NAME_ALREADY_IN_USE;
		}
	}

    pthread_mutex_lock(&userLock);
	user->name = reinterpret_cast<char*>(malloc(strlen(name)));
	strcpy(user->name, name);
    pthread_mutex_unlock(&userLock);
    
    return SUCCESS;
}


bool isUserLoggedIn(User* user)
{
    bool result = false;
    pthread_mutex_lock(&userLock);

    if (user->name != NULL)
        result = true;

    pthread_mutex_unlock(&userLock);
    return result;
}
