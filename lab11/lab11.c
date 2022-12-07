#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#define SUCCESS 0
#define ERROR 1
#define PTHREAD_CREATE_SUCCESS 0
#define N 10
#define NUMBER_OF_STRING 1
#define CODE_IS_IN_ERRNO 1
#define PRINT_ERROR_STRING 0
#define NUMBER_OF_MUTEX 3
#define PTHREAD_MUTEX_INIT_SUCCESS 0
#define FIRST_MUTEX_INDEX 0
#define SECOND_MUTEX_INDEX 1
#define THIRD_MUTEX_INDEX 2
#define PTHREAD_MUTEX_LOCK_SUCCESS 0
#define PTHREAD_MUTEX_CONSISTENT_SUCCESS 0
#define PTHREAD_MUTEX_UNLOCK_SUCCESS 0
#define SYNCHRO_TIME 1
#define FIRST_THREAD_INDEX 0
#define SECOND_THREAD_INDEX 1
#define FIRST_THREAD_FIRST_LOCK_INDEX 0
#define SECOND_THREAD_FIRST_LOCK_INDEX (NUMBER_OF_MUTEX - 1)
#define PTHREAD_JOIN_SUCCESS 0

#define INIT_LOCK_PRIMITIVE_SUCCESS 0
#define INIT_LOCK_PRIMITIVE_ERROR 1

typedef struct lockPrimitive
{
    pthread_mutex_t mutexArray[NUMBER_OF_MUTEX];
} lockPrimitive;

typedef struct threadData
{
    int threadIndex;
    int firstLockMutexIndex;
    char *message;
} threadData;

lockPrimitive locker;

void printError(int errorCode, char *string)
{
    if (string != NULL && errorCode == PRINT_ERROR_STRING)
    {
        fprintf(stderr, "%s", string);
    }
    if (errorCode == CODE_IS_IN_ERRNO)
    {
        perror(string);
    }
}

void printMessage(char *message)
{
    if (message == NULL)
    {
        return;
    }
    fprintf(stdout, "%s", message);
    return;
}

int mutexLockErrorChecker(int errorCode, pthread_mutex_t *mutex)
{
    if (errorCode == ENOTRECOVERABLE)
    {
        printError(PRINT_ERROR_STRING, "Error. Mutex con't be locked because it is not recoverable\n");
        return INIT_LOCK_PRIMITIVE_ERROR;
    }
    if (errorCode == EOWNERDEAD)
    {
        printError(PRINT_ERROR_STRING, "Warning. Owner of this mutex is died. Start process of recovering\n");
        int retCode = pthread_mutex_consistent(mutex);
        if (retCode != PTHREAD_MUTEX_CONSISTENT_SUCCESS)
        {
            printError(PRINT_ERROR_STRING, "Error. Mutex's owner is dead but mutex is not in inconsistent state\n");
            return INIT_LOCK_PRIMITIVE_ERROR;
        }
        printError(PRINT_ERROR_STRING, "Warning. Mutex is recovered. Start locking of recovered mutex\n");
        retCode = pthread_mutex_lock(mutex);
        if (retCode == PTHREAD_MUTEX_LOCK_SUCCESS)
        {
            printError(PRINT_ERROR_STRING, "Warning. Recovered mutex is succesfully locked\n");
            return INIT_LOCK_PRIMITIVE_SUCCESS;
        }
        printError(PRINT_ERROR_STRING, "Error. Error occured during lock of recovered mutex\n");
        return mutexLockErrorChecker(retCode, mutex);
    }
    if (errorCode == EDEADLK)
    {
        printError(PRINT_ERROR_STRING, "Error. Deadlock happened when programm try to use pthread_mutex_lock function\n");
        return INIT_LOCK_PRIMITIVE_ERROR;
    }
}

int mutexUnlockErrorChecker(int errorCode, pthread_mutex_t *mutex)
{
    if (errorCode == EPERM)
    {
        printError(PRINT_ERROR_STRING, "Error. This mutex can't be unlocked since it was locked by another thread\n");
        return INIT_LOCK_PRIMITIVE_ERROR;
    }
    return INIT_LOCK_PRIMITIVE_SUCCESS;
}

