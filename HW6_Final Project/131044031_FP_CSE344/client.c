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
#include <arpa/inet.h>
#define ARGC_NUMBER 6
#define SIZE 1024
static void signalHandler(int getSignalNumber);

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
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char request_message[SIZE] = {0};
    char receive_message[SIZE] = {0};
    char pid_array[SIZE] = {0};
    struct sigaction act = {
        {0}
    };
    act.sa_handler = signalHandler;
    act.sa_flags = 0;
    if (argc != ARGC_NUMBER) {
        fprintf(stderr, "Usage: executableFile clientName priority theHomework serverAddress serverPortAddress\n");
        exit(0);
    }
    if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1) || (sigaction(SIGTERM, &act, NULL) == -1)) {
        perror("Failed to set Signal handler");
        exit(1);
    }
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Socket didn't create\n");
        exit(0);
    }
    memset(&serv_addr, '0', sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[5]));

    if (inet_pton(AF_INET, argv[4], &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address\n");
        exit(0);
    }
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        fprintf(stderr, "Connection failed \n");
        exit(0);
    }
    strcpy(request_message, argv[1]);
    strcat(request_message, " ");
    strcat(request_message, argv[2]);
    strcat(request_message, " ");
    strcat(request_message, argv[3]);
    strcat(request_message, " ");
    sprintf(pid_array, "%d", getpid());
    strcat(request_message, pid_array);

    send(sock, request_message, strlen(request_message), 0);
    fprintf(stderr, "Client %s is requesting %s %s from server %s:%s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
    valread = read(sock, receive_message, 1024);
    fprintf(stderr, "%s\n", receive_message);

    close(sock);
    return 0;
}

