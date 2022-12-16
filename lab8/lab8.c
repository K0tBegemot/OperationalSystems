#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#define CODE_IS_IN_ERRNO 1
#define PRINT_ERROR_STRING 0
#define RIGHT_NUMBER_OF_ARGS 2
#define DEF_RET_VALUE_OF_COUNTPI 0
#define PTHREAD_CREATE_SUCCESS_VALUE 0
#define PTHREAD_MUTEX_INIT_SUCCESS 0
#define ARGV_NUM_OF_THREADS_INDEX 1
#define PTHREAD_JOIN_SUCCESS_VALUE 0
#define NUMBER_OF_ITERATIONS 2000000000
#define RESTART_COMPLETE 0
#define RESTART_FAILED 1
#define PTHREAD_END 0
#define PTHREAD_EXECUTED 1
#define DEF_RET_THREAD_NUMBER 0
#define NOT_EXECUTE 0
#define IS_NUMBER 0
#define IS_NOT_NUMBER 1
#define END_OF_C_LINE 0
#define INDEX_STEP 1
#define INVALID_PI -1

typedef struct arrayOfThreads
{
    int numberOfThreads;
    pthread_t *arrayOfThreads;
} arrayOfThreads;

typedef struct threadInitData
{
    int firstIndex;
    int iterationNumber;
} threadInitData;

int isNumber(char *numberStr)
{
    int index = 0;
    int retValue = IS_NUMBER;
    while (numberStr[index] != END_OF_C_LINE)
    {
        if (numberStr[index] < '0' || numberStr[index] > '9')
        {
            retValue = IS_NOT_NUMBER;
            break;
        }
        index += INDEX_STEP;
    }
    return retValue;
}

void printError(int errorCode, char *string)
{
    if (string == NULL)
    {
        return;
    }
    if (string != NULL && errorCode == PRINT_ERROR_STRING)
    {
        fprintf(stderr, "%s", string);
    }
    if (errorCode == CODE_IS_IN_ERRNO)
    {
        perror(string);
    }
}

arrayOfThreads *initialiseArrayOfThreads(int numberOfThreads)
{
    arrayOfThreads *array = (arrayOfThreads *)malloc(sizeof(arrayOfThreads));
    if (array == NULL)
    {
        printError(PRINT_ERROR_STRING, "arrayOfThreads* in function initialiseArrayOfThreads() is NULL\n");
        return array;
    }
    array->numberOfThreads = numberOfThreads;
    array->arrayOfThreads = (pthread_t *)malloc(sizeof(pthread_t) * array->numberOfThreads);
    if (array->arrayOfThreads == NULL)
    {
        printError(PRINT_ERROR_STRING, "pthread_t* in function initialiseArrayOfThreads() is NULL\n");
        return array;
    }
    return array;
}

void freeArrayOfThreads(arrayOfThreads *array)
{
    if (array == NULL)
    {
        printError(PRINT_ERROR_STRING, "arrayOfThreads* in function freeArrayOfThreads() is NULL\n");
        return;
    }
    if (array->arrayOfThreads == NULL)
    {
        printError(PRINT_ERROR_STRING, "pthread_t* in function freeArrayOfThreads() is NULL\n");
        free(array);
        return;
    }
    free(array->arrayOfThreads);
    free(array);
}

threadInitData *initialiseInitData(int numberOfIterations, int numberOfThreads)
{
    threadInitData *initData = (threadInitData *)malloc(sizeof(threadInitData) * numberOfThreads);
    int firstIndexTemp = 0;
    int numberOfThreadIterations = numberOfIterations / (numberOfThreads);
    int numberOfThreadIterationsLast = (numberOfIterations / (numberOfThreads)) + (numberOfIterations % (numberOfThreads));
    for (int i = 0; i < numberOfThreads - 1; i++)
    {
        (initData + i)->firstIndex = firstIndexTemp;
        (initData + i)->iterationNumber = numberOfThreadIterations;
        firstIndexTemp += numberOfThreadIterations;
    }
    (initData + numberOfThreads - 1)->firstIndex = firstIndexTemp;
    (initData + numberOfThreads - 1)->iterationNumber = numberOfThreadIterationsLast;
    return initData;
}

