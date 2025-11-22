#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8888
#define MAX_BUF 1024

int main(int argc, char const *argv[])
{
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    int client_len;
    int maxfd;  // 当前最大文件描述符
    int nready; // select返回的准备好的fd数

    /* 1.创建TCP监听套接字 listenfd，只用于“等待客户端来连接” */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket");
        exit(1);
    }

    /* 2.绑定IP地址 以及端口号 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意IP
    server_addr.sin_port = htons(PORT);              // 端口号

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    /* 3.开始监听 第二个参数backlog =5：内核连接队列大小*/
    if (listen(listenfd, 5) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("服务器启动，开始监听端口号：%d...", PORT);

    /********************************************************
     * ============ 下面进入 select 部分 ============
     ********************************************************/

    fd_set allset; // 保存所有需要监控的 fd
    fd_set rset;   // 每次传给select的临时集合;想要监听的可读集合

    int client[FD_SETSIZE]; // 保存所有已连接客户端的fd
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1; // 表面这个位置没有客户端占用；
    }

    maxfd = listenfd; // 此时listenfd只有一个，当accept返回时，才更新maxfd

    // 初始化fd集合
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset); // 将要打算监听的fd放入集合

    // 循环调用select函数监听
    while (1)
    {
        rset = allset; // 每次调用前重新赋值；因为select会修改rset
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1)
        {
            perror("select");
            return -1;
        }

        // 4.有新的客户端连接
        if (FD_ISSET(listenfd, &rset)) // 检查listenfd是否有事件
        {
            client_len = sizeof(client_addr);
            connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
            if (connfd = -1) // 连接失败
            {
                perror("accept");
                return -1;
            }

            // 客户端连接成功，保存到 client[]数组
            for (int i = 0; i < FD_SETSIZE; i++) // 最多有1024个客户端连接 由系统决定
            {
                if (client[i] < 0) // 即没有客户端占用；没有客户端连接
                {
                    client[i] = connfd;
                    break;
                }
            }

            // 将客户端加入allset
            FD_SET(connfd, &allset);

            // 调整maxfd
            if (maxfd < connfd)
            {
                maxfd = connfd;
            }

            if (nready < 0)
            {
                continue; // 没有事件发生则等待
            }
        }

        // 5.处理每个客户端的数据
    }
}
