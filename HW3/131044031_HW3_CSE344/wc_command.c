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
#define ARGC_NUMBER 2
int wc_command(const char* fileName);
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
        wc_command(argv[0]);
    }
    return 0;
}
int wc_command(const char* fileName) {
    int numberOfRow = 1;
    char character;

    file = fopen(fileName, "r");

    if (file == NULL) {

        printf("Unable to open file\n");
        return 0;
    }
    character = fgetc(file);
    for (;character != EOF;) {
        if (character == '\n') {
            ++numberOfRow;
        }
        character = fgetc(file);
    }
    fclose(file);
    fprintf(stdout, "%d\n", numberOfRow);
    return numberOfRow;
}
static void signalHandler(int getSignalNumber) {
    if (getSignalNumber == SIGINT) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");
        fclose(file);
        while (wait(NULL) > 0);
        exit(0);
    }
}