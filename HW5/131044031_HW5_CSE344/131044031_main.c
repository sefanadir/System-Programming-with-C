#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <signal.h>
#define SIZE 50
#define FLORIST_SIZE 3
#define CLIENTS_SIZE 100
#define FILENAME "data.dat"
#define ARGC_NUMBER 2
typedef struct florist_type {
    char name[SIZE];
    int coordinate_x;
    int coordinate_y;
    double average_speed;
    char flower_1[SIZE];
    char flower_2[SIZE];
    char flower_3[SIZE];
    int number_of_sales;
    double total_time;
} florist;

typedef struct client_type {
    int coordinate_x;
    int coordinate_y;
    char flower[SIZE];
} client;

typedef struct distance_type {
    double data;
    int florist_no;
} distance;

void* processingRequests(void *arg);
double randomAmountOfTime(double ms);
void sortDistance(distance * distances);
static void signalHandler(int getSignalNumber);
int containsFlower(int florist_no, int client_no);
double calculateDistance(int x_1, int y_1, int x_2, int y_2);
void setClientInformation(client *one_client, int info_order, char *line);
void setFloristInformation(florist *one_florist, int info_order, char *line);
void readInformationFromFile(const char * fileName, florist * florists_array, client * clients_array);
/*-------------------------------*/

FILE * florist_file;
pthread_cond_t cv;
pthread_mutex_t lock;

static int counter = 0;
static int time_counter = 0;
static int number_of_client = 0;
pthread_t client_td[CLIENTS_SIZE];

florist florists_array[FLORIST_SIZE];
client clients_array[CLIENTS_SIZE];

/*-------------------------------*/
int main(int argc, char** argv) {
    int i = 0, thread_error;
    if (argc != ARGC_NUMBER) {
        fprintf(stdout, "usage: floristApp data.dat\n");
        exit(0);
    }
    struct sigaction act = {
        {0}
    };
    act.sa_handler = signalHandler;
    act.sa_flags = 0;
    if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1) || (sigaction(SIGTERM, &act, NULL) == -1)) {
        perror("Failed to set Signal handler");
        exit(1);
    }
    for (i = 0; i < FLORIST_SIZE; ++i) {
        florists_array[i].number_of_sales = 0;
        florists_array[i].total_time = 0.0;
    }
    readInformationFromFile(argv[1], florists_array, clients_array);
    fprintf(stdout, "Florist application initializing from file: %s\n", argv[1]);
    fprintf(stdout, "3 florists have been created\n");
    fprintf(stdout, "Processing requests\n");

    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "Mutex has failed\n");
        exit(0);
    }
    for (i = 0; i < number_of_client; ++i) {
        thread_error = pthread_create(&(client_td[i]), NULL, &processingRequests, NULL);
        if (thread_error != 0)
            fprintf(stderr, "Thread can't be created :[%s]", strerror(thread_error));
    }
    for (i = 0; i < 100000000; ++i);
    for (i = 0; i < number_of_client; ++i) {
        pthread_cond_signal(&cv);
    }
    for (i = 0; i < number_of_client; ++i) {
        pthread_join(client_td[i], NULL);
    }
    fprintf(stdout, "All requests processed.\n");
    fprintf(stdout, "%s closing shop.\n", florists_array[0].name);
    fprintf(stdout, "%s closing shop.\n", florists_array[1].name);
    fprintf(stdout, "%s closing shop.\n", florists_array[2].name);
    fprintf(stdout, "Sale statistics for today :\n");
    fprintf(stdout, "-------------------------------------------------\n");
    fprintf(stdout, "Florist          # of sales      Total time\n");
    for (i = 0; i < FLORIST_SIZE; ++i) {
        fprintf(stdout, "%s             %d               %.3f\n", florists_array[i].name, florists_array[i].number_of_sales, florists_array[i].total_time);
    }
    fprintf(stdout, "-------------------------------------------------\n");
    pthread_mutex_destroy(&lock);
    return (EXIT_SUCCESS);
}

