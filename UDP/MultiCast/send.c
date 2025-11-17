// sender.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MULTICAST_IP "224.10.101.12"
#define MULTICAST_PORT 8888

int main()
{
    int sockfd;
    struct sockaddr_in addr;
    char msg[] = "Hello Multicast";

    // 1. 创建UDP套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 2. 设置多播地址
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MULTICAST_PORT);
    addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);

    // 3. 发送数据
    while (1) {
        sendto(sockfd, msg, strlen(msg), 0,
               (struct sockaddr *)&addr, sizeof(addr));
        printf("发送：%s\n", msg);
        sleep(1);
    }

    close(sockfd);
    return 0;
}
