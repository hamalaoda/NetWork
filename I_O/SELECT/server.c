#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>

#define PORT 8888
#define MAX_BUF 1024

int main(int argc, char const *argv[])
{
    int listenfd, connfd;
    struct sockaddr_in server_addr;

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

    fd_set allset;        // 保存所有需要监控的fd
    fd_set rset;          // 每次select使用的临界变量
    int maxfd = listenfd; // 当前最大的fd

    /*
     * client[i] 用来保存每个客户端的 fd。
     * FD_SETSIZE 一般是 1024，所以最多支持 1024 客户端。
     */

    int client[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1; // 表示该位置没有客户端占用
    }

    // 初始化fd集合
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset); // 将监听fd加入集合

    /********************************************************
     * 4. 主循环：不断使用 select 监听所有 fd 的变化
     ********************************************************/
    while (condition)
    {
        /* select会破坏 fd_set, 所以每次都要复制一份 */

        rset = allset;
    }
}
