/************************************************************
    File Name : transfer_server.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/04/02 10:14:59
************************************************************/

#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define NONE "\033[0m"

#define BUFF_SIZE 1024

void *receive_file(void *);

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }
    int port, server_listen;
    port = atoi(argv[1]);

    // Create socket
    if ((server_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    printf(GREEN "Socket Created\n" NONE);

    // Bind socket
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_listen, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        exit(1);
    }
    printf(GREEN "Scoket binded\n" NONE);

    // Listen socket
    if (listen(server_listen, 20) < 0) {
        perror("listen");
        exit(1);
    }

    // Waiting for the connection
    while (1) {
        int sockfd;
        if ((sockfd = accept(server_listen, NULL, NULL)) < 0) {
            perror("accept");
            close(sockfd);
            continue;
        }
        printf(YELLOW "Client connected\n" NONE);
        pthread_t tid;
        pthread_create(&tid, 0, receive_file, &sockfd);
    }
    close(server_listen);
    return 0;
}

void *receive_file(void *arg) {
    int sockfd = *(int *)arg;
    int fp;
    char buff[BUFF_SIZE] = {0};
    char filename[50] = {0};

    // Receive file from client
    if (recv(sockfd, (void *)filename, sizeof(filename), 0) < 0) {
        perror("recv");
        exit(1);
    }
    printf(YELLOW "Filename: %s\n" NONE, filename);

    // Create file for transfer
    if ((fp = open(filename, O_CREAT | O_RDWR, 0644)) < 0) {
        perror("open");
        return NULL;
    }

    // Write the received content to file
    int n_read, cnt = 0;
    while ((n_read = read(sockfd, buff, sizeof(buff))) > 0) {
        if (write(fp, buff, n_read) != n_read) {
            perror("write");
            return NULL;
        }
        printf(YELLOW "\r%d - n_read:%d  R/W once" NONE, cnt++, n_read);
        memset(buff, 0, sizeof(buff));
    }
    printf(GREEN "\nFile received successfully! %s\n" NONE, filename);
    close(fp);
    close(sockfd);
    return NULL;
}