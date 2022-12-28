#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define BUF_SIZE 4096
#define RETRY_SECONDS 5

#define EMPTY_ERRNO 0
#define CODE_IS_IN_ERRNO 1
#define PRINT_ERROR_STRING 0
#define OPEN_ERROR_CODE -1
#define NO_ERROR_OR_SUCCESS 1
#define STANDARD_SUCCESS 0
#define STANDARD_ERROR 1

#define CURRENT_FOLDER_PATH "."
#define PARENT_FOLDER_PATH ".."

typedef struct paths_t {
    char *src;
    char *dest;
} paths_t;

int isStringEmpty(const char* string)
{
    return (string == NULL || string[0] == '\0');
}

int isStringsEqual(const char* string1, const char* string2)
{
    return (strcmp(string1, string2) == 0);
}

void *copyPath(void *param);

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

int openFileRetry(char *path, int oflag, mode_t mode) {
    while (NO_ERROR_OR_SUCCESS) {
        int fd = open(path, oflag, mode);
        if (fd != OPEN_ERROR_CODE) {
            return fd;
        }
        if(errno != EMFILE)
        {
            printError(CODE_IS_IN_ERRNO, "Error. File can't be opened.\n");
            break;
        }
        sleep(RETRY_SECONDS);
    }
    return OPEN_ERROR_CODE;
}

DIR *openDirRetry(char *path) {
    while (NO_ERROR_OR_SUCCESS) {
        DIR *dir = opendir(path);
        if (dir != NULL) {
            return dir;
        }
        if (errno != EMFILE) {
            printError(CODE_IS_IN_ERRNO, "Error. Dir can't be opened.\n");
            break;
        }
        sleep(RETRY_SECONDS);
    }
    return NULL;
}

int createThreadRetry(void *param) {
    int retCode;
    pthread_t thread;
    while (NO_ERROR_OR_SUCCESS) {
        retCode = pthread_create(&thread, NULL, copyPath, param);
        if (retCode == STANDARD_SUCCESS) {
            retCode = pthread_detach(thread);
            if(retCode == STANDARD_SUCCESS)
            {
                return STANDARD_SUCCESS;
            }
            break;
        }
        if (retCode != EAGAIN) {
            printError(CODE_IS_IN_ERRNO, "Error. Unable to create thread.\n");
            break;
        }
        sleep(RETRY_SECONDS);
    }
    return STANDARD_ERROR;
}

void *mallocRetry(size_t size) {
    while (NO_ERROR_OR_SUCCESS) {
        void *ptr = malloc(size);
        if (ptr != NULL) {
            memset(ptr, 0, size);
            return ptr;
        }
        if (errno != EAGAIN) {
            printError(CODE_IS_IN_ERRNO, "Error. Unable to allocate memory.\n");
            break;
        }
        sleep(RETRY_SECONDS);
    }
    return NULL;
}

char *concatStringArray(const char **strings) {
    if (strings == NULL) {
        printError(PRINT_ERROR_STRING, "Error. Strings Array is null.\n");
        return NULL;
    }

    size_t arrayLength = 0;
    for (int i = 0; strings[i] != NULL; i++) {
        arrayLength += strlen(strings[i]);
    }

    char *str = (char *)mallocRetry((arrayLength + 1) * sizeof(char));

    if (str != NULL) {
        for (int i = 0; strings[i] != NULL; i++) {
            strcat(str, strings[i]);
        }
    }

    return str;
}

void freePaths(paths_t *paths) {
    if (paths == NULL) {
        return;
    }
    free(paths->src);
    free(paths->dest);
    free(paths);
}

paths_t *createPath(const char *srcPath, const char *destPath, const char *subPath) {
    if (srcPath == NULL || destPath == NULL) {
        printError(PRINT_ERROR_STRING, "Error. sourcePath or/and destPath is NULL.\n");
        return NULL;
    }

    paths_t *paths = (paths_t*)mallocRetry(sizeof(paths_t));
    if (paths == NULL) {
        return NULL;
    }

    char *delimiter = isStringEmpty(subPath) ? "" : "/";
	subPath = isStringEmpty(subPath) ? "" : subPath;

    paths->src = concatStringArray((const char *[]){ srcPath, delimiter, subPath, NULL });
    paths->dest = concatStringArray((const char *[]){ destPath, delimiter, subPath, NULL });

    if (paths->src == NULL || paths->dest == NULL) {
        freePaths(paths);
        return NULL;
    }

    return paths;
}

