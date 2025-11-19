#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8888
#define MAX_BUF 1024
int main(int argc, char const *argv[])
{
    int listenfd, connfd;
    struct sockaddr_in server_addr;

    /* 创建TCP监听套接字 listenfd，只用于“等待客户端来连接” */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket");
        exit(1);
    }

    /* 绑定IP地址 以及端口号 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意IP
    server_addr.sin_port = htons(PORT);              // 端口号

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    server_addr return 0;
}
