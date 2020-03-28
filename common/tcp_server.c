/************************************************************
    File Name : server.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/03/22 20:04:47
************************************************************/

#include "head.h"

int socket_create(int port) {
    int server_listen;

    if ((server_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    printf("Socket created.\n");
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_listen, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        return -1;
    }

    printf("Socket binded.\n");
    if (listen(server_listen, 20) < 0) {
        perror("server");
        return -1;
    }

    return server_listen;
}