#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define N 4
#define NUMBER_OF_STRING 10
#define LENGTH_OF_STRING_WITH_NULL 5
#define ASCII_ZERO_CODE (int)'0';

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

arrayOfString *initialiseArrayOfString()
{
    arrayOfString *array = (arrayOfString *)malloc(sizeof(arrayOfString) * N);
    if (array != 0)
    {
        for (int i = 0; i < N; i++)
        {
            array[i].numberOfString = NUMBER_OF_STRING;
            array[i].arrayOfString = (char **)malloc(sizeof(char *) * array[i].numberOfString);
            if (array[i].arrayOfString != 0)
            {
                for (int j = 0; j < array[i].numberOfString; j++)
                {
                    array[i].arrayOfString[j] = (char *)malloc(sizeof(char) * LENGTH_OF_STRING_WITH_NULL);
                    if (array[i].arrayOfString[j] != 0)
                    {
                        for (int k = 0; k < LENGTH_OF_STRING_WITH_NULL - 1; k++)
                        {
                            array[i].arrayOfString[j][k] = (char)j + k + ASCII_ZERO_CODE;
                        }
                        array[i].arrayOfString[j][LENGTH_OF_STRING_WITH_NULL - 1] = '\0';
                    }
                    else
                    {
                        fprintf(stderr, "array[i].arryOfString[j] in function initialiseArrayOfString() is NULL\n");
                    }
                }
            }
            else
            {
                fprintf(stderr, "array[i].arryOfString in function initialiseArrayOfString() is NULL\n");
            }
        }
    }
    else
    {
        fprintf(stderr, "arrayOfString* array in function initialiseArrayOfString() is NULL\n");
    }
    return array;
}

void freeArrayOfString(arrayOfString *array)
{
    if (array != 0)
    {
        for (int i = 0; i < N; i++)
        {
            if (array[i].arrayOfString != 0)
            {
                for (int j = 0; j < array[i].numberOfString; j++)
                {
                    if (array[i].arrayOfString[j] != 0)
                    {
                        free(array[i].arrayOfString[j]);
                    }
                    else
                    {
                        fprintf(stderr, "array[i].arrayOfString[j] in function freeArrayOfString() is NULL\n");
                    }
                }
                free(array[i].arrayOfString);
            }
            else
            {
                fprintf(stderr, "array[i].arrayOfString in function freeArrayOfString() is NULL\n");
            }
        }
        free(array);
    }
    else
    {
        fprintf(stderr, "array in function freeArrayOfString() is NULL\n");
    }
}

arrayOfThreads *initialiseArrayOfThreads()
{
    arrayOfThreads *array = (arrayOfThreads *)malloc(sizeof(arrayOfThreads));
    if (array != 0)
    {
        array->numberOfThreads = N;
        array->arrayOfThreads = (pthread_t *)malloc(sizeof(pthread_t) * array->numberOfThreads);
    }
    else
    {
        fprintf(stderr, "array in function initialiseArrayOfThreads() is NULL\n");
    }
    return array;
}

void freeArrayOfThreads(arrayOfThreads *array)
{
    if (array != 0)
    {
        if (array->arrayOfThreads != 0)
        {
            free(array->arrayOfThreads);
        }else
        {
            fprintf(stderr, "array->arrayOfThreads in function freeArrayOfThreads() is NULL\n");
        }
        free(array);
    }
    else
    {
        fprintf(stderr, "array in function freeArrayOfThreads() is NULL\n");
    }
}

void printError(char *string)
{
    if (string)
    {
        fprintf(stderr, "%s\n", string);
    }
}

void *printArrayOfString(void *structure)
{
    arrayOfString *array = (arrayOfString *)structure;
    for (int i = 0; i < array->numberOfString; i++)
    {
        fprintf(stdout, "%s\n", *(array->arrayOfString + i));
    }
}

int initialiseThreads(arrayOfThreads *arrayOfThreads, arrayOfString *arrayOfString)
{
    int ret = 0;
    if (arrayOfThreads && arrayOfString)
    {
        for (int i = 0; i < arrayOfThreads->numberOfThreads; i++)
        {
            int retCode = pthread_create(arrayOfThreads->arrayOfThreads + i , NULL, printArrayOfString, arrayOfString + i );
            if (retCode > 0)
            {
                fprintf(stderr, "Thread number %d has terminated with code %d\n", i, retCode);
            }
            else
            {
                ret += 1;
            }
        }
        for (int i = 0; i < arrayOfThreads->numberOfThreads; i++)
        {
            int retCode = pthread_join(*(arrayOfThreads->arrayOfThreads + i ), NULL);
            if(retCode > 0)
            {
                fprintf(stderr, "Thread number %d hasn't been joined. Code of error is %d\n", i, retCode);
            }
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
        printError("Error in function initializeThreads: not all threads were initialised succesfully\n");
    }
    freeArrayOfString(arrayOfString);
    freeArrayOfThreads(arrayOfThreads);
    exit(0);
}