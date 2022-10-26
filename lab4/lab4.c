#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define CHILD_SLEEP_TIME 5
#define MAIN_SLEEP_TIME 2
#define PTHREAD_CREATE_SUCCESS_VALUE 0
#define PTHREAD_CANCEL_SUCCESS_VALUE 0

void *threadFunction(void *string)
{
    if (string != 0)
    {
        fprintf(stdout, "%s", (char *)string);
        sleep(CHILD_SLEEP_TIME);
    }
}

void printError(char *string)
{
    if (string)
    {
        fprintf(stderr, "%s", string);
    }
}

void printMessage(char *string)
{
    if (string)
    {
        fprintf(stdout, "%s", string);
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
        printError("Error. Child thread wasn't created\n");
        return 0;
    }
    sleep(MAIN_SLEEP_TIME);
    int ret3 = pthread_cancel(thread);
    if (ret3 != PTHREAD_CANCEL_SUCCESS_VALUE)
    {
        printError("Error occured in function pthread_cancel. There is no such thread\n");
        return 0;
    }
    int ret1 = pthread_join(thread, &threadRes);
    if (threadRes == PTHREAD_CANCELED)
    {
        printMessage("Thread was cancelled. Ok\n");
        return 0;
    }
    if (ret1 == 0 && ret3 == 0)
    {
        printMessage("Child thread returned earlier then pthread_cancel was executed\n");
        return 0;
    }
    if (ret1 == ESRCH || ret3 == ESRCH)
    {
        printError("Errors in function pthread_cancel and pthread_join. There is no such thread\n");
        return 0;
    }
    return 0;
}