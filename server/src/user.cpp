#include <pthread.h>
#include "user.hpp"

#include <cstdlib>
#include <vector>
#include <cstring>

#include "clientthread.hpp"


static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = nullptr;          // last added user (aka always at front of list)
static User *userBack = nullptr;           // first added user (First-In-Last-Out)

UserIterator UserIterator::users = {};

UserIterator::UserIterator() : current(nullptr) {}

UserIterator::UserIterator(User* user)
{
    pthread_mutex_lock(&userLock);
    current = user;
    pthread_mutex_unlock(&userLock);
}

UserIterator UserIterator::begin()
{
    return { userBack };
}

UserIterator UserIterator::end()
{
    return {};
}

UserIterator& UserIterator::operator++()
{
    pthread_mutex_lock(&userLock);
    current = current->next;
    pthread_mutex_unlock(&userLock);
    return *this;
}

User* UserIterator::operator*() const
{
    return current;
}

bool UserIterator::operator!=(const UserIterator &other) const
{
    return current != other.current;
}


void User::add(int socketFd, std::string* serverName)
{
    User* newUser = reinterpret_cast<User*>(malloc(sizeof(User)));
    pthread_t threadId = 0;
    pthread_create(&threadId, nullptr, clientthread, newUser);

    newUser->thread = threadId;
    newUser->sock = socketFd;
    newUser->next = nullptr;
    newUser->name = nullptr;
    newUser->serverName = serverName;

    pthread_mutex_lock(&userLock);
    if (userBack == nullptr)
    {
        newUser->prev = nullptr;
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
    for (auto current : UserIterator::users)
    {
        if (current != userToRemove)
            continue;

        pthread_mutex_lock(&userLock);
        //only one user is left
        if (userBack == userFront)
        {
            userBack = userFront = nullptr;
        }
        // first user
        else if (current == userBack)
        {
            userBack = current->next;
            userBack->prev = nullptr;
        }
        // last user
        else if (current == userFront)
        {
            userFront = current->prev;
            userFront->next = nullptr;
        }
        // a nobody
        else
        {
            current->next->prev = current->prev;
            current->prev->next = current->next;
        }

        pthread_join(userToRemove->thread, nullptr);
        if (userToRemove->name != nullptr)
            free(userToRemove->name);
        free(userToRemove);
        
        pthread_mutex_unlock(&userLock);
        break;
    }
}

LoginResponseCode User::CheckAndProcessName(char* name)
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
    for (auto current : UserIterator::users)
	{
		if (current->name != nullptr && strcmp(current->name, name) == 0)
		{
			return NAME_ALREADY_IN_USE;
		}
	}

    pthread_mutex_lock(&userLock);
	this->name = reinterpret_cast<char*>(malloc(strlen(name)));
	strcpy(this->name, name);
    pthread_mutex_unlock(&userLock);
    
    return SUCCESS;
}


bool isUserLoggedIn(User* user)
{
    bool result = false;
    pthread_mutex_lock(&userLock);

    if (user->name != nullptr)
        result = true;

    pthread_mutex_unlock(&userLock);
    return result;
}
