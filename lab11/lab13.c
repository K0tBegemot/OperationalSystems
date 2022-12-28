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
#define PTHREAD_MUTEX_LOCK_SUCCESS 0
#define PTHREAD_MUTEX_CONSISTENT_SUCCESS 0
#define PTHREAD_MUTEX_UNLOCK_SUCCESS 0
#define INIT_LOCK_PRIMITIVE_SUCCESS 0
#define INIT_LOCK_PRIMITIVE_ERROR 1
#define NUMBER_OF_SECOND_THREAD 1
#define NUMBER_OF_FIRST_THREAD 0
#define PTHREAD_COND_INIT_SUCCESS 0
#define PTHREAD_COND_WAIT_SUCCESS 0
#define PTHREAD_COND_WAIT_ERROR 1
#define PTHREAD_JOIN_SUCCESS 0
#define PTHREAD_ATTR_INIT_SUCCESS 0
#define PTHREAD_SET_DETACH_SUCCESS 0
#define PTHREAD_MUTEX_DESTROY_SUCCESS 0
#define DESTROY_LOCK_PRIMITIVE_SUCCESS 0
#define DESTROY_LOCK_PRIMITIVE_ERROR -1
#define PRINT_PRIMITIVE_ERROR (long)1
#define PRINT_PRIMITIVE_SUCCESS (long)0
#define PTHREAD_COND_DESTROY_SUCCESS 0
#define PTHREAD_SIGNAL_SUCCESS 0

typedef struct lockPrimitive
{
    pthread_mutex_t mutex;
    pthread_cond_t condVar;
    int numberOfLastThread;
} lockPrimitive;

typedef struct threadData
{
    int threadNumber;
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
        return INIT_LOCK_PRIMITIVE_ERROR;
    }
    if (errorCode == EDEADLK)
    {
        printError(PRINT_ERROR_STRING, "Error. Deadlock happened when programm try to use pthread_mutex_lock function\n");
        return INIT_LOCK_PRIMITIVE_ERROR;
    }
    return INIT_LOCK_PRIMITIVE_SUCCESS;
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

int condErrorChecker(int errorCode, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    if ((mutexLockErrorChecker(errorCode, mutex) || mutexUnlockErrorChecker(errorCode, mutex)) != PTHREAD_COND_WAIT_SUCCESS)
    {
        return PTHREAD_COND_WAIT_ERROR;
    }
    int retCode = pthread_cond_wait(cond, mutex);
    if (retCode != PTHREAD_COND_WAIT_SUCCESS)
    {
        return condErrorChecker(retCode, cond, mutex);
    }
    return PTHREAD_COND_WAIT_SUCCESS;
}

int lockMutex()
{
    pthread_mutex_t *lockMutexPtr = &locker.mutex;
    int retCode = pthread_mutex_lock(lockMutexPtr);
    if (retCode != PTHREAD_MUTEX_LOCK_SUCCESS)
    {
        retCode = mutexLockErrorChecker(retCode, lockMutexPtr);
        return retCode;
    }
    return PTHREAD_MUTEX_LOCK_SUCCESS;
}

int unlockMutex()
{
    pthread_mutex_t *unlockMutexPtr = &locker.mutex;
    int retCode = pthread_mutex_unlock(unlockMutexPtr);
    if (retCode != PTHREAD_MUTEX_UNLOCK_SUCCESS)
    {
        retCode = mutexUnlockErrorChecker(retCode, unlockMutexPtr);
        return retCode;
    }
    return PTHREAD_MUTEX_UNLOCK_SUCCESS;
}

int waitCond()
{
    pthread_cond_t *cond = &locker.condVar;
    pthread_mutex_t *mutex = &locker.mutex;
    int retCode = pthread_cond_wait(&locker.condVar, &locker.mutex);
    if (retCode != PTHREAD_COND_WAIT_SUCCESS)
    {
        retCode = condErrorChecker(retCode, cond, mutex);
        return retCode;
    }
    return PTHREAD_COND_WAIT_SUCCESS;
}

int destroyMutexErrorHandler(int errorCode)
{
    if (errorCode == EINVAL)
    {
        return DESTROY_LOCK_PRIMITIVE_SUCCESS;
    }
    if (errorCode == EBUSY)
    {
        errno = errorCode;
        printError(CODE_IS_IN_ERRNO, "Error. Function try to destroy locked mutex\n");
        return DESTROY_LOCK_PRIMITIVE_ERROR;
    }
    return DESTROY_LOCK_PRIMITIVE_SUCCESS;
}

int destroyCondVarErrorHandler(int errorCode)
{
    if (errorCode == EINVAL)
    {
        return DESTROY_LOCK_PRIMITIVE_SUCCESS;
    }
    if (errorCode == EBUSY)
    {
        errno = errorCode;
        printError(CODE_IS_IN_ERRNO, "Error. Function try to destroy waited condition variable\n");
        return DESTROY_LOCK_PRIMITIVE_ERROR;
    }
    return DESTROY_LOCK_PRIMITIVE_SUCCESS;
}

