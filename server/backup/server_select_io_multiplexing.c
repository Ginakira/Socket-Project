/************************************************************
    File Name : server_select_iom.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/04/12 14:27:31
************************************************************/

#include "../../common/color.h"
#include "../../common/common.h"
#include "../../common/head.h"
#include "../../common/tcp_server.h"

#define MAX_CLIENTS 256
#define BUFF_SIZE 4096

struct Buffer {
    int fd;
    int flag;
    int send_index;
    int recv_index;
    char buff[BUFF_SIZE];
};

char ch_char(char c) {
    if (c >= 'a' && c <= 'z')
        return c - 32;
    else
        return c;
}

struct Buffer *AllocBuffer() {
    struct Buffer *buffer = (struct Buffer *)malloc(sizeof(struct Buffer));
    if (buffer == NULL) return NULL;
    buffer->fd = -1;
    buffer->flag = buffer->recv_index = buffer->send_index = 0;
    memset(buffer->buff, 0, BUFF_SIZE);
    return buffer;
}

void FreeBuffer(struct Buffer *buffer) {
    free(buffer);
    return;
}

int RecvToBuffer(int fd, struct Buffer *buffer) {
    char buff[BUFF_SIZE] = {0};
    int recv_num;
    while (1) {
        recv_num = recv(fd, buff, sizeof(buff), 0);
        if (recv_num <= 0) break;
        for (int i = 0; i < recv_num; ++i) {
            if (buffer->recv_index < sizeof(buffer->buff)) {
                buffer->buff[buffer->recv_index++] = ch_char(buff[i]);
            }
            if (buffer->recv_index > 1 &&
                buffer->buff[buffer->recv_index - 1] == '\n' &&
                buffer->buff[buffer->recv_index - 2] == '\n') {
                buffer->flag = 1;
            }
        }
        if (recv_num < 0) {
            if (errno == EAGAIN) return 0;
            return -1;
        } else if (recv_num == 0) {
            return 1;
        }
    }

    return 0;
}

int SendFromBuffer(int fd, struct Buffer *buffer) {
    int send_num;
    while (buffer->send_index < buffer->recv_index) {
        send_num = send(fd, buffer->buff + buffer->send_index,
                        buffer->recv_index - buffer->send_index, 0);
        if (send_num < 0) {
            if (errno == EAGAIN) return 0;
            buffer->fd = -1;
            return -1;
        }
        buffer->send_index += send_num;
    }
    if (buffer->send_index == buffer->recv_index) {
        buffer->send_index = buffer->recv_index = 0;
    }
    buffer->flag = 0;
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    int server_listen, fd, max_fd;

    if ((server_listen = socket_create(atoi(argv[1]))) < 0) {
        perror("socket_create");
        exit(1);
    }

    struct Buffer *buffer[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        buffer[i] = AllocBuffer();
    }

    make_nonblock(server_listen);

    fd_set rfds, wfds, efds;
    max_fd = server_listen;

    while (1) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&efds);

        FD_SET(server_listen, &rfds);

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (buffer[i]->fd == server_listen) continue;
            if (buffer[i]->fd > 0) {
                if (max_fd < buffer[i]->fd) max_fd = buffer[i]->fd;
                FD_SET(buffer[i]->fd, &rfds);
                if (buffer[i]->flag == 1) {
                    FD_SET(buffer[i]->fd, &wfds);
                }
            }
        }

        if (select(max_fd + 1, &rfds, &wfds, NULL, NULL) < 0) {
            perror("select");
            return 1;
        }

        if (FD_ISSET(server_listen, &rfds)) {
            if ((fd = accept(server_listen, NULL, NULL)) < 0) {
                perror("accept");
                return 1;
            }

            if (fd > MAX_CLIENTS) {
                printf(YELLOW "[FULL]" NONE " Too many connections!\n");
                close(fd);
            } else {
                make_nonblock(fd);
                if (buffer[fd]->fd == -1) {
                    buffer[fd]->fd = fd;
                    printf(GREEN "[Login]" NONE " New sock_fd %d logged in\n",
                           fd);
                }
            }

            for (int i = 0; i < max_fd; ++i) {
                int retval = 0;
                if (i == server_listen) continue;
                if (FD_ISSET(i, &rfds)) {
                    retval = RecvToBuffer(i, buffer[i]);
                }
                if (retval == 0 && FD_ISSET(i, &wfds)) {
                    retval = SendFromBuffer(i, buffer[i]);
                }
                if (retval) {
                    buffer[i]->fd = -1;
                    printf(L_RED "[Logout]" NONE " %d\n", i);
                    close(i);
                }
            }
        }
    }

    return 0;
}