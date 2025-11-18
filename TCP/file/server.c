#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ctl.h"



int main(int argc, char const *argv[])
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    addr.sin_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr), sizeof(addr);
    listen(server_fd, 5);

    printf("服务器启动，等待客户端连接\n");

    int client_fd = accept(server_fd, NULL, NULL);
    printf("客户端已连接");

    char cmd[4], filename[128], buffer[1024];

    while (1)
    {
        memset(cmd, 0, sizeof(cmd));

        if (strncmp(cmd, "get", 3) == 0)
        {
            sscanf(cmd, "get %s", filename);
            printf("客户端请求下载文件：%s\n", filename);
        }
        else if (strncmp(cmd, "put", 3))
        {
            sscanf(cmd, "put %s", filename);
            printf("客户端请求上传文件");
        }
        else if (strncmp(cmd, "quit", 4) == 0)
        {
            printf("客户端退出\n");
            break;
        }
        else
        {
            printf("未收到命令，断开连接。\n");
            break;
        }
    }

    close(server_fd);
    close(client_fd);
    return 0;
}
