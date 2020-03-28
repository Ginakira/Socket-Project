/************************************************************
    File Name : server.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/03/28 14:38:45
************************************************************/

#include "../common/head.h"
#include "../common/tcp_server.h"

void chstr(char *str) {
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32;
        }
    }
}

void *work(void *arg) {
    int *fd = (int *)arg;
    if (send(*fd, "You are here.", sizeof("You are here."), 0) < 0) {
        perror("send");
        close(*fd);
    }
    while (1) {
        char msg[512] = {0};
        if (recv(*fd, msg, sizeof(msg), 0) <= 0) {
            perror("recv");
            break;
        }
        printf("recv: %s\n", msg);
        chstr(msg);
        if (send(*fd, msg, strlen(msg), 0) < 0) {
            perror("send");
            break;
        }
        printf("Success in ECHO!\n");
    }
    close(*fd);
    return NULL;
}

int main(int argc, char **argv) {
    int port, server_listen, fd;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        return 1;
    }
    port = atoi(argv[1]);

    if ((server_listen = socket_create(port)) < 0) {
        perror("socket_create");
        return 2;
    }

    pthread_t tid;
    while (1) {
        if ((fd = accept(server_listen, NULL, NULL)) < 0) {
            perror("accept");
        }
        printf("New client login.\n");
        pthread_create(&tid, NULL, work, (void *)&fd);
    }

    return 0;
}