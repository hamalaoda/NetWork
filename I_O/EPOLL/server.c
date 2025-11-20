#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h> // epoll 函数所需头文件

#define PORT 8888
#define MAX_BUFFER 1024
#define MAX_EVENTS 128 // epoll_wait 一次最多能处理的事件数

// 函数：打印错误并退出
void error_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

// 函数：将文件描述符注册到 epoll 实例中
void epoll_add(int epfd, int fd, uint32_t events)
{
    // 声明 epfd 变量：int epfd (epoll 实例描述符)
    // 声明 fd 变量：int fd (待添加的文件描述符)
    // 声明 events 变量：uint32_t events (关注的事件类型，如 EPOLLIN)
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        error_exit("错误：epoll_ctl 添加 FD 失败");
    }
}

int main()
{
    int listen_fd, client_fd, epfd;
    // 声明 listen_fd 变量：int (监听套接字)
    // 声明 client_fd 变量：int (新的客户端套接字)
    // 声明 epfd 变量：int (epoll 实例描述符)
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;    // 声明 client_len 变量：socklen_t (客户端地址结构体长度)
    char buffer[MAX_BUFFER]; // 声明 buffer 变量：char[MAX_BUFFER] (数据缓冲区)
    int nbytes;              // 声明 nbytes 变量：int (实际读写的字节数)

    // 用于存储 epoll_wait 返回的就绪事件数组
    struct epoll_event events[MAX_EVENTS]; // 声明 events 变量：struct epoll_event[MAX_EVENTS] (就绪事件数组)
    int ready_count;                       // 声明 ready_count 变量：int (实际就绪的 FD 数量)

    // ------------------------------------
    // 1. 初始化监听套接字
    // ------------------------------------
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        error_exit("错误：创建套接字失败");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        error_exit("错误：绑定套接字失败");
    }

    if (listen(listen_fd, 10) == -1)
    {
        error_exit("错误：开始监听失败");
    }

    printf("服务器已启动，正在监听端口 %d...\n", PORT);

    // ------------------------------------
    // 2. 创建 epoll 实例并注册监听 FD
    // ------------------------------------
    // 创建 epoll 实例
    if ((epfd = epoll_create(1)) == -1)
    {
        error_exit("错误：epoll_create 失败");
    }

    // 将监听 FD 注册到 epoll 实例中，关注可读事件
    epoll_add(epfd, listen_fd, EPOLLIN);

    // ------------------------------------
    // 3. 主循环：使用 epoll_wait 监控就绪事件
    // ------------------------------------
    while (1)
    {
        // 阻塞等待事件发生，timeout = -1 (永久等待)
        ready_count = epoll_wait(epfd, events, MAX_EVENTS, -1);

        if (ready_count == -1)
        {
            error_exit("错误：epoll_wait 调用失败");
        }

        // 遍历就绪的事件 (ready_count)
        for (int i = 0; i < ready_count; i++)
        {
            int current_fd = events[i].data.fd; // 声明 current_fd 变量：int (当前就绪的文件描述符)

            // --- A. 监听套接字就绪：有新的连接请求 ---
            if (current_fd == listen_fd)
            {
                client_len = sizeof(client_addr);
                if ((client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len)) == -1)
                {
                    perror("错误：接受连接失败");
                    continue;
                }

                // 将新的客户端 FD 注册到 epoll 实例中
                epoll_add(epfd, client_fd, EPOLLIN);

                printf("新连接已接受。FD: %d\n", client_fd);
            }

            // --- B. 客户端套接字就绪：有数据可读或连接断开 ---
            else if (events[i].events & EPOLLIN)
            {

                memset(buffer, 0, MAX_BUFFER);
                nbytes = read(current_fd, buffer, MAX_BUFFER - 1);

                if (nbytes <= 0)
                {
                    // 客户端断开连接或发生错误
                    if (nbytes == 0)
                    {
                        printf("客户端 FD %d 已断开连接。\n", current_fd);
                    }
                    else
                    {
                        perror("错误：从客户端读取数据失败");
                    }

                    // 从 epoll 实例中移除 FD，并关闭它
                    if (epoll_ctl(epfd, EPOLL_CTL_DEL, current_fd, NULL) == -1)
                    {
                        perror("错误：epoll_ctl 删除 FD 失败");
                    }
                    close(current_fd);
                }
                else
                {
                    // 成功读取到数据，进行回显
                    printf("收到来自 FD %d 的数据: %s", current_fd, buffer);
                    write(current_fd, "回显: ", 9);
                    write(current_fd, buffer, nbytes);
                }
            }

            // --- C. 发生其他事件：例如错误或挂断 ---
            else
            {
                fprintf(stderr, "FD %d 发生未知或错误事件.\n", current_fd);
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, current_fd, NULL) == -1)
                {
                    perror("错误：epoll_ctl 删除错误 FD 失败");
                }
                close(current_fd);
            }
        } // End for 遍历就绪事件
    } // End while (1)

    close(epfd);
    close(listen_fd);
    return 0;
}