#define _GNU_SOURCE
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
#define ARGC_NUMBER 1
#define COMMAND_SIZE 100

void exit_command();
int pwd_command();
int help_command();
int cd_command(const char * goToDir);
void run_shell();
void run_command(char commandList[][COMMAND_SIZE], int commandIterator);
static void signalHandler(int getSignalNumber);
void createExePaths();
void forkExec(char * exeName);
char directory[1024];
char saveDirectory[1024];
char ls_path[1024];
char wc_path[1024];
char cat_path[1024];
char *parameter[2];
pid_t childProcessPid;
int main(int argc, char** argv) {
    struct sigaction act={{0}};
    act.sa_handler = signalHandler;
    act.sa_flags = 0;

    if (argc != ARGC_NUMBER) {
        fprintf(stderr, "Usage: ./executionName ./executionNamecat ./executionNamels ./executionNamewc");
    } else {
        if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1)) {
            perror("Failed to set Signal handler");
            exit(1);
        }
        pwd_command();
        createExePaths();
        run_shell();
    }
    return 0;
}


static void signalHandler(int getSignalNumber) {
    if (getSignalNumber == SIGINT) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");
        while (wait(NULL) > 0);
        exit(0);
    }
}

void run_shell() {
    char *command=NULL;
    char *commandToken=NULL;
    int commandIterator = 0;
    int listSize = COMMAND_SIZE - 1;
    size_t commandSize = COMMAND_SIZE;
    char commandList[COMMAND_SIZE][COMMAND_SIZE];
    const char delimeter[3] = {' ', '\t', '\n'};

    while (1) {
        printf("-> ");
        getline(&command, &commandSize, stdin);
        commandIterator = listSize;
        commandToken = strtok(command, delimeter);

        while (commandToken != NULL) {
            strcpy(commandList[listSize], commandToken);
            --listSize;
            if (listSize < 0) {
                listSize = COMMAND_SIZE - 1;
            }
            commandToken = strtok(NULL, delimeter);
        }
        run_command(commandList, commandIterator);
    }
}

void run_command(char commandList[][COMMAND_SIZE], int commandIterator) {

    if (strcmp(commandList[commandIterator], "ls") == 0) {
        parameter[0]=commandList[commandIterator];
        parameter[1]=&directory[0];
        forkExec(ls_path);
    } else if (strcmp(commandList[commandIterator], "pwd") == 0) {
        pwd_command();
    } else if (strcmp(commandList[commandIterator], "cd") == 0) {
        cd_command(commandList[commandIterator - 1]);
        pwd_command();
    } else if (strcmp(commandList[commandIterator], "help") == 0) {
        help_command();
    } else if (strcmp(commandList[commandIterator], "cat") == 0) {
        parameter[0]=commandList[commandIterator-1];
        parameter[1]=&directory[0];
        forkExec(cat_path);
    } else if (strcmp(commandList[commandIterator], "wc") == 0) {
        parameter[0]=commandList[commandIterator-1];
        parameter[1]=&directory[0];
        forkExec(wc_path);
    } else if (strcmp(commandList[commandIterator], "exit") == 0) {
        exit_command();
    }
}

void exit_command() {
    while (wait(NULL) > 0);
    exit(1);
}

int pwd_command() {
    if (getcwd(directory, sizeof (directory)) == NULL) {
        fprintf(stderr,"error pwd");
        return 0;
    } else {
        fprintf(stdout, "%s\n", directory);
        return 1;
    }
    return 0;
}

int cd_command(const char * goToDir) {
    int changeDir = 0;
    changeDir = chdir(goToDir);
    if (changeDir != 0) {
        fprintf(stderr, "Directory not changed\n");
        return 0;
    }
    return 1;
}

int help_command() {
    fprintf(stdout, "ls  : lists directory contents of files and directories.\n");
    fprintf(stdout, "pwd : finds the full path to the current directory.\n");
    fprintf(stdout, "cd  : changes the shell's current working directory.\n");
    fprintf(stdout, "cat : which will print on standard output the contents of the file provided to it as argument\n");
    fprintf(stdout, "wc  : which will print on standard output the number of lines in the file provided to it as argument\n");
    fprintf(stdout, "exit: which will exit the shell\n");
    return 1;
}
void createExePaths(){
    getcwd(saveDirectory, sizeof (saveDirectory));
    strcpy(ls_path,saveDirectory);
    strcat(ls_path,"/");
    strcat(ls_path,"ls_exe");

    strcpy(wc_path,saveDirectory);
    strcat(wc_path,"/");
    strcat(wc_path,"wc_exe");

    strcpy(cat_path,saveDirectory);
    strcat(cat_path,"/");
    strcat(cat_path,"cat_exe");
}
void forkExec(char * exeName){
    childProcessPid = fork();
    if (childProcessPid < 0) {
        printf("Stop Due to Fork Failure");

    } else if (childProcessPid == 0) {
        execv(exeName,parameter);
        exit(0);

    } else {
        while (wait(NULL) > 0);
    }
}