int lockMutex(int index)
{
    pthread_mutex_t *lockMutexPtr = locker.mutexArray + index;
    int retCode = pthread_mutex_lock(lockMutexPtr);
    if (retCode != PTHREAD_MUTEX_LOCK_SUCCESS)
    {
        retCode = mutexLockErrorChecker(retCode, lockMutexPtr);
        return retCode;
    }
    return PTHREAD_MUTEX_LOCK_SUCCESS;
}

int unlockMutex(int index)
{
    pthread_mutex_t *unlockMutexPtr = locker.mutexArray + index;
    int retCode = pthread_mutex_unlock(unlockMutexPtr);
    if (retCode != PTHREAD_MUTEX_UNLOCK_SUCCESS)
    {
        retCode = mutexUnlockErrorChecker(retCode, unlockMutexPtr);
        return retCode;
    }
    return PTHREAD_MUTEX_UNLOCK_SUCCESS;
}

int initLockPrimitive()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    for (int i = 0; i < NUMBER_OF_MUTEX; i++)
    {
        int retCode = pthread_mutex_init(locker.mutexArray + i, &attr);
        if (retCode != PTHREAD_MUTEX_INIT_SUCCESS)
        {
            errno = retCode;
            printError(CODE_IS_IN_ERRNO, "Error while processing pthread_mutex_init\n");
            return INIT_LOCK_PRIMITIVE_ERROR;
        }
    }
    pthread_mutexattr_destroy(&attr);
    return lockMutex(FIRST_THREAD_FIRST_LOCK_INDEX);
}

void *printPrimitive(void *voidData)
{
    threadData *data = (threadData *)voidData;
    int cur_mutex_index = data->firstLockMutexIndex;
    int retCode = 0;
    if (data->threadIndex == SECOND_THREAD_INDEX)
    {
        retCode = lockMutex((cur_mutex_index) % NUMBER_OF_MUTEX);
        if (retCode != PTHREAD_MUTEX_LOCK_SUCCESS)
        {
            return NULL;
        }
    }
    for (int i = 0; i < N; i++)
    {
        retCode = lockMutex((cur_mutex_index + 1) % NUMBER_OF_MUTEX);
        if (retCode != PTHREAD_MUTEX_LOCK_SUCCESS)
        {
            return NULL;
        }
        printMessage(data->message);
        unlockMutex((cur_mutex_index) % NUMBER_OF_MUTEX);
        if (retCode != PTHREAD_MUTEX_UNLOCK_SUCCESS)
        {
            return NULL;
        }
        cur_mutex_index = (cur_mutex_index + 1) % NUMBER_OF_MUTEX;
    }
    retCode = unlockMutex(cur_mutex_index % NUMBER_OF_MUTEX);
    if (retCode != PTHREAD_MUTEX_UNLOCK_SUCCESS)
    {
        return NULL;
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t newThread;
    initLockPrimitive();
    threadData childData;
    childData.threadIndex = SECOND_THREAD_INDEX;
    childData.firstLockMutexIndex = SECOND_THREAD_FIRST_LOCK_INDEX;
    childData.message = "This is second thread!\n";
    int createResult = pthread_create(&newThread, NULL, printPrimitive, &childData);
    sleep(SYNCHRO_TIME);
    if (createResult != PTHREAD_CREATE_SUCCESS)
    {
        printError(PRINT_ERROR_STRING, "Error: pthread_create couldn't create thread\n");
        return ERROR;
    }
    threadData mainData;
    mainData.threadIndex = FIRST_THREAD_INDEX;
    mainData.firstLockMutexIndex = FIRST_THREAD_FIRST_LOCK_INDEX;
    mainData.message = "This is first thread!\n";
    printPrimitive(&mainData);
    int joinResult = pthread_join(newThread, NULL);
    if (joinResult != PTHREAD_JOIN_SUCCESS)
    {
        printError(CODE_IS_IN_ERRNO, "Error: pthread_join coudn't join thread\n");
        return ERROR;
    }
}