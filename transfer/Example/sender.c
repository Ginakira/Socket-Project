/************************************************************
    File Name : sender.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/04/02 18:19:50
************************************************************/

#include "../common/common.h"
#include "../common/head.h"
#include "../common/tcp_client.h"

struct FileMsg {
    long size;
    char name[50];
    char buf[4096];
};

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [ip] [port]\n", argv[0]);
        return 1;
    }

    int sockfd, port;
    char buff[100] = {0};
    char name[50] = {0};
    struct FileMsg filemsg;

    port = atoi(argv[2]);
    if ((sockfd = socket_connect(argv[1], port)) < 0) {
        perror("socket_connect");
        return 1;
    }

    while (1) {
        scanf("%[^\n]s", buff);
        getchar();
        if (!strncmp("send ", buff, 5)) {
            strcpy(name, buff + 5);
        } else {
            fprintf(stderr, "Invailid command!\n");
            continue;
        }
        FILE *fp = NULL;
        size_t size;
        if ((fp = fopen(name, "rb")) == NULL) {
            perror("fopen");
            continue;
        }

        fseek(fp, 0L, SEEK_END);
        filemsg.size = ftell(fp);
        strcpy(filemsg.name, name);

        fseek(fp, 0L, SEEK_SET);
        while ((size = fread(filemsg.buf, 1, 4096, fp))) {
            send(sockfd, (void *)&filemsg, sizeof(filemsg), 0);
            memset(filemsg.buf, 0, sizeof(filemsg.buf));
        }
        printf("After Send!\n");
    }
    close(sockfd);
    return 0;
}