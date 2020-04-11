/************************************************************
    File Name : client.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/03/22 20:21:17
************************************************************/
#include "color.h"
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
    printf(YELLOW "Socket created." NONE "\n");

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        return -1;
    }
    printf(GREEN "Socket connected." NONE "\n");
    return sockfd;
}

int socket_connect_timeout(char *host, int port, int timeout) {
    int sockfd;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    printf(YELLOW "Socket created." NONE "\n");

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        return -1;
    }

    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int retval = select(sockfd + 1, &rfds, NULL, NULL, &tv);

    if (retval < 0) {
        perror("select");
        exit(1);
    }

    if (retval == 0) {
        printf(RED " [ERR] " NONE "Socket connection has timed out!\n");
        exit(1);
    }

    if (FD_ISSET(sockfd, &rfds)) {
        printf(GREEN "Socket connected." NONE "\n");
        return sockfd;
    }

    printf(RED " [ERR] " NONE "Socket connection failed!\n");
    return -1;
}
