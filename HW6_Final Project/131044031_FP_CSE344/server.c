#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#define SIZE 1024
#define ARGC_NUMBER 4
#define PI 3.14159265

typedef struct provider_type {
    char name[SIZE];
    int performance;
    int price;
    int duration;
    int queue[2];
    int sleep_state;
    int online;
} provider;

typedef struct client_type {
    char name[SIZE];
    char priority;
    int homework;
    pid_t pid;
} client;
int lowCost();
int highPerformance();
int highSpeed();

static void signalHandler(int getSignalNumber);
double calculateFactorial(double x);
double calculateCosine(double x);
double taylorSeries(double number);
double randomSleepTime();
void *processingRequests(void *arg);
void *connection_handler(void *socket_desc);
int calculate_row_number(char *fileName);
void server_start_menu(provider *teachers, int providerSize, char *logFileName);
provider *create_provider_array(provider *teachers, char *fileName);
pthread_t *create_thread_array(pthread_t *client_teachers, int size);
void record_information_of_client(char * incomingRequest);
int create_provider_threads(provider *teachers, char *fileName);
void setTeacherInformation(provider *one_provider, int info_order, char *line);
void setClientInformation(client *one_student, int info_order, char *line);
pthread_cond_t cv;
pthread_mutex_t lock_operation;
pthread_mutex_t lock_provider;
pthread_mutex_t lock_write;

FILE * providers_file;
static int counter = 0;
static int provider_size = 0;
static int order = 0;
static int check_message_state = 0;
client one_client;
provider *providers = NULL;
pthread_t *client_teachers = NULL;
char incomingRequest[SIZE] = {0};
char finded_provider_name[SIZE] = {0};
char sent_message[SIZE] = {0};

static void signalHandler(int getSignalNumber) {
    int i = 0;
    if (getSignalNumber == SIGTERM) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");

        exit(0);
    } else if (getSignalNumber == SIGINT) {
        fprintf(stderr, "CTRL-C trapped system is shutting down\n");


        exit(0);
    }
}

int main(int argc, char** argv) {
    int i = 0, size = 0, thread_error = 0, true = 1;
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;
    char incomingRequest[1024] = {0};
    char buffer[SIZE] = {0};
    struct sigaction act = {
        {0}
    };
    act.sa_handler = signalHandler;
    act.sa_flags = 0;
    if (argc != ARGC_NUMBER) {
        fprintf(stderr, "Usage: executableFile connectionPort providersFile logFile\n");
        exit(0);
    }
    
    if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1) || (sigaction(SIGTERM, &act, NULL) == -1)) {
        perror("Failed to set Signal handler");
        exit(1);
    }
    if (pthread_mutex_init(&lock_provider, NULL) != 0) {
        fprintf(stderr, "Mutex has failed\n");
        exit(0);
    }
    if (pthread_mutex_init(&lock_operation, NULL) != 0) {
        fprintf(stderr, "Mutex has failed\n");
        exit(0);
    }
    if (pthread_mutex_init(&lock_write, NULL) != 0) {
        fprintf(stderr, "Mutex has failed\n");
        exit(0);
    }
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        fprintf(stderr, "Could not create socket\n");
        exit(0);
    }
    true = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &true, sizeof (int)) == -1) {
        fprintf(stderr, "Set sock opt error\n");
        exit(0);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    if (bind(socket_desc, (struct sockaddr *) &server, sizeof (server)) < 0) {
        fprintf(stderr, "bind failed\n");
        exit(0);
    }
    listen(socket_desc, 10);

    size = calculate_row_number(argv[2]);
    provider_size = size;
    providers = create_provider_array(providers, argv[2]);
    create_provider_threads(providers, argv[2]);
    client_teachers = create_thread_array(client_teachers, size);

    server_start_menu(providers, size, argv[3]);
    for (i = 0; i < size; ++i) {
        thread_error = pthread_create(&(client_teachers[i]), NULL, &processingRequests, (void*) &providers[i]);
        if (thread_error != 0)
            fprintf(stderr, "Thread can't be created :[%s]", strerror(thread_error));
    }

    while ((client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t*) & c))) {
        puts("Connection accepted");
        sleep(1);
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void*) new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }
        pthread_join(sniffer_thread, NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }

    for (i = 0; i < size; ++i) {
        pthread_join(client_teachers[i], NULL);
    }
    free(providers);
    free(client_teachers);
    pthread_mutex_destroy(&lock_write);
    pthread_mutex_destroy(&lock_provider);
    pthread_mutex_destroy(&lock_operation);
    return (EXIT_SUCCESS);
}

