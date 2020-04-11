/************************************************************
    File Name : server_io_multiplexing.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/04/11 11:36:04
************************************************************/

#include "../common/color.h"
#include "../common/common.h"
#include "../common/head.h"
#include "../common/tcp_server.h"

#define MAX_CLIENTS 128

int socket_fds[MAX_CLIENTS];
int client_connected = 0;
int max_fd_number = 0x80000000;

int main(int argc, char **argv) {
    int server_listen, port;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    if ((server_listen = socket_create(port)) < 0) {
        perror("socket_create");
        exit(1);
    }

    fd_set watching_fds;
    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    char buff[1024] = {0};
    max_fd_number = server_listen;

    while (1) {
        FD_ZERO(&watching_fds);
        FD_SET(server_listen, &watching_fds);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (socket_fds[i] != 0) {
                FD_SET(socket_fds[i], &watching_fds);
                max_fd_number = socket_fds[i] > max_fd_number ? socket_fds[i]
                                                              : max_fd_number;
            }
        }

        int retval = select(max_fd_number + 1, &watching_fds, NULL, NULL, &tv);

        if (retval < 0) {
            perror("select");
            break;
        }

        if (retval == 0) {
            printf(YELLOW " [TIMEOUT] " NONE "No client is active.\n");
            continue;
        }

        if (FD_ISSET(server_listen, &watching_fds)) {
            int new_fd = accept(server_listen, NULL, NULL);
            if (new_fd < 0) {
                perror("accept");
                break;
            }
            socket_fds[client_connected++] = new_fd;
            printf(GREEN " [LOGIN] " NONE "New client connected!\n");
            if (--retval == 0) continue;
        }

        for (int i = 0; i < client_connected; ++i) {
            if (socket_fds[i] && FD_ISSET(socket_fds[i], &watching_fds)) {
                memset(buff, 0, sizeof(buff));
                if (recv(socket_fds[i], buff, sizeof(buff), 0) <= 0) {
                    socket_fds[i] = 0;
                    continue;
                }
                printf(GREEN "[%d]" NONE " : %s\n", socket_fds[i], buff);
            }
        }
    }
    return 0;
}