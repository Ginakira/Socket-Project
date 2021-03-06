/************************************************************
    File Name : client.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/03/29 18:16:08
************************************************************/

#include "../common/color.h"
#include "../common/common.h"
#include "../common/my_chatroom.h"
#include "../common/tcp_client.h"

char *conf = "./client.conf";
int sockfd;
pthread_t send_tid, recv_tid;

void logout(int signalnum) {
    pthread_cancel(recv_tid);
    close(sockfd);
    printf("\r" RED "[LOGOUT]" NONE " Bye!\n");
    exit(1);
}

void *client_send(void *arg) {
    signal(SIGINT, logout);

    char c = 'a';
    // printf(L_PINK "%s:" NONE, get_value(conf, "MY_NAME"));
    struct Msg msg = *(struct Msg *)arg;
    while (c != EOF) {
        char buff[512] = {0};
        scanf("%[^\n]s", buff);
        c = getchar();
        if (buff[0] == '@') {
            msg.flag = 1;
            char *pos = strstr(buff, " ");
            if (pos == NULL) {
                msg.flag = 0;
                strcpy(msg.message, buff);
            } else {
                strcpy(msg.message, pos + 1);
                strncpy(msg.to, buff + 1, pos - buff - 1);
                msg.to[pos - buff - 1] = 0;
                printf(YELLOW "[PRIVATE] : to %s" NONE "\n", msg.to);
            }

        } else if (buff[0] == '#') {
            msg.flag = 4;
            strcpy(msg.message, buff);
        } else {
            msg.flag = 0;
            strcpy(msg.message, buff);
        }
        chat_send(msg, sockfd);
        memset(msg.message, 0, sizeof(msg.message));
        memset(msg.to, 0, sizeof(msg.to));
    }
    close(sockfd);
    return NULL;
}

void *client_recv(void *arg) {
    struct RecvMsg rmsg;
    while (1) {
        rmsg = chat_recv(sockfd);
        if (rmsg.retval < 0) {
            return NULL;
        }
        printf("\r" BLUE "%s" NONE GREEN "[%d]" NONE ": %s\n", rmsg.msg.from,
               rmsg.msg.flag, rmsg.msg.message);
    }
    return NULL;
}

int main() {
    int port;
    char ip[20] = {0};
    struct Msg msg;

    port = atoi(get_value(conf, "SERVER_PORT"));
    strcpy(ip, get_value(conf, "SERVER_IP"));
    printf("IP = %s, PORT = %d\n", ip, port);

    if ((sockfd = socket_connect(ip, port)) < 0) {
        perror("socket_connect");
        return 1;
    }

    strcpy(msg.from, get_value(conf, "MY_NAME"));
    msg.flag = 2;

    if (chat_send(msg, sockfd) < 0) {
        return 2;
    }

    struct RecvMsg rmsg = chat_recv(sockfd);

    if (rmsg.retval < 0) {
        fprintf(stderr, "Error!\n");
        return 1;
    }

    printf(GREEN "Server" NONE " : %s\n", rmsg.msg.message);

    if (rmsg.msg.flag == 3) {
        close(sockfd);
        return 1;
    }

    if (pthread_create(&send_tid, NULL, client_send, (void *)&msg) != 0) {
        perror("pthread_create");
    }

    if (pthread_create(&recv_tid, NULL, client_recv, NULL) != 0) {
        perror("pthread_create");
    }

    pthread_join(send_tid, NULL);
    pthread_cancel(recv_tid);
    close(sockfd);

    return 0;
}