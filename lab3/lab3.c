#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#define N 4
#define NUMBER_OF_STRING 10
#define LENGTH_OF_STRING_WITH_NULL 5
#define ASCII_ZERO_CODE (int)'0'
#define CODE_IS_IN_ERRNO -1
#define NULL_POINTER -2
#define UNCESS_THREAD_ARRAY_INIT -3
#define GETTID 186
#define GET_TID_ERR_VALUE -1
#define DEF_VALUE_OF_INIT_THREADS 0
#define PTHREAD_CREATE_SUCCESS_VALUE 0
#define PTHREAD_JOIN_SUCCESS_VALUE 0

typedef struct arrayOfString
{
    int numberOfString;
    char **arrayOfString;
} arrayOfString;

typedef struct arrayOfThreads
{
    int numberOfThreads;
    pthread_t *arrayOfThreads;
} arrayOfThreads;

void printError(int errorCode, char *string)
{
    if (string == NULL)
    {
        return;
    }
    if (errorCode == CODE_IS_IN_ERRNO)
    {
        perror(string);
        return;
    }
    if (errorCode == NULL_POINTER || errorCode == UNCESS_THREAD_ARRAY_INIT)
    {
        fprintf(stderr, "%s", string);
    }
    errno = errorCode;
    perror(string);
}

arrayOfString *initialiseArrayOfString()
{
    arrayOfString *array = (arrayOfString *)malloc(sizeof(arrayOfString) * N);
    if (array == NULL)
    {
        printError(CODE_IS_IN_ERRNO, "malloc can't allocate memory in function initialiseArrayOfString\n");
        return array;
    }
    for (int i = 0; i < N; i++)
    {
        array[i].numberOfString = NUMBER_OF_STRING;
        array[i].arrayOfString = (char **)malloc(sizeof(char *) * array[i].numberOfString);
        if (array[i].arrayOfString == NULL)
        {
            printError(CODE_IS_IN_ERRNO, "malloc can't allocate memory in function initialiseArrayOfString\n");
            continue;
        }
        for (int j = 0; j < array[i].numberOfString; j++)
        {
            array[i].arrayOfString[j] = (char *)malloc(sizeof(char) * LENGTH_OF_STRING_WITH_NULL);
            if (array[i].arrayOfString[j] == NULL)
            {
                printError(CODE_IS_IN_ERRNO, "malloc can't allocate memory in function initialiseArrayOfString\n");
                continue;
            }
            for (int k = 0; k < LENGTH_OF_STRING_WITH_NULL - 1; k++)
            {
                array[i].arrayOfString[j][k] = (char)j + k + ASCII_ZERO_CODE + i;
            }
            array[i].arrayOfString[j][LENGTH_OF_STRING_WITH_NULL - 1] = '\0';
        }
    }
    return array;
}

void freeArrayOfString(arrayOfString *array)
{
    if (array == NULL)
    {
        printError(NULL_POINTER, "arrayOfString* in function freeArrayOfString() is NULL\n");
        return;
    }
    for (int i = 0; i < N; i++)
    {
        if (array[i].arrayOfString == NULL)
        {
            printError(NULL_POINTER, "char** in function freeArrayOfString() is NULL\n");
            continue;
        }

        for (int j = 0; j < array[i].numberOfString; j++)
        {
            if (array[i].arrayOfString[j] == NULL)
            {
                printError(NULL_POINTER, "char* in function freeArrayOfString() is NULL\n");
                continue;
            }

            free(array[i].arrayOfString[j]);
        }

        free(array[i].arrayOfString);
    }
    free(array);
}

arrayOfThreads *initialiseArrayOfThreads()
{
    arrayOfThreads *array = (arrayOfThreads *)malloc(sizeof(arrayOfThreads));
    if (array == NULL)
    {
        printError(NULL_POINTER, "arrayOfThreads* in function initialiseArrayOfThreads() is NULL\n");
        return array;
    }
    array->numberOfThreads = N;
    array->arrayOfThreads = (pthread_t *)malloc(sizeof(pthread_t) * array->numberOfThreads);
    return array;
}

void freeArrayOfThreads(arrayOfThreads *array)
{
    if (array == NULL)
    {
        printError(NULL_POINTER, "arrayOfThreads* in function freeArrayOfThreads() is NULL\n");
        return;
    }
    if (array->arrayOfThreads == NULL)
    {
        printError(NULL_POINTER, "pthread_t* in function freeArrayOfThreads() is NULL\n");
        free(array);
        return;
    }
    free(array->arrayOfThreads);
    free(array);
}

void *printArrayOfString(void *structure)
{
    arrayOfString *array = (arrayOfString *)structure;
    if (array == NULL)
    {
        return 0;
    }
    pid_t tid = syscall(GETTID);
    if (tid == GET_TID_ERR_VALUE)
    {
        printError(CODE_IS_IN_ERRNO, "syscall gettid exit with error\n");
    }
    for (int i = 0; i < array->numberOfString; i++)
    {
        if (array->arrayOfString == NULL)
        {
            break;
        }
        fprintf(stdout, "%s%d%s%s\n", "TID : ", tid, " . String: ", *(array->arrayOfString + i));
    }
    return 0;
}

int initialiseThreads(arrayOfThreads *arrayOfThreads, arrayOfString *arrayOfString)
{
    int ret = DEF_VALUE_OF_INIT_THREADS;
    if (arrayOfThreads == NULL || arrayOfString == NULL || arrayOfThreads->arrayOfThreads == NULL)
    {
        return ret;
    }
    for (int i = 0; i < arrayOfThreads->numberOfThreads; i++)
    {
        int retCode = pthread_create(arrayOfThreads->arrayOfThreads + i, NULL, printArrayOfString, arrayOfString + i);
        if (retCode != PTHREAD_CREATE_SUCCESS_VALUE)
        {
            fprintf(stderr, "Thread number %d has terminated with code %d\n", i, retCode);
            return ret;
        }
    }
    for (int i = 0; i < arrayOfThreads->numberOfThreads; i++)
    {
        int retCode = pthread_join(*(arrayOfThreads->arrayOfThreads + i), NULL);
        if (retCode != PTHREAD_JOIN_SUCCESS_VALUE)
        {
            fprintf(stderr, "Thread number %d hasn't been joined. Code of error is %d\n", i, retCode);
            return ret;
        }
        ret += 1;
    }
    return ret;
}

int main(int argc, char **argv)
{
    arrayOfString *arrayOfString = initialiseArrayOfString();
    arrayOfThreads *arrayOfThreads = initialiseArrayOfThreads();
    if (initialiseThreads(arrayOfThreads, arrayOfString) < N)
    {
        printError(UNCESS_THREAD_ARRAY_INIT, "Error in function initializeThreads: not all threads were initialised succesfully\n");
    }
    freeArrayOfString(arrayOfString);
    freeArrayOfThreads(arrayOfThreads);
    exit(0);
}
