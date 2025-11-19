#include "ctl.h"

int main(int argc, char const *argv[])
{
    // 创建套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 绑定地址
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8000);

    // 改为自己的服务器ip
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    connect(sock, (struct sockaddr *)&server, sizeof(server));
    printf("已连接到服务器\n");

    char cmd[256], filename[256];

    while (1)
    {
        // 显示命令
        printf("\n输入命令: put file | get file | quit \n");
        scanf("%s", cmd);

        if (strcmp(cmd, "put") == 0)
        {
            scanf("%s", filename);
            do_put(sock, filename);
        }
        else if (strcmp(cmd, "get") == 0)
        {
            scanf("%s", filename);
            do_get(sock, filename);
        }
        else if (strcmp(cmd, "quit") == 0)
        {
            break;
        }
        else
        {
            printf("无效命令\n");
        }

        close(sock);
        return 0;
    }
}
