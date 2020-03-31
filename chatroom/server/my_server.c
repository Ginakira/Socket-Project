/************************************************************
    File Name : server.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/03/29 16:13:13
************************************************************/

#include "../common/chatroom.h"
#include "../common/color.h"
#include "../common/common.h"
#include "../common/tcp_server.h"

struct User {
    char name[20];
    int online;
    pthread_t tid;
    int fd;
};

char *conf = "./server.conf";

struct User *client;

// Message Forwarding
void *msg_forward(void *arg) {
    struct Msg msg = *(struct Msg *)arg;
    if (msg.flag == 1) {
        for (int i = 0; i < MAX_CLIENT; ++i) {
            if (client[i].online && !strcmp(client[i].name, msg.to)) {
                chat_send(msg, client[i].fd);
                printf(GREEN "[FORWARD PRIVATE]" NONE " to %s\n",
                       client[i].name);
                return NULL;
            }
        }
        printf(RED "[PRIVATE FAILED]" NONE " to %s\n", msg.to);
    } else if (msg.flag == 0 || msg.flag == 2) {
        for (int i = 0; i < MAX_CLIENT; ++i) {
            if (client[i].online && strcmp(client[i].name, msg.from)) {
                chat_send(msg, client[i].fd);
                printf(GREEN "[FORWARD]" NONE " to %s\n", client[i].name);
            }
        }
    }
    return NULL;
}

void *work(void *arg) {
    int sub = *(int *)arg;
    int client_fd = client[sub].fd;
    struct RecvMsg rmsg;
    printf(GREEN "LOGIN" NONE " : %s\n", client[sub].name);

    // When a user logged in, notify all online users
    struct Msg login_notice;
    login_notice.flag = 2;
    sprintf(login_notice.from, "Server");
    sprintf(login_notice.message, "User %s is" GREEN " online" NONE "\n",
            client[sub].name);
    msg_forward((void *)&login_notice);

    while (1) {
        rmsg = chat_recv(client_fd);
        if (rmsg.retval < 0) {
            printf(PINK "LOGOUT" NONE " : %s \n", client[sub].name);
            close(client_fd);
            client[sub].online = 0;

            // When a user logged out, notify all online users
            struct Msg logout_notice;
            logout_notice.flag = 2;
            sprintf(logout_notice.from, "Server");
            sprintf(logout_notice.message,
                    "User %s has been" RED " offline" NONE "\n",
                    client[sub].name);
            msg_forward((void *)&logout_notice);

            return NULL;
        }
        printf(BLUE "%s" NONE GREEN "[%d]" NONE ": %s\n", rmsg.msg.from,
               rmsg.msg.flag, rmsg.msg.message);
        if (rmsg.msg.flag == 0 || rmsg.msg.flag == 1) {
            pthread_t forward_tid;
            pthread_create(&forward_tid, NULL, msg_forward, (void *)&rmsg.msg);
        }
    }
    return NULL;
}

int find_sub() {
    for (int i = 0; i < MAX_CLIENT; ++i) {
        if (!client[i].online) return i;
    }
    return -1;
}

bool check_online(char *name) {
    for (int i = 0; i < MAX_CLIENT; ++i) {
        if (client[i].online && !strcmp(name, client[i].name)) {
            printf(YELLOW "[WARNING]" NONE " %s is already online.\n", name);
            return true;
        }
    }
    return false;
}

int main() {
    int port, server_listen, fd;
    struct RecvMsg recvmsg;
    struct Msg msg;
    port = atoi(get_value(conf, "SERVER_PORT"));
    client = (struct User *)calloc(MAX_CLIENT, sizeof(struct User));

    if ((server_listen = socket_create(port)) < 0) {
        perror("socket_create");
        return 1;
    }
    while (1) {
        if ((fd = accept(server_listen, NULL, NULL)) < 0) {
            perror("accept");
            continue;
        }
        recvmsg = chat_recv(fd);
        if (recvmsg.retval < 0) {
            close(fd);
            continue;
        }

        if (check_online(recvmsg.msg.from)) {
            msg.flag = 3;
            strcpy(msg.message, "You have already logged in");
            chat_send(msg, fd);
            close(fd);
            continue;
        }

        msg.flag = 2;
        strcpy(msg.message, "Welcome to chat room!");
        chat_send(msg, fd);

        int sub;
        sub = find_sub();
        client[sub].online = 1;
        client[sub].fd = fd;
        strcpy(client[sub].name, recvmsg.msg.from);
        pthread_create(&client[sub].tid, NULL, work, (void *)&sub);
    }
    return 0;
}