void* processingRequests(void *arg) {
    int i = 0;
    distance d[3];
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&cv, &lock);
    counter += 1;
    d[0].data = calculateDistance(florists_array[0].coordinate_x, florists_array[0].coordinate_y,
            clients_array[counter - 1].coordinate_x, clients_array[counter - 1].coordinate_y);
    d[0].florist_no = 0;

    d[1].data = calculateDistance(florists_array[1].coordinate_x, florists_array[1].coordinate_y,
            clients_array[counter - 1].coordinate_x, clients_array[counter - 1].coordinate_y);
    d[1].florist_no = 1;

    d[2].data = calculateDistance(florists_array[2].coordinate_x, florists_array[2].coordinate_y,
            clients_array[counter - 1].coordinate_x, clients_array[counter - 1].coordinate_y);
    d[2].florist_no = 2;
    sortDistance(d);
    for (i = 0; i < FLORIST_SIZE; i++) {
        if ((containsFlower(d[i].florist_no, counter - 1) == 1)) {

            fprintf(stdout, "Florist %s has delivered a %s to client%d in %.2f\n", florists_array[i].name, clients_array[counter - 1].flower, counter, randomAmountOfTime(florists_array[d[i].florist_no].average_speed));
            florists_array[i].total_time += randomAmountOfTime(florists_array[d[i].florist_no].average_speed);
            florists_array[i].number_of_sales += 1;

            break;
        }
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

static void signalHandler(int getSignalNumber) {
    int i = 0;
    if (getSignalNumber == SIGTERM) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");

        fclose(florist_file);
        for (i = 0; i < number_of_client; ++i) {
            pthread_join(client_td[i], NULL);
        }
        pthread_mutex_destroy(&lock);
        exit(0);
    } else if (getSignalNumber == SIGINT) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");

        fclose(florist_file);
        for (i = 0; i < number_of_client; ++i) {
            pthread_join(client_td[i], NULL);
        }
        pthread_mutex_destroy(&lock);
        exit(0);
    }
}

void readInformationFromFile(const char * fileName, florist * florists_array, client * clients_array) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;
    int info_order = 1;
    char *token = NULL;
    const char delimeters[8] = "( ,;:)\n";
    florist_file = fopen(fileName, "r");
    if (florist_file == NULL) {
        perror("Failed to open file");
        exit(0);
    }
    while ((read = getline(&line, &len, florist_file)) != -1 && count < FLORIST_SIZE) {
        if (strcmp(line, "") != 0) {
            token = strtok(line, delimeters);
            strcpy(florists_array[count].name, token);
            token = strtok(NULL, delimeters);
            info_order = 1;
            while (token != NULL) {

                setFloristInformation(&florists_array[count], info_order, token);
                token = strtok(NULL, delimeters);
                ++info_order;
            }
            ++count;
        }
    }
    count = 0;
    while ((read = getline(&line, &len, florist_file)) != -1) {
        ++number_of_client;
        if (strcmp(line, "") != 0) {
            token = strtok(line, delimeters);
            token = strtok(NULL, delimeters);
            info_order = 1;
            while (token != NULL) {

                setClientInformation(&clients_array[count], info_order, token);
                token = strtok(NULL, delimeters);
                ++info_order;
            }
        }
        ++count;
    }
    fclose(florist_file);
}

void sortDistance(distance * distances) {
    int i = 0, j = 0;
    for (i = 0; i < FLORIST_SIZE; i++) {
        for (j = 0; j < FLORIST_SIZE; j++) {
            if (distances[i].data <= distances[j].data) {
                int temp = distances[i].data;
                distances[i].data = distances[j].data;
                distances[j].data = temp;

                temp = distances[i].florist_no;
                distances[i].florist_no = distances[j].florist_no;
                distances[j].florist_no = temp;
            }
        }
    }
}

int containsFlower(int florist_no, int client_no) {
    if (strcmp(florists_array[florist_no].flower_1, clients_array[client_no].flower) == 0) {
        return 1;
    } else if (strcmp(florists_array[florist_no].flower_2, clients_array[client_no].flower) == 0) {
        return 1;
    } else if (strcmp(florists_array[florist_no].flower_3, clients_array[client_no].flower) == 0) {
        return 1;
    }
    return 0;
}

double calculateDistance(int x_1, int y_1, int x_2, int y_2) {

    return sqrt(pow((x_1 - x_2), 2) + pow((y_1 - y_2), 2));
}

double randomAmountOfTime(double ms) {
    srand(time(NULL));
    int t=(double) 10 + rand() % 41;
    t+=time_counter;
    ++time_counter;
    t=(t % 41);
    return t+ms;
}
void setFloristInformation(florist *one_florist, int info_order, char *line) {
    switch (info_order) {
        case 1:
            one_florist->coordinate_x = atoi(line);
            break;

        case 2:
            one_florist->coordinate_y = atoi(line);
            break;

        case 3:
            one_florist->average_speed = atof(line);
            break;

        case 4:
            strcpy(one_florist->flower_1, line);
            break;

        case 5:
            strcpy(one_florist->flower_2, line);
            break;

        case 6:
            strcpy(one_florist->flower_3, line);

            break;
    }
}

void setClientInformation(client *one_client, int info_order, char *line) {
    switch (info_order) {
        case 1:
            one_client->coordinate_x = atoi(line);
            break;

        case 2:
            one_client->coordinate_y = atoi(line);
            break;

        case 3:
            strcpy(one_client->flower, line);
            break;
    }
}