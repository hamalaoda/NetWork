#include <stdio.h>
#include <sys/socket.h>     
#include <arpa/inet.h>     
#include <linux/if_ether.h> 
#include <netpacket/packet.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

void my_sendto(int sockfd, unsigned char *msg, int len, char *if_name);
void *recv_arp_ack(void *arg);

int main(void)
{
    // 创建原始套接字
    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd <0)
    {
        perror("socket");
        return 0;
    }

    //创建线程，完成接收arp应答
    pthread_t tid;
    pthread_create(&tid, NULL, recv_arp_ack, &sockfd);
    pthread_detach(tid);
    
    //发送arp请求
    for (int i = 1; i < 255; i++)
    {
        // 构建arp报文
        unsigned char msg[] = 
        {
            /* -------------mac报文-----14B--------*/
            0xff,0xff,0xff,0xff,0xff,0xff,  //广播的目的MAC地址
            0x00,0x0c,0x29,0x67,0x18,0xa8,  //源mac地址（Ubuntu）
            0x08,0x06,                      //arp协议
            /* ----------arp请求报文-----28B--------*/
            0x00,0x01,                      //硬件类型
            0x08,0x00,                      //协议类型
            06,                           //硬件地址长度 （mac为六字节）
            4,                           //协议地址长度 （IPv4为4字节）
            0,1,                           //arp请求
            0x00,0x0c,0x29,0x67,0x18,0xa8,  //源mac地址（Ubuntu）
            10,7,170,10,                    //源IP地址
            0x00,0x00,0x00,0x00,0x00,0x00,  //目的mac地址（因为不知道，所以全0）
            10,7,170,i                     //目的IP地址          
        };

        my_sendto(sockfd, msg, sizeof(msg), "ens33");
    }

    sleep(5);   //接收所以arp应答
    close(sockfd);
    return 0;
}

void *recv_arp_ack(void* arg)
{
    int sockfd =  *(int *)arg; //获取文件描述符

    while (1)
    {
        unsigned char buf[1500] = "";
        int len = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
        if (len>0)  //接收成功
        {
            unsigned short mac_type = ntohs(*(unsigned short *)(buf+12));
            if (mac_type == 0x0806) //是arp报文
            {
                unsigned short arp_op = ntohs(*(unsigned short *)(buf+20));
                if (arp_op == 2)    //是arp应答
                {
                    char src_mac[18] = "";
                    sprintf(src_mac,"%02x:%02x:%02x:%02x:%02x:%02x",
                        buf[22+0], buf[22+1], buf[22+2], buf[22+3], buf[22+4], buf[22+5]);
                    char src_ip[16] = "";
                    inet_ntop(AF_INET, buf+28, src_ip, 16);
                    printf("结果：%s--->%s\n",src_mac,src_ip);
                }        
            }
            
        }
        
    }
    return NULL;
}
void my_sendto(int sockfd, unsigned char *msg, int len, char *if_name)  //if_name 网络接口名称
{
    //从ens33网络接口发送msgarp请求帧数据
    struct ifreq ethreq;
    strncpy(ethreq.ifr_name, if_name, IF_NAMESIZE);
    if (-1 == ioctl(sockfd, SIOCGIFINDEX, &ethreq))
    {
        perror("ioctl");
        close(sockfd);
        _exit(-1);
    }

    struct sockaddr_ll sll;
    bzero(&sll, sizeof(sll));
    sll.sll_ifindex = ethreq.ifr_ifindex;
    //发送arp请求
    int ret = sendto(sockfd, msg, len, 0, (struct sockaddr*)&sll, sizeof(sll));
    if (ret < 0)
    {
        perror("sendto");
        close(sockfd);
        _exit(-1);
    }
}