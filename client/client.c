/************************************************************
    File Name : client.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/03/28 14:51:34
************************************************************/

#include "../common/common.h"
#include "../common/head.h"
#include "../common/tcp_client.h"

int main(int argc, char **argv) {
    char msg[512] = {0};
    int sockfd;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [ip] [port]\n", argv[0]);
        return 1;
    }

    if ((sockfd = socket_connect_timeout(argv[1], atoi(argv[2]), 10)) < 0) {
        perror("socket_connect");
        return 2;
    }
    make_nonblock_ioctl(sockfd);
    recv(sockfd, msg, 512, 0);
    printf("Recv: %s\n", msg);
    close(sockfd);
}