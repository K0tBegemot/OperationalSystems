#include <stdio.h>
#include <pthread.h>

#define SUCCESS 0
#define ERROR 1
#define PTHREAD_CREATE_ERROR 0
#define N 10

void* printMessage() {
    for (int i = 0; i < N; i++)
    {
        printf("New thread\n");
    }
}

int main() {
    pthread_t new_thread;
    int create_result = pthread_create(&new_thread, NULL, printMessage, NULL);

    if (create_result != PTHREAD_CREATE_ERROR) {
        printf("pthread_create error: couldn't create thread\n");
        return ERROR;
    }

    for (int i = 0; i < N; i++)
    {
        printf("Main thread\n");
    }

    pthread_exit(NULL);
}

