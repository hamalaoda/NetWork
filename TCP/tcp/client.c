#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    /* 1.创建socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    /* 2.服务器地址 */
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    /* 3.转换字符串IP > 数字IP*/
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    /* 4. 连接服务器 */
    connect(sock, (struct sockaddr *)&server, sizeof(server));
    printf("服务器连接成功");

    char buf[1024];

    /* 循环写入并发送 */
    while (1)
    {
        printf("输入要发送的内容：");
        fgets(buf, sizeof(buf), stdin);

        // 输入exit退出循环
        if (strcmp(buf, "exit\n") == 0)
        {
            break;
        }

        // 发送给服务器
        send(sock, buf, strlen(buf), 0);
    }

    /* 5.关闭服务器 */
    close(sock);
    return 0;
}