void* processingRequests(void *arg) {
    int i = 0;
    int size = 0;
    double result = 0.0;
    double time = 0;

    fprintf(stderr, "Provider %s waiting for tasks\n", ((provider*) arg)->name);
    while (1) {
        pthread_mutex_lock(&lock_provider);
        if (strcmp((((provider*) arg)->name), finded_provider_name) == 0) {

            result = taylorSeries(one_client.homework);
            time = randomSleepTime();
            sleep(time);
            fprintf(stderr, "%s's task completed by %s in %.2f seconds, cos(%d)=%.2f, cost is %d total time spent %.2f seconds.\n",
                    one_client.name,
                    ((provider*) arg)->name,
                    time,
                    one_client.homework,
                    result,
                    ((provider*) arg)->price,
                    time);
            strcat(finded_provider_name, " ");
            sprintf(sent_message, "%s's task completed by %s in %.2f seconds, cos(%d)=%.2f, cost is %d total time spent %.2f seconds.\n",
                    one_client.name,
                    ((provider*) arg)->name,
                    time,
                    one_client.homework,
                    result,
                    ((provider*) arg)->price,
                    time);
            check_message_state = 1;
            order = 0;
        } else {
            pthread_cond_wait(&cv, &lock_provider);
        }
        pthread_mutex_unlock(&lock_provider);
    }
    return NULL;
}

void *connection_handler(void *socket_desc) {
    int i = 0, find_provider;
    int sock = *(int*) socket_desc;
    int read_size;
    char *message, client_message[SIZE];
    if ((read_size = recv(sock, client_message, SIZE, 0)) > 0) {

        while (1) {
            if (order == 0) {
                record_information_of_client(client_message);

                fprintf(stderr, "%s %c %d %d\n", one_client.name, one_client.priority, one_client.homework, one_client.pid);
                order = 1;
                if (one_client.priority == 'C') {
                    find_provider = lowCost();
                    pthread_mutex_lock(&lock_operation);
                    strcpy(finded_provider_name, providers[find_provider].name);

                    for (i = 0; i < provider_size; ++i)
                        pthread_cond_signal(&cv);

                    pthread_mutex_unlock(&lock_operation);
                } else if (one_client.priority == 'Q') {
                    find_provider = highPerformance();
                    pthread_mutex_lock(&lock_operation);
                    strcpy(finded_provider_name, providers[find_provider].name);

                    for (i = 0; i < provider_size; ++i)
                        pthread_cond_signal(&cv);
                    pthread_mutex_unlock(&lock_operation);
                } else if (one_client.priority == 'T') {

                    find_provider = highSpeed();
                    pthread_mutex_lock(&lock_operation);
                    strcpy(finded_provider_name, providers[find_provider].name);

                    for (i = 0; i < provider_size; ++i)
                        pthread_cond_signal(&cv);
                    pthread_mutex_unlock(&lock_operation);
                }
                break;
            }
        }
        while (1) {
            if (check_message_state == 1) {
                pthread_mutex_lock(&lock_write);
                write(sock, sent_message, strlen(sent_message));
                pthread_mutex_unlock(&lock_write);
                break;
            }
        }
        check_message_state = 0;
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        fprintf(stderr, "recv failed\n");
    }
    free(socket_desc);
    return 0;
}

void server_start_menu(provider *teachers, int providerSize, char *logFileName) {
    int i = 0;
    fprintf(stderr, "Logs kept at  %s\n", logFileName);
    fprintf(stderr, "%d provider threads created\n", providerSize);

    for (i = 0; i < providerSize; ++i) {
        fprintf(stdout, "%s %d %d %d\n", teachers[i].name,
                teachers[i].performance, teachers[i].price, teachers[i].duration);
    }
}

int calculate_row_number(char *fileName) {
    size_t len = 0;
    ssize_t read = 0;
    int rowNumber = 0;
    char * line = NULL;
    providers_file = fopen(fileName, "r");
    if (providers_file == NULL) {
        fprintf(stderr, "Failed to open %s", fileName);
        exit(0);
    }
    while ((read = getline(&line, &len, providers_file)) != -1)
        ++rowNumber;
    return rowNumber - 1;
}

provider * create_provider_array(provider *teachers, char *fileName) {
    int size = calculate_row_number(fileName);
    teachers = (provider*) malloc(size * sizeof (provider));
    return teachers;
}

