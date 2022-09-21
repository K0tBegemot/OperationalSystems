#include <stdio.h>
#include <pthread.h>

#define SUCCESS 0
#define ERROR 1
#define PTHREAD_CREATE_ERROR 0
#define N 10

void *printMessage(void *message)
{
    if (message != NULL)
    {
        for (int i = 0; i < N; i++)
        {
            fprintf(stdout, "%s", (char *)message);
        }
    }
    else
    {
        fprintf(stderr, "Main process exited or null pointer was given to function\n");
    }
}

int main()
{
    char *mainThreadText = "Main thread\n";
    char *childThreadText = "New thread\n";
    pthread_t newThread;
    int createResult = pthread_create(&newThread, NULL, printMessage, childThreadText);

    if (createResult != PTHREAD_CREATE_ERROR)
    {
        fprintf(stderr, "pthread_create error: couldn't create thread\n");
        return ERROR;
    }

    int retCode = pthread_join(newThread, NULL);
    if(retCode != 0)
    {
        fprintf(stderr, "Pthread_join function exit with error. Error code: %d\n", retCode);
    }

    printMessage(mainThreadText);

    pthread_exit(NULL);
}
