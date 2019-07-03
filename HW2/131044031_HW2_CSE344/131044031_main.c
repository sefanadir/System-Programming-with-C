#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#define READ_MODE  1
#define WRITE_MODE 0
#define SIZE_OF_INT 4
#define NUMBERS_OF_ARGC 4
#define MINIMUM_ROW_NUMBER 1
#define NUMBER_OF_ROW_FILE "row.txt"

int recordNumberOfRow(int mod, int rowNumber);
int * createRandomArray(int * randomArray);
int readFile(const char *fileName, int *randomArray);
int addArrayToFile(const char *fileName, int countLine, int *randomArray);
char * fileName;
int fd_1, fd_2;
int * randomArray;
int sizeOfFile = 0;
int sizeOfArray = 0;
struct flock lock_1;
struct flock lock_2;
struct flock lock_3;

static void signalHandler(int getSignalNumber) {

    if (getSignalNumber == SIGINT) {

        perror("CTRL-C trapped system is shutting down");
        free(randomArray);
        close(fd_1);
        close(fd_2);

        remove(NUMBER_OF_ROW_FILE);
        while (wait(NULL) > 0);
        exit(0);
    }
}

int main(int argc, char** argv) {

    pid_t childProcessPid;
    struct sigaction act;
    srand(time(NULL));
    int countLine = 0;
    int loop = 0;

    act.sa_handler = signalHandler;
    act.sa_flags = 0;

    if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1)) {
        perror("Failed to set Signal handler");
        exit(1);
    }

    if (argc != NUMBERS_OF_ARGC) {
        printf("Usage: ./execute -N filename -M ");
    } else if (argc == NUMBERS_OF_ARGC) {

        sizeOfArray = atoi(argv[1]);
        fileName = argv[2];
        sizeOfFile = atoi(argv[3]);

        randomArray = (int *) malloc(sizeOfArray * sizeof (int));

        childProcessPid = fork();
        if (childProcessPid < 0) {
            perror("Stop Due to Fork Failure");

        } else if (childProcessPid == 0) {
            while (1) {

                readFile(fileName, randomArray);
                ++loop;
            }
        } else {
            while (1) {

                randomArray = createRandomArray(randomArray);
                countLine = addArrayToFile(fileName, countLine, randomArray);
                ++loop;
            }
            remove(NUMBER_OF_ROW_FILE);
            while (wait(NULL) > 0);
        }
    }
    return (EXIT_SUCCESS);
}

int * createRandomArray(int * randomArray) {
    int i = 0;
    printf("Process A: ");
    for (i = 0; i < sizeOfArray; ++i) {
        randomArray[i] = rand() % 10;
        printf("%d ", randomArray[i]);
    }
    printf("\n");
    return randomArray;
}

int addArrayToFile(const char *fileName, int countLine, int *randomArray) {
    int currentSize = countLine, b;

    if ((fd_1 = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0640)) == -1) {
        perror("Failed to open file");
        exit(0);
    } else {
        currentSize = recordNumberOfRow(READ_MODE, currentSize);
        if (currentSize < sizeOfFile) {
            lock_1.l_type = F_WRLCK;
            fcntl(fd_1, F_SETLKW, &lock_1);
            b = sizeOfArray * sizeof (int);

            write(fd_1, randomArray, b);
            close(fd_1);
            ++currentSize;
            recordNumberOfRow(WRITE_MODE, currentSize);

            lock_1.l_type = F_UNLCK;
            fcntl(fd_1, F_SETLKW, &lock_1);
        } else {
            printf("Capacity of file is full\n");
        }
    }
    return currentSize;
}

int readFile(const char *fileName, int *randomArray) {
    int i, bytesNumber;
    int rowNumber = 0, currentLocation = 0;

    if ((fd_1 = open(fileName, O_RDONLY | O_CREAT, 0640)) == -1) {
        perror("Failed to open file");
        exit(0);
    } else {
        rowNumber = recordNumberOfRow(READ_MODE, rowNumber);
        if (rowNumber >= MINIMUM_ROW_NUMBER && rowNumber < sizeOfFile) {
            lock_1.l_type = F_WRLCK;
            fcntl(fd_1, F_SETLKW, &lock_1);

            currentLocation = (rowNumber - 1) * sizeOfArray * SIZE_OF_INT;

            lseek(fd_1, currentLocation, SEEK_CUR);

            bytesNumber = sizeOfArray * SIZE_OF_INT;

            read(fd_1, randomArray, bytesNumber);
            --rowNumber;
            printf("Process B: ");
            for (i = 0; i < sizeOfArray; ++i) {
                printf("%d ", randomArray[i]);
            }
            printf("\n");

            close(fd_1);

            lock_1.l_type = F_UNLCK;
            fcntl(fd_1, F_SETLKW, &lock_1);

            recordNumberOfRow(WRITE_MODE, rowNumber);
        }
    }

    return 1;
}

int recordNumberOfRow(int mod, int rowNumber) {
    int readRowNumber = 0;
    /* write mod */
    if (mod == 0) {
        if ((fd_2 = open(NUMBER_OF_ROW_FILE, O_WRONLY | O_CREAT, 0640) == -1)) {
            perror("Failed to open file");
            exit(0);
        } else {
            lock_2.l_type = F_WRLCK;
            fcntl(fd_2, F_SETLKW, &lock_2);

            write(fd_2, &rowNumber, sizeof (int));
            close(fd_2);

            lock_2.l_type = F_UNLCK;
            fcntl(fd_2, F_SETLKW, &lock_2);
        }
    }/* read mod */
    else if (mod == 1) {

        if ((fd_2 = open(NUMBER_OF_ROW_FILE, O_RDONLY | O_CREAT, 0640)) == -1) {
            perror("Failed to open file");
            exit(0);
        } else {
            lock_2.l_type = F_WRLCK;
            fcntl(fd_2, F_SETLKW, &lock_2);

            read(fd_2, &readRowNumber, sizeof (int));
            close(fd_2);

            lock_2.l_type = F_UNLCK;
            fcntl(fd_2, F_SETLKW, &lock_2);
        }
    }
    return readRowNumber;
}
