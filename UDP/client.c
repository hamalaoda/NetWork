#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

//客户端

int main()
{
        int sockfd;
        int ret;
        socklen_t addrlen;
        struct sockaddr_in srvaddr;
        struct sockaddr_in cltaddr;
        char buf[512];
        /* 创建UDP通信套接字文件 */
        sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
        if (sockfd == -1) {
                perror("socket");
                return -1; 
        }

        srvaddr.sin_family = AF_INET;
        srvaddr.sin_port = htons(8888);
        inet_pton(AF_INET, "10.7.170.19", &srvaddr.sin_addr.s_addr);
        while(1) {
                addrlen = sizeof(cltaddr);
                fgets(buf, sizeof(buf), stdin);
                /* 客户端作为数据的发送端：向服务器发送数据 */
                ret = sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr *)&srvaddr, addrlen);
                if (ret == -1) {
                        perror("sendto");
                        return -1;
                }

                memset(buf, 0, sizeof(buf));
                /* 客户端作为数据的接收端：接收服务器端的数据应答 */
                ret = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
                if (-1 == ret ) {
                        perror("recvfrom");
                        return -1;
                }

                printf("buf:%s\n", buf);
        }

}