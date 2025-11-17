#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BROADCAST_PORT 8888  // 广播接收端口
#define BROADCAST_ADDR "255.255.255.255"  // IPv4 广播地址
#define BUFFER_SIZE 1024  // 发送数据缓冲区大小

int main() {
    int sockfd;
    struct sockaddr_in broadcast_addr;
    char buffer[BUFFER_SIZE] = "Hello, Broadcast!";  // 待发送的广播数据

    // ========== 步骤1：创建UDP数据报套接字 ==========
    // SOCK_DGRAM 表示数据报套接字（UDP），AF_INET 表示IPv4地址族
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // ========== 步骤2：启用广播发送功能 ==========
    // 默认UDP套接字不允许广播，需通过setsockopt开启
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("setsockopt (SO_BROADCAST) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // ========== 步骤3：配置广播目标地址和端口 ==========
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;  // IPv4 地址族
    broadcast_addr.sin_port = htons(BROADCAST_PORT);  // 网络字节序端口号
    // 将广播IP地址转换为网络字节序并赋值
    if (inet_pton(AF_INET, BROADCAST_ADDR, &broadcast_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // ========== 发送广播数据 ==========
    ssize_t sent_bytes = sendto(sockfd, buffer, strlen(buffer), 0, 
                               (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    if (sent_bytes < 0) {
        perror("sendto failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Broadcast sent: %s (bytes sent: %zd)\n", buffer, sent_bytes);

    // 关闭套接字
    close(sockfd);
    return 0;
}