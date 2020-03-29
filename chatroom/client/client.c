/************************************************************
    File Name : client.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/03/29 18:16:08
************************************************************/

#include "../common/chatroom.h"
#include "../common/color.h"
#include "../common/common.h"
#include "../common/tcp_client.h"

char *conf = "./client.conf";

int main() {
    int port, sockfd;
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

    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork");
    }

    if (pid == 0) {
        system("clear");
        while (1) {
            printf(L_PINK "Please input message:" NONE "\n");
            scanf("%[^\n]s", msg.message);
            getchar();
            chat_send(msg, sockfd);
            memset(msg.message, 0, sizeof(msg.message));
            system("clear");
        }
    }

    return 0;
}