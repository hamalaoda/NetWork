#include "ctl.h"

/********************************************
 *     recv_n()
 *     保证从 TCP 连接中“读满 len 字节”
 *     原因：TCP 是流式协议，一次 recv 可能读不满
 ********************************************/
void recv_n(int sock, void *buf, int len)
{
    int total = 0;
    while (total < len)
    {
        int n = 0;
        n = recv(sock, (char *)buf + total, len - total, 0);

        if (n <= 0)
        {
            perror("recv");
            exit(1);
        }
        total += n;
    }
}

/********************************************
 * send_n()
 * 保证“发送满 len 字节”
 ********************************************/

void send_n(int sock, void *buf, int len)
{
    int total = 0;
    while (total < len)
    {
        int n = send(sock, (char *)buf + total, len - total, 0);
        if (n <= 0)
        {
            perror("send");
            exit(1);
        }
        total += n;
    }
}

/***********************
 * 处理 PUT（从接收缓冲区读到服务器端；客户端上传）
 ***********************/
void handle_put(int client_fd)
{
    int name_len;

    // 读取文件长度
    recv_n(client_fd, &name_len, sizeof(int)); // 通过client_fd将真实的文件名长度传给函数

    // 读取文件名
    char filename[256];
    recv_n(client_fd, failname, name_len); // 接收真实的文件名，并传给failename
    filename[name_len] = '\0';             // 补'\0'表示结束

    // 读取文件大小
    long long filesize;
    recv_n(client_fd, &filesize, sizeof(long long)); // 由客户端通过client_fd传递实际的文件大小

    // 创建文件
    FILE *fp = fopen(filename, "wb"); // 以二进制的形式“b”创建名为filename的文件
    if (!fp)
    {
        printf("无法创建文件\n");
        return;
    }

    printf("开始接收文件: %s 大小 %lld 字节\n", filename, filesize);

    // 接收文件内容
    char buffer[1024]; // 缓冲区。临时存储从socket接收的数据
    long long recived = 0;

    while (recived < filesize)
    {
        int need = (filesize - recived > 1024) ? 1024 : (filesize - recived); // 每次读取的字节数
        recv_n(client_fd, buffer, need);
        fwrite(buffer, 1, need, fp); // 将缓冲区的字节写入文件，1表示每次写入1个字节，need表示写入次数
        recived += need;
    }

    fclose(fp);
    printf("文件接收完毕：%s\n", filename);
}

/***********************
 * 处理 GET（客户端下载）
 * 服务器端往客户端发送数据
 ***********************/

void handle_get(int client_fd)
{
    int name_len;

    // 接收文件名长度
    recv_n(client_fd, &name_len, sizeof(int));

    // 接收文件名
    char filename[256];
    recv_n(client_fd, filename, name_len);
    filename[name_len] = '\0'; // 补'\0'表示结束

    // 打开文件
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        // 文件不存在，直接发送 ERR
        send(client_fd, "ERR", 3, 0);
        printf("客户端请求 %s，但该文件不存在\n", filename);
        return;
    }

    // 文件存在，发送“OK”
    send(client_fd, "OK", 2, 0);

    // 文件名长度+文件名
    send_n(client_fd, &name_len, sizeof(int));
    send_n(client_fd, &filename, name_len);

    // 文件大小
    fseek(fp, 0, SEEK_END);
    long long filesize = ftell(fp);                  // 返回文件当前相当于开头的字节量，即文件的总字节数
    fseek(fp, 0, SEEK_SET);                          // 将文件指针由尾到头
    send_n(client_fd, &filesize, sizeof(long long)); // 用8个字节的容器来存储实际的文件内容大小
    printf("开始发送文件：%s 大小 %lld 字节\n", filename, filesize);

    // 发送文件内容
    char buffer[1024];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        send_n(client_fd, buffer, n);
    }

    fclose(fp);
    printf("文件发送完毕：%s\n", filename);
}

/***********************
 * 上传文件（PUT）(客户端往发送缓冲区发送)
 ***********************/
void do_put(int sock, char *filename)
{
    FILE *fp = fopen(filename, "rb"); // 只读二进制形式打开文件
    if (fp == NULL)
    {
        perror("文件打开失败\n");
        return;
    }

    // 发送put命令
    send(sock, "put", 3, 0);

    // 发送文件名长度+文件名
    int name_len = strlen(filename);
    send_n(sock, &name_len, sizeof(int));
    send_n(sock, filename, name_len);

    // 发送文件大小
    fseek(fp, 0, SEEK_END);
    long long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    send_n(sock, &filesize, sizeof(long long)); // 通过sock传递真实大小

    printf("开始上传：%s 大小 %lld 字节\n", filename, filesize);

    // 发送文件内容
    char buffer[1024];
    size_t n;
    while (n = fread(buffer, 1, sizeof(buffer), fp) > 0)
    {
        send_n(sock, buffer, n);
    }
}

/***********************
 * 下载文件（GET）
 ***********************/

void do_get(int sock, char *filename)
{
    // 发送 get 命令
    send(sock, "get", 3, 0);

    // 发送文件名 +文件名长度
    int name_len = strlen(filename);
    send_n(sock, &name_len, sizeof(int)); // 文件名长度
    send_n(sock, filename, name_len);     // 文件名

    // 接收状态：“OK” 或 “ERR”
    char status[4] = {0};
    recv(sock, status, 3, 0);

    if (strcmp(status, "ERR") == 0)
    {
        printf("服务器上不存在文件：%s\n", filename);
        return;
    }

    // 读取文件名长度+文件名
    recv_n(sock, &name_len, sizeof(int));
    recv_n(sock, filename, name_len);
    filename[name_len] = '\0';

    // 读取文件大小
    long long filesize;
    recv_n(sock, &filesize, sizeof(long long));

    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        perror("无法创建文件");
        return;
    }

    printf("开始下载：%s 大小 %lld 字节\n", filename, filesize);

    // 接受文件的内容
    char buffer[1024];
    long long recv_total = 0;
}