void traverseDirectory(DIR *dir, paths_t *oldPath) {
    if (oldPath == NULL) {
        printError(PRINT_ERROR_STRING, "Error. traverseDirectory: old_paths is null\n");
        return;
    }

    struct dirent *result;

    while (NO_ERROR_OR_SUCCESS) {
        errno = EMPTY_ERRNO;
        result = readdir(dir);
        if (result == NULL && errno == EMPTY_ERRNO) {
            break;
        }

        if(result == NULL && errno != EMPTY_ERRNO){
            printError(CODE_IS_IN_ERRNO, "Error. TraverseDirectoryError.\n");
            return;
        }

        if (isStringsEqual(result->d_name, CURRENT_FOLDER_PATH) || isStringsEqual(result->d_name, PARENT_FOLDER_PATH)) {
            continue;
        }

        paths_t *newPath = createPath(oldPath->src, oldPath->dest, result->d_name);
        if (newPath == NULL) {
            continue;
        }

        if (createThreadRetry(newPath) == STANDARD_ERROR) {
            freePaths(newPath);
            continue;
        }
    }
}

void copyDir(paths_t *path, mode_t mode) {
    if (path == NULL || path->src == NULL || path->dest == NULL) {
        printError(PRINT_ERROR_STRING, "Error. copyDir: path is NULL\n");
        return;
    }

    errno = 0;  //errno is unique to calling thread
    if (mkdir(path->dest, mode) == OPEN_ERROR_CODE && errno != EEXIST) {
        printError(CODE_IS_IN_ERRNO, "Error. Can't create dir with some path\n");
        return;
    }

    DIR *dir = openDirRetry(path->src);
    if (dir == NULL) {
        return;
    }

    traverseDirectory(dir, path);

    if (closedir(dir) == OPEN_ERROR_CODE) {
        printError(CODE_IS_IN_ERRNO, "Error. Can't close directory.\n");
        return;
    }
}

void copyData(int srcFD, int destFD, paths_t *path) {
    if (path == NULL) {
        printError(PRINT_ERROR_STRING, "Error. copyData: path is null\n");
        return;
    }

    char buf[BUF_SIZE];
    while (NO_ERROR_OR_SUCCESS) {
        ssize_t rBytesNumber = read(srcFD, buf, BUF_SIZE);
        if (rBytesNumber == OPEN_ERROR_CODE) {
            printError(CODE_IS_IN_ERRNO, path->src);
            return;
        }
        if (rBytesNumber == 0) {
            break;
        }

        ssize_t offset = 0;
        ssize_t wBytesNumber = 0;
        while (offset < rBytesNumber) {
            wBytesNumber = write(destFD, buf + offset, rBytesNumber - offset);
            if (wBytesNumber == OPEN_ERROR_CODE) {
                printError(CODE_IS_IN_ERRNO, path->dest);
                return;
            }
            offset += wBytesNumber;
        }
    }
}

void copyFile(paths_t *paths, mode_t mode) {
    if (paths == NULL || paths->src == NULL || paths->dest == NULL) {
        printError(PRINT_ERROR_STRING, "Error. copyFile: paths is NULL\n");
        return;
    }

    int srcFD = openFileRetry(paths->src, O_RDONLY, mode);
    if (srcFD == OPEN_ERROR_CODE) {
        return;
    }

    int destFD = openFileRetry(paths->dest, O_WRONLY | O_CREAT | O_EXCL, mode);
    if (destFD == OPEN_ERROR_CODE) {
        close(srcFD);
        return;
    }

    copyData(srcFD, destFD, paths);

    if (close(srcFD) == OPEN_ERROR_CODE) {
        printError(PRINT_ERROR_STRING, paths->src);
    }
    if (close(destFD) == OPEN_ERROR_CODE) {
        printError(PRINT_ERROR_STRING, paths->dest);
    }
}

void *copyPath(void *param) {
    if (param == NULL) {
        printError(PRINT_ERROR_STRING, "copyPath: param is null\n");
        return NULL;
    }

	paths_t *path = (paths_t *)param;
    struct stat stat_buf;
	
    if (lstat(path->src, &stat_buf) == OPEN_ERROR_CODE) {
        printError(CODE_IS_IN_ERRNO, path->src);
        freePaths(path);
        return NULL;
    }

    if (S_ISDIR(stat_buf.st_mode)) {
        copyDir(path, stat_buf.st_mode);
    }
    if (S_ISREG(stat_buf.st_mode)) {
        copyFile(path, stat_buf.st_mode);
    }

    freePaths(path);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printError(PRINT_ERROR_STRING, "Usage: [srcPath], [destPath]. Number of arguments isn't correct. Try again!\n");
        return EXIT_FAILURE;
    }

    paths_t *paths = createPath(argv[1], argv[2], "");
    if (paths == NULL) {
        printError(PRINT_ERROR_STRING, "Usage: [srcPath], [destPath]. Invalid srcPath or destPath. Try again!\n");
        return EXIT_FAILURE;
    }
    copyPath(paths);
    pthread_exit(NULL);
}
