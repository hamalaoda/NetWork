#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BROADCAST_PORT 8888  // 广播端口（必须与发送端一致）
#define BUFFER_SIZE 1024     // 接收数据缓冲区大小

int main() {
    int sockfd;
    struct sockaddr_in recv_addr;  // 接收端地址结构
    struct sockaddr_in sender_addr; // 发送端地址结构
    socklen_t sender_addr_len = sizeof(sender_addr);
    char buffer[BUFFER_SIZE];

    // ========== 步骤 1：创建 UDP 套接字 ==========
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // ========== 步骤 2：设置套接字选项，允许重用端口和地址 ==========
    // SO_REUSEADDR：允许绑定到已在使用中的地址（避免程序重启时端口占用问题）
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // ========== 步骤 3：配置接收端地址并绑定端口 ==========
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;         // IPv4 地址族
    recv_addr.sin_addr.s_addr = INADDR_ANY; // 监听所有网络接口（0.0.0.0）
    recv_addr.sin_port = htons(BROADCAST_PORT); // 绑定到广播端口

    // 绑定套接字到指定地址和端口
    if (bind(sockfd, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening for broadcast messages on port %d...\n", BROADCAST_PORT);

    // ========== 步骤 4：循环接收广播数据 ==========
    while (1) {
        // 清空缓冲区
        memset(buffer, 0, BUFFER_SIZE);

        // 接收数据（阻塞等待）
        ssize_t recv_bytes = recvfrom(
            sockfd,
            buffer,
            BUFFER_SIZE - 1,  // 预留 1 字节存字符串结束符
            0,
            (struct sockaddr*)&sender_addr,
            &sender_addr_len
        );

        if (recv_bytes < 0) {
            perror("recvfrom failed");
            continue;  // 出错后继续接收下一个数据
        }

        // 将发送端 IP 地址转换为字符串
        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);

        // 打印接收信息
        printf("Received broadcast from %s:%d: %s\n",
               sender_ip,
               ntohs(sender_addr.sin_port),
               buffer);
    }

    // 关闭套接字（实际不会执行到这里，因为循环是无限的）
    close(sockfd);
    return 0;
}