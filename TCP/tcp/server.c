#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    /* 1.创建监听socket，用于等待客户端的请求 */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    /* 2.准备服务器结构地址 */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;         // IPv4
    addr.sin_port = htons(8888);       // 端口
    addr.sin_addr.s_addr = INADDR_ANY; // 绑定本地所有网卡

    /* 3.绑定端口 */
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));

    /* 4.开始监听,队列大小为5 */
    listen(server_fd, 5);
    printf("服务器已启动，等待客户端连接....\n");

    /* 5.阻塞等待客户端连接 */
    int client_fd = accept(server_fd, NULL, NULL);
    printf("客户端已连接\n");

    /* 6.循环通信 */
    char buf[1024];
    while (1)
    {
        // 接收客户端数据
        int n = recv(client_fd, buf, sizeof((buf)-1), 0);

        // n <= 表示：对方关闭 或 网络断开
        if (n <= 0)
        {
            printf("客户端断开连接\n");
            break;
        }
        else
        {
            buf[n] = '\0';
            printf("客户端发来的数据：%s\n", buf);
        }
    }

    // 关闭套接字
    close(client_fd);
    close(server_fd);

    return 0;
}
