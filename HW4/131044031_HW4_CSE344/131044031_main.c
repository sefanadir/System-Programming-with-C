#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>  
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#define SIZE 6
#define CHEFS 6
#define ARGC_NUMBER 1

typedef struct chefIngredients {
    char ingredients_1[SIZE];
    char ingredients_2[SIZE];
} chef;

typedef struct salerIngredients {
    char ingredients_1[SIZE];
    char ingredients_2[SIZE];
} wholesaler;

chef products;
sem_t *start;
sem_t *between;
sem_t *end;
wholesaler wholeProducts;
wholesaler *productSM;
int sharedmemId = 0;
static int flag = 1;
int uniqueNumber = 0;
int createRandomNumber();
void createRandomSellingProducts();
static void signalHandler(int getSignalNumber);

int main(int argc, char** argv) {
    int i = 0;
    int semValue = 100;
    pid_t childProcessPid;
    struct sigaction act = {
        {0}
    };
    act.sa_handler = signalHandler;
    act.sa_flags = 0;
    if (argc != ARGC_NUMBER) {
        fprintf(stderr, "usage: ./execute \n");
    } else {

        if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1) || (sigaction(SIGTERM, &act, NULL) == -1)) {
            perror("Failed to set Signal handler");
            exit(1);
        }

        sharedmemId = shmget(IPC_PRIVATE, sizeof (wholesaler), IPC_CREAT | 0666);
        if (sharedmemId < 0) {
            fprintf(stderr, "Stop Due to Shared Memory Failure\n");
            exit(0);
        }
        productSM = (wholesaler*) shmat(sharedmemId, (void*) 0, 0);

        start = sem_open("start", O_CREAT | O_EXCL, 0644, semValue);
        sem_unlink("start");

        between = sem_open("between", O_CREAT | O_EXCL, 0644, semValue);
        sem_unlink("between");

        end = sem_open("end", O_CREAT | O_EXCL, 0644, semValue);
        sem_unlink("end");


        createRandomSellingProducts();
        for (i = 1; i <= CHEFS; ++i) {

            childProcessPid = fork();
            if (childProcessPid < 0) {
                fprintf(stderr, "Stop Due to Fork Failure\n");
                exit(0);
            } else if (childProcessPid == 0) {
                if (i == 1) {
                    strcpy(products.ingredients_1, "eggs");
                    strcpy(products.ingredients_2, "sugar");
                    fprintf(stderr, "chef %d is waiting for eggs and sugar\n", i);
                } else if (i == 2) {
                    strcpy(products.ingredients_1, "eggs");
                    strcpy(products.ingredients_2, "flour");
                    fprintf(stderr, "chef %d is waiting for eggs and flour\n", i);
                } else if (i == 3) {
                    strcpy(products.ingredients_1, "eggs");
                    strcpy(products.ingredients_2, "butter");
                    fprintf(stderr, "chef %d is waiting for eggs and butter\n", i);
                } else if (i == 4) {
                    strcpy(products.ingredients_1, "flour");
                    strcpy(products.ingredients_2, "butter");
                    fprintf(stderr, "chef %d is waiting for flour and butter\n", i);
                } else if (i == 5) {
                    strcpy(products.ingredients_1, "flour");
                    strcpy(products.ingredients_2, "sugar");
                    fprintf(stderr, "chef %d is waiting for flour and sugar\n", i);
                } else if (i == 6) {
                    strcpy(products.ingredients_1, "butter");
                    strcpy(products.ingredients_2, "sugar");
                    fprintf(stderr, "chef %d is waiting for butter and sugar\n", i);
                }
                break;
            }
        }
        while (flag == 1) {
            if (childProcessPid > 0) {
                sem_wait(start);
                sem_wait(between);

                createRandomSellingProducts();

                sem_post(between);
                sem_post(end);
            } else if (childProcessPid == 0) {
                sem_wait(end);
                sem_wait(between);

                if (
                        (strcmp(productSM->ingredients_1, products.ingredients_1) == 0 &&
                        strcmp(productSM->ingredients_2, products.ingredients_2) == 0) ||
                        (strcmp(productSM->ingredients_2, products.ingredients_1) == 0 &&
                        strcmp(productSM->ingredients_1, products.ingredients_2) == 0)) {
                    fprintf(stderr, "wholesaler delivers: %s and %s\n", productSM->ingredients_1, productSM->ingredients_2);
                    fprintf(stderr, "chef %d has taken: %s\n", i, productSM->ingredients_1);
                    fprintf(stderr, "wholesaler is waiting for the dessert\n");
                    fprintf(stderr, "chef %d has taken: %s\n", i, productSM->ingredients_2);
                    fprintf(stderr, "chef %d is preparing the dessert\n", i);
                    fprintf(stderr, "chef %d has delivered the dessert to the wholesaler\n", i);
                    fprintf(stderr, "wholesaler has obtained the dessert and left to sell it\n");
                    fprintf(stderr, "chef %d is waiting %s and %s\n", i, products.ingredients_1, products.ingredients_2);
                }
                sem_post(between);
                sem_post(start);
            }
        }
    }
    return (EXIT_SUCCESS);
}

int createRandomNumber() {
    int number = (rand() % 6 + 1) + uniqueNumber;
    number = number % 6 + 1;
    ++uniqueNumber;
    if (uniqueNumber == 100)
        uniqueNumber = 0;
    return number;
}

void createRandomSellingProducts() {
    switch (createRandomNumber()) {
        case 1:
            strcpy(wholeProducts.ingredients_1, "eggs");
            strcpy(wholeProducts.ingredients_2, "sugar");
            break;
        case 2:
            strcpy(wholeProducts.ingredients_1, "eggs");
            strcpy(wholeProducts.ingredients_2, "butter");
            break;
        case 3:
            strcpy(wholeProducts.ingredients_1, "eggs");
            strcpy(wholeProducts.ingredients_2, "flour");
            break;
        case 4:
            strcpy(wholeProducts.ingredients_1, "sugar");
            strcpy(wholeProducts.ingredients_2, "butter");
            break;
        case 5:
            strcpy(wholeProducts.ingredients_1, "flour");
            strcpy(wholeProducts.ingredients_2, "butter");
            break;
        case 6:
            strcpy(wholeProducts.ingredients_1, "sugar");
            strcpy(wholeProducts.ingredients_2, "flour");
            break;
    }
    strcpy(productSM->ingredients_1, wholeProducts.ingredients_1);
    strcpy(productSM->ingredients_2, wholeProducts.ingredients_2);
}

static void signalHandler(int getSignalNumber) {
    pid_t wpid = 0;
    int status = 0;
    if (getSignalNumber == SIGTERM) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");
        flag = 0;
        shmdt(productSM);
        shmctl(sharedmemId, IPC_RMID, NULL);
        sem_destroy(start);
        sem_destroy(between);
        sem_destroy(end);
        while ((wpid = wait(&status)) > 0);
        exit(0);
    } else if (getSignalNumber == SIGINT) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");
        flag = 0;
        shmdt(productSM);
        shmctl(sharedmemId, IPC_RMID, NULL);
        sem_destroy(start);
        sem_destroy(between);
        sem_destroy(end);
        while ((wpid = wait(&status)) > 0);
        exit(0);
    }
}
