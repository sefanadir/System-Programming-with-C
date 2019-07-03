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
#include <stdlib.h>
#define ARGC_NUMBER 2
int cat_command(const char* fileName);
static void signalHandler(int getSignalNumber);
FILE *file=NULL;
int main(int argc, char** argv) {
    struct sigaction act={{0}};
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
        cat_command(argv[0]);
    }
    
    return 0;
}

int cat_command(const char* fileName) {
    char character=' ';
    
    file = fopen(fileName, "r");

    if (file == NULL) {
        printf("Unable to open file\n");
        return 0;
    }
    character = fgetc(file);
    while (character != EOF) {
        if (character != '\n') {
            fprintf(stdout, "%c", character);
        } else if (character == '\n') {
            fprintf(stdout, "\n");
        }
        character = fgetc(file);
    }
    fprintf(stdout, "\n");
    fclose(file);
    return 1;
}
static void signalHandler(int getSignalNumber) {
    if (getSignalNumber == SIGINT) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");
        fclose(file);
        exit(0);
    }
}
