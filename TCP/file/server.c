#include "ctl.h"

int main(int argc, char const *argv[])
{
    // 创建套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        return -1;
    }

    // 绑定地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("服务器启动，等待客户端连接\n");

    // 等待客户端
    int client_fd = accept(server_fd, NULL, NULL);
    printf("客户端已连接\n");

    // 主循环处理命令
    while (1)
    {
        char cmd[4] = {0};

        // 读取 put 或 get 命令
        int n = recv(client_fd, cmd, 3, 0);
        if (n < 0)
        {
            break;
        }

        if (strcmp(cmd, "put") == 0)
        {
            handle_put(client_fd);
        }
        else if (strcmp(cmd, "get") == 0)
        {
            handle_get(client_fd);
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
