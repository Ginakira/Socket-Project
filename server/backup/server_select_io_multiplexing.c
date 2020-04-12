/************************************************************
    File Name : server_select_iom.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/04/12 14:27:31
************************************************************/

#include "../common/color.h"
#include "../common/common.h"
#include "../common/head.h"
#include "../common/tcp_server.h"

#define MAX_CLIENTS 256
#define BUFF_SIZE 512

int sock_fds[MAX_CLIENTS];
int max_fd_number;

char ch_char(char c) {
    if (c >= 'a' && c <= 'z')
        return c - 32;
    else
        return c;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    int server_listen, fd;
    if ((server_listen = socket_create(atoi(argv[1]))) < 0) {
        perror("socket_create");
        exit(1);
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        sock_fds[i] = -1;
    }

    fd_set event_set;
    sock_fds[0] = server_listen;
    max_fd_number = server_listen;

    while (1) {
        FD_ZERO(&event_set);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (sock_fds[i] < 0) continue;
            FD_SET(sock_fds[i], &event_set);
            max_fd_number =
                sock_fds[i] > max_fd_number ? sock_fds[i] : max_fd_number;
        }

        int retval;
        if ((retval = select(max_fd_number + 1, &event_set, NULL, NULL, NULL)) <
            0) {
            perror("select");
            return 1;
        }

        if (FD_ISSET(sock_fds[0], &event_set)) {
            if ((fd = accept(server_listen, NULL, NULL)) < 0) {
                perror("accept");
                continue;
            }
            retval--;

            int ind;
            for (ind = 1; ind < MAX_CLIENTS; ++ind) {
                if (sock_fds[ind] < 0) {
                    sock_fds[ind] = fd;
                    printf(GREEN "[Login]" NONE " New sock_fd %d logged in\n",
                           sock_fds[ind]);
                    break;
                }
            }
            if (ind == MAX_CLIENTS) {
                printf(YELLOW "[FULL]" NONE " Too many connections!\n");
                close(fd);
            }
        }

        for (int i = 1; retval && i < MAX_CLIENTS; ++i) {
            if (sock_fds[i] < 0) continue;
            if (FD_ISSET(sock_fds[i], &event_set)) {
                retval--;
                char buff[BUFF_SIZE] = {0};
                if (recv(sock_fds[i], buff, BUFF_SIZE, 0) > 0) {
                    printf(GREEN "[Recv]" NONE " %s\n", buff);
                    for (int i = 0; i < strlen(buff); ++i) {
                        buff[i] = ch_char(buff[i]);
                    }
                    send(sock_fds[i], buff, strlen(buff), 0);
                } else {
                    close(sock_fds[i]);
                    sock_fds[i] = -1;
                }
            }
        }
    }

    return 0;
}