void freeInitData(threadInitData* data)
{
    if(data != NULL)
    {
        free(data);
    }
}

void cleanup_countPI_handler_1(void *doublePointer)
{
    if (doublePointer != NULL)
    {
        free(doublePointer);
    }
}

int getFirstDataIndex(threadInitData *initData)
{
    return initData->firstIndex;
}

int getDataLastBound(threadInitData *initData)
{
    return initData->firstIndex + initData->iterationNumber;
}

void *countPIThread(void *initData)
{
    double *localPIRetValue = (double *)malloc(sizeof(double));
    *localPIRetValue = 0;
    int localFirst = getFirstDataIndex((threadInitData *)initData);
    int localLast = getDataLastBound((threadInitData *)initData);
    for (int i = localFirst; i < localLast; i++)
    {
        (*localPIRetValue) += (1.0 / (i * 4.0 + 1.0));
        (*localPIRetValue) -= (1.0 / (i * 4.0 + 3.0));
    }
    pthread_exit((void *)(localPIRetValue));
}

double countPI(arrayOfThreads *arrayOfThreads, int numberOfIterations)
{
    double retDouble = DEF_RET_VALUE_OF_COUNTPI;
    if (arrayOfThreads == NULL || arrayOfThreads->arrayOfThreads == NULL)
    {
        printError(PRINT_ERROR_STRING, "Array of thread is null\n");
        return INVALID_PI;
    }
    threadInitData *initData = initialiseInitData(numberOfIterations, arrayOfThreads->numberOfThreads);
    for (int i = 0; i < arrayOfThreads->numberOfThreads; i++)
    {
        int retCode = pthread_create(arrayOfThreads->arrayOfThreads + i, NULL, countPIThread, initData + i);
        if (retCode != PTHREAD_CREATE_SUCCESS_VALUE)
        {
            printError(CODE_IS_IN_ERRNO, "Error in function pthread_create!\n");
            return INVALID_PI;
        }
    }
    for (int i = 0; i < arrayOfThreads->numberOfThreads; i++)
    {
        double *retDoubleThread;
        int retCode = pthread_join(*(arrayOfThreads->arrayOfThreads + i), (void **)(&retDoubleThread));
        if (retCode != PTHREAD_JOIN_SUCCESS_VALUE)
        {
            fprintf(stderr, "Thread number %d hasn't been joined. Code of error is %d\n", i, retCode);
            return INVALID_PI;
        }
        retDouble += *(retDoubleThread);
    }
    freeInitData(initData);
    return retDouble;
}

int main(int argc, char **argv)
{
    if (argc != RIGHT_NUMBER_OF_ARGS)
    {
        printError(PRINT_ERROR_STRING, "Wrong number of parameters in programm. Try again!\n");
        return 0;
    }
    if(isNumber(argv[ARGV_NUM_OF_THREADS_INDEX]) == IS_NOT_NUMBER)
    {
        printError(PRINT_ERROR_STRING, "Number of threads parameter is not number. Try again!\n");
        return 0;
    }
    arrayOfThreads *array = initialiseArrayOfThreads(atoi(argv[ARGV_NUM_OF_THREADS_INDEX]));
    double pi = countPI(array, NUMBER_OF_ITERATIONS);
    if(pi == INVALID_PI)
    {
        printError(PRINT_ERROR_STRING, "Error happened during execution of programm. Try again!\n");
        return 0;
    }
    pi *= 4;
    freeArrayOfThreads(array);
    fprintf(stdout, "\nPI number: %.15lf\n", pi);
    return 0;
}