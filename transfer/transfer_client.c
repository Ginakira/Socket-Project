/************************************************************
    File Name : transfer_client.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/04/02 15:26:28
************************************************************/

#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define NONE "\033[0m"

#define BUFF_SIZE 1024

int main(int argc, char **argv) {
    int port, sockfd;
    char ip[20] = {0};

    if (argc != 4) {
        fprintf(stderr, "Usage: %s [ip] [port] [filepath]\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[2]);
    strcpy(ip, argv[1]);
    printf(YELLOW "IP = %s, PORT = %d\n" NONE, ip, port);

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    printf(GREEN "Socket Created\n" NONE);

    // Connect to server
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = INADDR_ANY;
    if (connect(sockfd, (struct sockaddr *)&client, sizeof(client)) < 0) {
        perror("connect");
        exit(1);
    }
    printf(GREEN "Server connected\n" NONE);

    int fp, n_read;
    char filename[50] = {0};
    char buff[BUFF_SIZE] = {0};

    strcpy(filename, argv[3]);

    if ((fp = open(filename, O_RDONLY)) < 0) {
        perror("open");
        exit(1);
    }
    printf(YELLOW "File %s opened\n" NONE, filename);

    // Send filename to server
    char *slash = strrchr(filename, '/');
    if (send(sockfd, (void *)slash + 1, strlen(filename) - (slash - filename),
             0) < 0) {
        perror("send_filename");
        exit(1);
    }
    printf("File name sent successfully!\n");

    int cnt = 0;
    while ((n_read = read(fp, buff, sizeof(buff))) > 0) {
        if (send(sockfd, (void *)buff, sizeof(buff), 0) < 0) {
            perror("send_file");
            exit(1);
        }
        printf(YELLOW "\r%d - n_read:%d  R/W once" NONE, cnt++, n_read);
        memset(buff, 0, sizeof(buff));
    }
    printf(GREEN "\nFile sent successfully! %s\n" NONE, filename);

    close(fp);
    close(sockfd);
    return 0;
}