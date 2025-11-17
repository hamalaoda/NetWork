#include <stdio.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>

//服务器

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

        /* 将UDP套接字文件绑定主机的IP地址和固定的端口号 */
        srvaddr.sin_family = AF_INET;
        srvaddr.sin_port = htons(8888);
        inet_pton(AF_INET, "10.7.170.19", &srvaddr.sin_addr.s_addr);
        if (-1 == bind(sockfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr))) {
                perror("bind");
                return -1;
        } 
        
        while(1) {
                addrlen = sizeof(cltaddr);
                memset(buf, 0, sizeof(buf));
                /* 作为接收端：接收对端的所发送的数据 */
                ret = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&cltaddr, &addrlen);
                if (-1 == ret ) { 
                        perror("recvfrom");
                        return -1; 
                }
    
                printf("%s->%d:%s\n", inet_ntoa(cltaddr.sin_addr), ntohs(cltaddr.sin_port), buf);
                /* 作为发送端：发送数据到对端 */
                ret = sendto(sockfd, buf, ret, 0, (const struct sockaddr *)&cltaddr, addrlen);
                if (ret == -1) {
                        perror("sendto");
                        return -1; 
                }

        }
    
}