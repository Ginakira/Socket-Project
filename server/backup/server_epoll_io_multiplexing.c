/*************************************************************************
> File Name: epoll.c
> Author:
> Mail:
> Created Time: Sat 11 Apr 2020 08:26:36 PM CST
************************************************************************/

#include <stdio.h>
#include <sys/epoll.h>

#include "../common/common.h"
#include "../common/head.h"
#include "../common/tcp_server.h"

#define MAX_EVENTS 10
#define BUFFSIZE 512

int main(int argc, char **argv) {
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, nfds, epollfd;
    char buff[BUFFSIZE] = {0};

    if (argc != 2) exit(1);
    listen_sock = socket_create(atoi(argv[1]));

    /* Code to set up listening socket, 'listen_sock',
    (socket(), bind(), listen()) omitted */

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                conn_sock = accept(listen_sock, NULL, NULL);
                if (conn_sock == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                make_nonblock(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
            } else {
                // do_use_fd(events[n].data.fd);
                if (events[n].events & (EPOLLIN | EPOLLERR | EPOLLHUP)) {
                    memset(buff, 0, sizeof(buff));
                    if (recv(events[n].data.fd, buff, BUFFSIZE, 0) > 0) {
                        printf("recv: %s \n", buff);
                        for (int i = 0; i < strlen(buff); ++i) {
                            if (buff[i] >= 'a' && buff[i] <= 'z') buff[i] -= 32;
                        }
                        send(events[n].data.fd, buff, strlen(buff), 0);
                    } else {
                        if (epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd,
                                      NULL) < 0) {
                            perror("epoll_ctrl");
                        }
                        close(events[n].data.fd);
                        printf("Logout!\n");
                    }
                }
            }
        }
    }
    return 0;
}
