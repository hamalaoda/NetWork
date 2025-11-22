#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    // 设置超时时间
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    // 调用select
    retval = select(1, &rfds, NULL, NULL, &tv);

    printf("come from select\n");

    if (retval == -1)
    {
        perror("select");
    }
    else if (FD_ISSET(0, &rfds)) // 0标准输入
    {
        printf("data is available now\n"); // 表面有可读事件发生
    }
    else
    {
        printf("no data five seconds");
    }

    return 0;
}
