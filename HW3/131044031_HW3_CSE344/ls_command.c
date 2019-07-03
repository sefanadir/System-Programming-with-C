#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#define PATH_SIZE 1024
#define ARGC_NUMBER 2
void ls_command(const char *searchWay);
int checkDirectory(const char *path);
char * determineFileType(const char *fileName);
static void signalHandler(int getSignalNumber);
char fileType[512];
DIR *dirPath=NULL;

int main(int argc, char** argv) {
    struct sigaction act= {{0}};
    act.sa_handler = signalHandler;
    act.sa_flags = 0;

    if (argc != ARGC_NUMBER) {
        fprintf(stderr, "Usage: invalid");
    } else {
        if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1)) {
            perror("Failed to set Signal handler");
            exit(1);
        }
        chdir(argv[1]);
        ls_command(argv[1]);
    }
    return 0;
}

void ls_command(const char *searchWay) {

    struct dirent *direntPath=NULL;
    struct stat buffer= {0};
    pid_t childProcessPid=0;
    int size = 0;
    char currentWay[PATH_SIZE];
    dirPath = opendir(searchWay);

    if (dirPath == NULL) {
        printf("Cannot open directory\n");
        exit(0);
    }
    strcpy(currentWay, searchWay);
    while ((direntPath = readdir(dirPath)) != NULL) {

        size = strlen(direntPath->d_name);

        if (strcmp(direntPath->d_name, ".") != 0 && strcmp(direntPath->d_name, "..") != 0
                && direntPath->d_name[size - 1] != '~') {
            strcpy(currentWay, searchWay);
            strcat(currentWay, "/");
            strcat(currentWay, direntPath->d_name);
        }

        if (strcmp(direntPath->d_name, ".") != 0 && strcmp(direntPath->d_name, "..") != 0 &&
                checkDirectory(currentWay) && direntPath->d_name[size - 1] != '~') {

            childProcessPid = fork();

            if (childProcessPid < 0) {
                printf("Stop Due to Fork Failure");

            } else if (childProcessPid == 0) {
                closedir(dirPath);
                fprintf(stderr, "dir name: %s dir type: %d dir size: %lu\n", direntPath->d_name, direntPath->d_type, direntPath->d_off);

                ls_command(currentWay);

                exit(0);

            } else {
                while (wait(NULL) > 0);
            }
        } else if (strcmp(direntPath->d_name, ".") != 0 && strcmp(direntPath->d_name, "..") != 0 &&
                !checkDirectory(currentWay) && direntPath->d_name[size - 1] != '~') {
           
            stat(currentWay, &buffer);
            childProcessPid = fork();

            if (childProcessPid < 0) {
                printf("Stop Due to Fork Failure");
            } else if (childProcessPid == 0) {

                fprintf(stderr, "file name: %s file size: %lu  file type: %s", direntPath->d_name, 
                                            buffer.st_size, determineFileType(direntPath->d_name));
                closedir(dirPath);
                exit(0);
            } else {
                while (wait(NULL) > 0);
            }
        }
    }
    closedir(dirPath);
}

int checkDirectory(const char *path) {
    struct stat buffer={0};
    if (stat(path, &buffer) == -1)
        return 0;
    else
        return S_ISDIR(buffer.st_mode);
}

char * determineFileType(const char *fileName) {
    struct stat buffer={0};
    if (stat(fileName, &buffer) != -1) {
        if (S_ISREG(buffer.st_mode))
            sprintf(fileType, "%s", "regular\n");
        else if (S_ISDIR(buffer.st_mode))
            sprintf(fileType, "%s", "directory\n");
        else if (S_ISFIFO(buffer.st_mode))
            sprintf(fileType, "%s", "fifo");
        else if (S_ISLNK(buffer.st_mode))
            sprintf(fileType, "%s", "link");
    }
    return fileType;
}
static void signalHandler(int getSignalNumber) {
    if (getSignalNumber == SIGINT) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");
        closedir(dirPath);
        exit(0);
    }
}