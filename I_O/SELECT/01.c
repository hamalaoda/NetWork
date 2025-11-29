#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define PORT 9999
#define MAX_BUF 1024 // 最大监听的fd数量

// select 实现多客户端服务器编程
int main(int argc, char const *argv[])
{
    int listenfd;                   // 服务器启动后唯一用于接收新连接的socket
    struct sockaddr_in server_addr; // 服务器的地址
    struct sockaddr_in client_addr; // 客户端地址
    socklen_t client_len;           // 客户端地址的长度
    fd_set allset;                  // 保存所有需要监听的 fd
    fd_set rset;                    // 保存临时传给select的临时集合；想要监听的fd
    int maxfd;                      // 最大的文件描述符
    int nready;                     // select返回的准备好的fd数
    int connfd;                     // 用于标识服务器与客户端的套接字
    char buf[MAX_BUF];              // 数据缓冲区
    int sockfd;                     // confd的中间变量
    int n = 0;                      // 从客户端读到的字节数

    // socket()创建套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket");
        exit(1);
    }

    // 绑定ip以及端口号
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // 开始监听客户端；从普通套接字变为监听套接字
    listen(listenfd, 5); // backlog=5;即三次握手已准备好的客户端

    printf("服务器已成功启动，开始监听端口号%d...\n", PORT);

    /*
     *   进入select部分
     */

    int client[FD_SETSIZE]; // 保存所有要监控的 fd
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1; // 表面这个位置没有客户端的连接
    }

    // 初始化allset集合
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset); // 将所有打算监听的fd放入集合
                               // 便于select监听是否有新客户端连接到来
                               // 此时只有listenfd  allset[0]的位置为1；表明select将监控这个文件描述符上发生的事情

    maxfd = listenfd; // 初始只有一个监听fd，后续每accept一个客户端就更新maxfd
                      // 初始是3，因为0，1，2被系统占用

    // 循环调用select函数监听
    while (1)
    {
        rset = allset;                                       // 每次调用之前重新赋值，因为select会修改rset的值
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL); //+1因为文件描述符从0开始，要检测本身

        if (nready == -1)
        {
            perror("select");
            return -1;
        }

        // 有新的客户端连接 内核将 listenfd标记为“可读”状态
        if (FD_ISSET(listenfd, &rset)) // 检测到listenfd上出现了可读事件
        {
            client_len = sizeof(client_addr);                                        // 设置：地址的缓冲区有16字节
            connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len); // 在listenfd基础上递增

            if (connfd == -1) // 失败，非阻塞没有连接、被信号中断
            {
                perror("accept");
                continue;
            }

            // 客户端连接成功，保存到 client[]数组
            for (int i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0)
                {
                    client[i] = connfd;
                    break;
                }

                if (i == FD_SETSIZE)
                {
                    printf("客户端连接超过限制，无法再连接\n");
                }
            }

            printf("新的客户端已连接: %s:%d, fd =%d\n",
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port),
                   connfd);

            // 将客户端加入allset 改变的全局的值
            FD_SET(connfd, &allset);

            // 调整manfd
            if (maxfd < connfd)
            {
                maxfd = connfd;
            }

            if (--nready <= 0) // 此时nready表示准备好的文件描述符个数，
                               // 如果 =1 且刚处理完listenfd，则没有其他描述符（如断开连接/客户端有数据）可处理
                               // 则直接跳到下一次循环
            {
                continue;
            }
        }

        // 处理每个客户端的数据
        for (int i = 0; i < FD_SETSIZE; i++)
        {
            sockfd = client[i];
            if (sockfd < 0) // 表明没有客户端连接
            {
                continue;
            }

            if (FD_ISSET(sockfd, &rset)) // 检测到客户端有数据
            {
                memset(buf, 0, sizeof(buf));
                n = read(sockfd, buf, sizeof(buf)); // 将客户端读到的数据返回缓冲区

                if (-1 == n) // 读取文件失败
                {
                    perror("read");
                    close(connfd);
                    FD_CLR(sockfd, &allset); // 在allset清空，不然下一循环又会被赋值
                    client[i] = -1;
                }

                if (n == 0) // 客户端断开连接
                {
                    printf("客户端%d断开连接\n", sockfd);
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }

                else
                {
                    printf("收到来自客户端%d的%d字节的数据:.%s\n", sockfd, n, buf);

                    // 回显给服务器
                    write(sockfd, buf, n);
                }

                if (--nready <= 0) // 同上，表示没有数据处理了
                {
                    break;
                }
            }
        }
    }
    return 0;
}