pthread_t *create_thread_array(pthread_t *client_teachers, int size) {
    client_teachers = (pthread_t *) malloc(size * sizeof (pthread_t));
    return client_teachers;
}

int create_provider_threads(provider *teachers, char *fileName) {
    ssize_t read;
    size_t len = 0;
    int teacher_order = 0;
    int info_order = 1;
    char * line = NULL;
    char *token = NULL;
    const char delimeters[9] = " ( ,;:)\n";

    providers_file = fopen(fileName, "r");
    if (providers_file == NULL) {
        fprintf(stderr, "Failed to open file %s", fileName);
        exit(0);
    }
    read = getline(&line, &len, providers_file);
    while ((read = getline(&line, &len, providers_file)) != -1) {
        if (strcmp(line, "") != 0) {
            token = strtok(line, delimeters);
            info_order = 1;
            while (token != NULL) {
                setTeacherInformation(&teachers[teacher_order], info_order, token);
                token = strtok(NULL, delimeters);
                ++info_order;
            }
            ++teacher_order;
        }
    }
}

void setTeacherInformation(provider *one_provider, int info_order, char *line) {
    switch (info_order) {
        case 1:
            strcpy(one_provider->name, line);
            break;

        case 2:
            one_provider->performance = atoi(line);
            break;

        case 3:
            one_provider->price = atoi(line);
            break;

        case 4:
            one_provider->duration = atoi(line);
            break;
    }
    one_provider->queue[0] = 0;
    one_provider->queue[1] = 0;
    one_provider->sleep_state = 0;
    one_provider->online = 1;
}

void record_information_of_client(char * incomingRequest) {
    int info_order = 1;
    char *token = NULL;
    const char delimeters[9] = " ( ,;:)\n";

    token = strtok(incomingRequest, delimeters);
    info_order = 1;
    while (token != NULL) {
        setClientInformation(&one_client, info_order, token);
        token = strtok(NULL, delimeters);
        ++info_order;
    }
}

void setClientInformation(client *one_student, int info_order, char *line) {
    switch (info_order) {
        case 1:
            strcpy(one_student->name, line);
            break;

        case 2:
            one_student->priority = *line;
            break;

        case 3:
            one_student->homework = atoi(line);
            break;

        case 4:
            one_student->pid = atoi(line);
            break;
    }
}

int lowCost() {
    int i = 0, minimumCost = 0;
    minimumCost = providers[0].price;
    for (i = 1; i < provider_size; i++) {
        if ((providers[i].price < minimumCost) &&
                providers[i].online == 1 && (providers[i].queue[0] == 0 || providers[i].queue[1] == 0)) {
            minimumCost = providers[i].price;
        }
    }
    for (i = 0; i < provider_size; i++) {
        if (minimumCost == providers[i].price) {
            return i;
        }
    }
}

int highPerformance() {
    int i = 0, maxPerformance = 0;
    maxPerformance = providers[0].performance;
    for (i = 1; i < provider_size; i++) {
        if ((providers[i].performance > maxPerformance) &&
                providers[i].online == 1 && (providers[i].queue[0] == 0 || providers[i].queue[1] == 0)) {
            maxPerformance = providers[i].performance;
        }
    }
    for (i = 0; i < provider_size; i++) {
        if (maxPerformance == providers[i].performance) {
            return i;
        }
    }
}

int highSpeed() {
    int i = 0, highSpeed = 0;
    highSpeed = providers[0].duration;
    for (i = 1; i < provider_size; i++) {
        if ((providers[i].duration > highSpeed) &&
                providers[i].online == 1 && (providers[i].queue[0] == 0 || providers[i].queue[1] == 0)) {
            highSpeed = providers[i].duration;
        }
    }
    for (i = 0; i < provider_size; i++) {
        if (highSpeed == providers[i].duration) {
            return i;
        }
    }
}

double calculateFactorial(double x) {
    if (x == 0 || x == 1)
        return 1;
    return x * calculateFactorial(x - 1);
}

double calculateCosine(double x) {
    int n = 20;
    double result = 0.0;
    for (int i = 0; i < n; i++) {
        result += pow(-1.0, i) * pow(x, 2.0 * i) / calculateFactorial(2.0 * i);
    }
    return result;
}

double taylorSeries(double number) {
    return calculateCosine(PI / (180 / number));
}

double randomSleepTime() {
    srand(time(NULL));
    return (double) 5 + rand() % 10;
}
