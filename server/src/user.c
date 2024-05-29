#include <pthread.h>
#include <stdlib.h>
#include "util.h"
#include "user.h"
#include "clientthread.h"

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = NULL;          // last added user (aka always at front of list)
static User *userBack = NULL;           // first added user (First-In-Last-Out)


User* addUser(int socketFd) 
{
    User* newUser = malloc(sizeof(User));
    pthread_t threadId = 0;
    int returnType = pthread_create(&threadId, NULL, clientthread, newUser);

    newUser->thread = threadId;
    newUser->sock = socketFd;
    newUser->prev = NULL;
    newUser->next = NULL;
    newUser->name = NULL;

    pthread_mutex_lock(&userLock);
    if (userBack == NULL)
    {
        userFront = newUser;
        userBack = newUser;
    } else
    {
        userFront->next = newUser;
        newUser->prev = userFront;
        userFront = newUser;
    }
    pthread_mutex_unlock(&userLock);

    return newUser;
}


void removeUser(User* userToRemove)
{
    User* current = iterator(NULL);
    while (1)
    {
        if (current != userToRemove)
        {
            current = iterator(current);
            continue;

        } else if (current == NULL)
        {
            errorPrint("ERROR: requested user to remove does not exist!");
            break;
        }

        pthread_mutex_lock(&userLock);
        if (userBack == userFront)
        {
            userBack = userFront = NULL;
        }
        else if (current == userBack)
        {
            userBack = current->next;
            userBack->prev = NULL;
        }
        else if (current == userFront)
        {
            userFront = current->prev;
            userFront->next = NULL;
        }
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


User* iterator(User* currentUser)
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