void destroyLockPrimitive()
{
    int retCode = pthread_mutex_destroy(&locker.mutex);
    if (retCode != PTHREAD_MUTEX_DESTROY_SUCCESS)
    {
        retCode = destroyMutexErrorHandler(retCode);
        return;
    }
    retCode = pthread_cond_destroy(&locker.condVar);
    if (retCode != PTHREAD_COND_DESTROY_SUCCESS)
    {
        retCode = destroyCondVarErrorHandler(retCode);
        return;
    }
    return;
}

int unlockLockPrimitive()
{
    int retCode = pthread_cond_signal(&locker.condVar);
    // do nothingg cause it's not important here, that happened with signal. Point is, that if we bloack mutex in main thread, then cond_wait
    // in child thread will terminate with error and child thread will close
    retCode = unlockMutex();
    if (retCode != PTHREAD_MUTEX_UNLOCK_SUCCESS)
    {
        return INIT_LOCK_PRIMITIVE_ERROR;
    }
    return INIT_LOCK_PRIMITIVE_SUCCESS;
}

int initLockPrimitive()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    int retCode = pthread_mutex_init(&locker.mutex, &attr);
    if (retCode != PTHREAD_MUTEX_INIT_SUCCESS)
    {
        errno = retCode;
        printError(CODE_IS_IN_ERRNO, "Error while processing pthread_mutex_init\n");
        destroyLockPrimitive();
        return INIT_LOCK_PRIMITIVE_ERROR;
    }
    pthread_mutexattr_destroy(&attr);
    retCode = pthread_cond_init(&locker.condVar, NULL);
    if (retCode != PTHREAD_COND_INIT_SUCCESS)
    {
        errno = retCode;
        printError(CODE_IS_IN_ERRNO, "Error while processing pthread_lock_init\n");
        destroyLockPrimitive();
        return INIT_LOCK_PRIMITIVE_ERROR;
    }
    locker.numberOfLastThread = NUMBER_OF_SECOND_THREAD;
    return INIT_LOCK_PRIMITIVE_SUCCESS;
}

void *printPrimitive(void *voidData)
{
    int retCode = 0;
    int currentIndex = 0;
    threadData *data = (threadData *)voidData;
    while (currentIndex < N)
    {
        retCode = lockMutex();
        if (retCode != PTHREAD_MUTEX_LOCK_SUCCESS)
        {
            return (void *)PRINT_PRIMITIVE_ERROR;
        }
        while (locker.numberOfLastThread == data->threadNumber)
        {
            retCode = waitCond();
            if (retCode != PTHREAD_COND_WAIT_SUCCESS)
            {
                return (void *)PRINT_PRIMITIVE_ERROR;
            }
        }
        printMessage(data->message);
        locker.numberOfLastThread = data->threadNumber;
        currentIndex += 1;
        retCode = unlockMutex();
        if (retCode != PTHREAD_MUTEX_UNLOCK_SUCCESS)
        {
            return (void *)PRINT_PRIMITIVE_ERROR;
        }
        retCode = pthread_cond_signal(&locker.condVar);
        if (retCode != PTHREAD_SIGNAL_SUCCESS)
        {
            return (void *)PRINT_PRIMITIVE_ERROR;
        }
    }
    return (void *)PRINT_PRIMITIVE_SUCCESS;
}

int main()
{
    pthread_t newThread;
    initLockPrimitive();
    threadData childData;
    childData.threadNumber = NUMBER_OF_SECOND_THREAD;
    childData.message = "This is second thread!\n";
    int createResult = pthread_create(&newThread, NULL, printPrimitive, &childData);
    if (createResult != PTHREAD_CREATE_SUCCESS)
    {
        printError(PRINT_ERROR_STRING, "Error: pthread_create couldn't create thread\n");
        return ERROR;
    }
    threadData mainData;
    mainData.threadNumber = NUMBER_OF_FIRST_THREAD;
    mainData.message = "This is first thread!\n";
    long thread1RetValue = (long)printPrimitive(&mainData);
    long thread2RetValue = PRINT_PRIMITIVE_ERROR;
    int joinResult = pthread_join(newThread, (void **)&thread2RetValue);
    int retCode = unlockLockPrimitive();
    if (retCode == INIT_LOCK_PRIMITIVE_ERROR)
    {
        return ERROR;
    }
    destroyLockPrimitive();
    if (joinResult != PTHREAD_JOIN_SUCCESS)
    {
        printError(CODE_IS_IN_ERRNO, "Error: pthread_join coudn't join thread\n");
        return ERROR;
    }
    if (thread1RetValue == PRINT_PRIMITIVE_ERROR || thread2RetValue == PRINT_PRIMITIVE_ERROR)
    {
        printError(CODE_IS_IN_ERRNO, "Error: one of the threads ended with an error\n");
        return ERROR;
    }
    pthread_exit(NULL);
}