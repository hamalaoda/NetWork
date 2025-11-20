#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#define PORT 8888
#define MAX_BUFFER 1024
// 初始化 FD 数组容量，可动态调整
#define INITIAL_FD_CAPACITY 5

int main(int argc, char const *argv[])
{
    int listen_fd, client_fd;
    struct sockaddr_in server_addr, client_server;
    char buffer[MAX_BUFFER];

    // 动态数组相关变量
    int fd_count = 0;                      // 当前活动的FD数量
    int fd_capacity = INITIAL_FD_CAPACITY; // 数组当前容量

    // 动态分配struct pollfd 数组
    struct pollfd = malloc();

    /* 1.初始化监听套接字 */
    if ((listen_fd = socket(, AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("创建套接字失败");
        exit(1);
    }

    // 绑定服务器ip与端口号，以及使用协议
    memset(&server_addr, 0, sizeof(server_addr)); // 清空结构体
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 绑定任意可用IPv4地址
    server_addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("绑定套接字失败");
        exit(1);
    }

    if (listen(listen_fd, 10) == -1)
    {
        perror("监听失败");
        exit(1);
    }

    printf("服务器正常启动，开始监听端口 %d ...\n", PORT);
    return 0;
}
