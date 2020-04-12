/************************************************************
    File Name : server.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/04/11 18:12:28
************************************************************/

#include "../../common/color.h"
#include "../../common/common.h"
#include "../../common/head.h"
#include "../../common/tcp_server.h"

#define POLLSIZE 128
#define BUFFSIZE 512

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
        perror("scoket_create");
        exit(1);
    }

    struct pollfd event_set[POLLSIZE];

    for (int i = 0; i < POLLSIZE; ++i) {
        event_set[i].fd = -1;
    }

    event_set[0].fd = server_listen;
    event_set[0].events = POLLIN;

    while (1) {
        int retval;
        if ((retval = poll(event_set, POLLSIZE, -1)) < 0) {
            perror("poll");
            return 1;
        }

        if (event_set[0].revents & POLLIN) {
            if ((fd = accept(server_listen, NULL, NULL)) < 0) {
                perror("accept");
                continue;
            }
            retval--;

            int i;
            for (i = 1; i < POLLSIZE; ++i) {
                if (event_set[i].fd < 0) {
                    event_set[i].fd = fd;
                    event_set[i].events = POLLIN;
                    break;
                }
            }
            if (i == POLLSIZE) {
                printf(YELLOW "[FULL]" NONE " Too many connections!\n");
                close(fd);
            }
        }

        for (int i = 1; i < POLLSIZE; ++i) {
            if (event_set[i].fd < 0) continue;
            if (event_set[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                retval--;
                char buff[BUFFSIZE] = {0};
                if (recv(event_set[i].fd, buff, BUFFSIZE, 0) > 0) {
                    printf(GREEN "[Recv]" NONE " %s\n", buff);
                    for (int i = 0; i < strlen(buff); ++i) {
                        buff[i] = ch_char(buff[i]);
                    }
                    send(event_set[i].fd, buff, strlen(buff), 0);
                } else {
                    close(event_set[i].fd);
                    event_set[i].fd = -1;
                }
            }
            
            if (retval <= 0) break;
        }
    }

    return 0;
}