// receiver.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MULTICAST_IP "224.10.101.12"
#define MULTICAST_PORT 8888

int main()
{
    int sockfd;
    struct sockaddr_in local_addr;
    struct ip_mreq mreq;
    char buf[256];

    // 1. 创建UDP套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 2. 允许端口复用（多个接收者都监听同一个多播端口）
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 3. 绑定到本机 IP + 端口
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有本地网卡
    local_addr.sin_port = htons(MULTICAST_PORT);

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // 4. 加入多播组
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY); // 自动选择网卡

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &mreq, sizeof(mreq)) < 0) {
        perror("IP_ADD_MEMBERSHIP");
        exit(1);
    }

    printf("已加入多播组 %s:%d，等待接收...\n", MULTICAST_IP, MULTICAST_PORT);

    // 5. 循环接收
    while (1) {
        int n = recv(sockfd, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = '\0';
            printf("收到：%s\n", buf);
        }
    }

    close(sockfd);
    return 0;
}
