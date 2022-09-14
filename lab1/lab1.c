#include <stdio.h>
#include <pthread.h>

#define SUCCESS 0
#define ERROR 1
#define PTHREAD_CREATE_ERROR 0
#define N 10

void* printMessage()
{
    for (int i = 0; i < N; i++)
    {
        fprintf(stdout, "New thread\n");
    }
}

int main()
{
    pthread_t newThread;
    int createResult = pthread_create(&newThread, NULL, printMessage, NULL);

    if (createResult != PTHREAD_CREATE_ERROR) {
        fprintf(stderr, "pthread_create error: couldn't create thread\n");
        return ERROR;
    }

    for (int i = 0; i < N; i++)
    {
        fprintf(stdout, "Main thread\n");
    }

    pthread_exit(NULL);
}