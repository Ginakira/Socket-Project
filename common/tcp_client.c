/************************************************************
    File Name : client.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/03/22 20:21:17
************************************************************/
#include "head.h"

int socket_connect(char *host, int port) {
    int sockfd;
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    printf("Socket created.\n");

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        return -1;
    }
    printf("Socket connected.\n");
    return sockfd;
}
