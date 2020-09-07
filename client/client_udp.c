/************************************************************
    File Name : client_udp.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/05/28 19:30:20
************************************************************/

#include "../common/color.h"
#include "../common/common.h"
#include "../common/head.h"
#include "../common/udp_server.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s ipaddr port\n", argv[0]);
        exit(1);
    }

    int port, sockfd;
    struct sockaddr_in client;
    socklen_t addr_len = sizeof(client);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }

    port = atoi(argv[2]);

    printf(BLUE "UDP Socket created." NONE "\n");
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    inet_aton(argv[1], &client.sin_addr);

    while (1) {
        char buff[512] = {0};
        scanf("%s", buff);
        if ((sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client,
                    addr_len)) > 0) {
            printf("[Sended] : %s\n", buff);
        }
        // recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client, &addr_len);
        printf("[Received] : %s\n", buff);
    }
    return 0;
}
