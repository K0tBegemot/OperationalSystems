#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define NOT_EXECUTE 0
#define CHILD_SLEEP_TIME 5
#define MAIN_SLEEP_TIME 2
#define PTHREAD_CREATE_SUCCESS_VALUE 0
#define PTHREAD_CANCEL_SUCCESS_VALUE 0
#define PTHREAD_JOIN_SUCCESS_VALUE 0
#define CODE_IS_IN_ERRNO 1
#define PRINT_ERROR_STRING 0

void printMessage(char* string)
{
    if (string)
    {
        fprintf(stdout, "%s", string);
    }
}

void cleanupHandler(void *args)
{
    printMessage("Cleanup Handler executed here\n");
}

void *threadFunction(void *string)
{
    if (string != NULL)
    {
        pthread_cleanup_push(cleanupHandler, NULL);
        fprintf(stdout, "%s", (char *)string);
        sleep(CHILD_SLEEP_TIME);
        pthread_cleanup_pop(NOT_EXECUTE);
    }
}

void printError(int errorCode, char *string)
{
    if (string != NULL && errorCode == PRINT_ERROR_STRING)
    {
        fprintf(stderr, "%s", string);
    }
    if(errorCode == CODE_IS_IN_ERRNO)
    {
        perror(string);
    }
}

int main()
{
    pthread_t thread;
    void *threadRes;
    char *string = "Child thread watching over you. Ha ha ha...\n";
    int ret = pthread_create(&thread, NULL, threadFunction, string);
    if (ret != PTHREAD_CREATE_SUCCESS_VALUE)
    {
        printError(CODE_IS_IN_ERRNO, "Error. Child thread wasn't created\n");
        return 0;
    }
    sleep(MAIN_SLEEP_TIME);
    int ret3 = pthread_cancel(thread);
    if (ret3 != PTHREAD_CANCEL_SUCCESS_VALUE)
    {
        printError(CODE_IS_IN_ERRNO, "Error occured in function pthread_cancel. There is no such thread\n");
        return 0;
    }
    int ret1 = pthread_join(thread, &threadRes);
    if (threadRes == PTHREAD_CANCELED)
    {
        printMessage("Thread was cancelled. Ok\n");
        return 0;
    }
    if (ret1 == PTHREAD_JOIN_SUCCESS_VALUE && ret3 == PTHREAD_CANCEL_SUCCESS_VALUE)
    {
        printMessage("Child thread returned earlier then pthread_cancel was executed\n");
        return 0;
    }
    if (ret1 == ESRCH || ret3 == ESRCH)
    {
        printError(PRINT_ERROR_STRING, "Errors in function pthread_cancel and pthread_join. There is no such thread\n");
        return 0;
    }
    return 0;
}
