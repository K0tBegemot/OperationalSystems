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
    if (string)
    {
        if (errorCode < 0)
        {
            perror(string);
            return;
        }
        errno = errorCode;
        perror(string);
    }
}

arrayOfString *initialiseArrayOfString()
{
    arrayOfString *array = (arrayOfString *)malloc(sizeof(arrayOfString) * N);
    if (array == 0)
    {
        printError(CODE_IS_IN_ERRNO, "malloc can't allocate memory in function initialiseArrayOfString\n");
        return array;
    }
    for (int i = 0; i < N; i++)
    {
        array[i].numberOfString = NUMBER_OF_STRING;
        array[i].arrayOfString = (char **)malloc(sizeof(char *) * array[i].numberOfString);
        if (array[i].arrayOfString == 0)
        {
            printError(CODE_IS_IN_ERRNO, "malloc can't allocate memory in function initialiseArrayOfString\n");
            continue;
        }
        for (int j = 0; j < array[i].numberOfString; j++)
        {
            array[i].arrayOfString[j] = (char *)malloc(sizeof(char) * LENGTH_OF_STRING_WITH_NULL);
            if (array[i].arrayOfString[j] == 0)
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
    if (array == 0)
    {
        printError(NULL_POINTER, "arrayOfString* in function freeArrayOfString() is NULL\n");
        return;
    }
    for (int i = 0; i < N; i++)
    {
        if (array[i].arrayOfString == 0)
        {
            printError(NULL_POINTER, "char** in function freeArrayOfString() is NULL\n");
        }
        else
        {
            for (int j = 0; j < array[i].numberOfString; j++)
            {
                if (array[i].arrayOfString[j] == 0)
                {
                    printError(NULL_POINTER, "char* in function freeArrayOfString() is NULL\n");
                }else
                {
                    free(array[i].arrayOfString[j]);
                }
            }
            free(array[i].arrayOfString);
        }
    }
    free(array);
}

arrayOfThreads *initialiseArrayOfThreads()
{
    arrayOfThreads *array = (arrayOfThreads *)malloc(sizeof(arrayOfThreads));
    if (array == 0)
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
    if (array == 0)
    {
        printError(NULL_POINTER, "arrayOfThreads* in function freeArrayOfThreads() is NULL\n");
        return;
    }
    else
    {
        if (array->arrayOfThreads == 0)
        {
            printError(NULL_POINTER, "pthread_t* in function freeArrayOfThreads() is NULL\n");
        }
        else
        {
            free(array->arrayOfThreads);
        }
        free(array);
    }
}

void *printArrayOfString(void *structure)
{
    arrayOfString *array = (arrayOfString *)structure;
    if (array == 0)
    {
        return 0;
    }
    pid_t tid = syscall(GETTID);
    if(tid == -1)
    {
        printError(CODE_IS_IN_ERRNO, "syscall gettid exit with error\n");
    }
    for (int i = 0; i < array->numberOfString; i++)
    {
        if (array->arrayOfString == 0)
        {
            continue;
        }
        fprintf(stdout, "%s%d%s%s\n", "TID : ", tid, " . String: ", *(array->arrayOfString + i));
    }
    return 0;
}

int initialiseThreads(arrayOfThreads *arrayOfThreads, arrayOfString *arrayOfString)
{
    int ret = 0;
    if (arrayOfThreads == 0 || arrayOfString == 0 || arrayOfThreads->arrayOfThreads == 0)
    {
        return 0;
    }
    for (int i = 0; i < arrayOfThreads->numberOfThreads; i++)
    {
        int retCode = pthread_create(arrayOfThreads->arrayOfThreads + i, NULL, printArrayOfString, arrayOfString + i);
        if (retCode > 0)
        {
            fprintf(stderr, "Thread number %d has terminated with code %d\n", i, retCode);
            return 0;
        }
        ret += 1;
    }
    for (int i = 0; i < arrayOfThreads->numberOfThreads; i++)
    {
        int retCode = pthread_join(*(arrayOfThreads->arrayOfThreads + i), NULL);
        if (retCode > 0)
        {
            fprintf(stderr, "Thread number %d hasn't been joined. Code of error is %d\n", i, retCode);
            return 0;
        }
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
