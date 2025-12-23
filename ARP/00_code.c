#include <stdio.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <sys/types.h>

int main(int argc, char const *argv[])
{
    //创建原始套接字
    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    printf("sockfd = %d\n", sockfd);
    
    //获取网络帧数据
    while (1)
    {
        unsigned char buf[1500] = ""; //必须无符号，越界处理
        int len = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);

        // 解mac报文
        char src_mac [18] = ""; //  17个字符+结束符
        char dst_mac [18] = "";

        sprintf(dst_mac,"%02x:%02x:%02x:%02x:%02x:%02x",buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        sprintf(src_mac,"%02x:%02x:%02x:%02x:%02x:%02x",buf[0+6], buf[1+6], buf[2+6], buf[3+6], buf[4+6], buf[5+6]);
    
        // 类型
        unsigned short mac_type = ntohs( *(unsigned short *)(buf+12));
        printf("%s------->%s\n",src_mac,dst_mac);   //打印源地址到目的地址

        if (mac_type == 0x0800)
        {
            printf("------------IP报文-------------");
            unsigned char * ip_addr = buf +14;  //跳过mac头部
            int ip_head_len = (ip_addr[0]&0x0f)*4;
            unsigned char ip_type = *(ip_addr +9); //类型
            char src_ip[16] = "";
            char dst_ip[16] = "";

            inet_ntop(AF_INET, ip_addr+12, src_ip,16);
            inet_ntop(AF_INET, ip_addr+16, dst_ip,16);

            printf("\t----%s---->%s\n",src_ip, dst_ip);

            if (ip_type == 1)
            {
                printf("\t----------ICMP-----------\n");
            }
            else if (ip_type ==2)
            {
                printf("\t----------IGMP-----------\n");
            }
            else if (ip_type == 6)
            {
                printf("\t----------TCP-----------\n");
                // 跳过mac和IP的头部
                unsigned char *tcp_addr = buf+14+ip_head_len;
                unsigned short src_port = ntohs(*(unsigned short *)(tcp_addr));
                unsigned short dst_port = ntohs(*(unsigned short *)(tcp_addr + 2));
                int tcp_head_len = ((*(tcp_addr +12))&0xf0)>>4;
                printf("\t-----%hu------>%hu:%s\n",src_port,dst_port,tcp_addr+ip_head_len);
            }
            else if (ip_type == 17)
            {
                printf("\t----------UDP-----------\n");
                // 跳过mac和IP的头部
                unsigned char *udp_addr = buf+14+ip_head_len;
                unsigned short src_port = ntohs(*(unsigned short *)(udp_addr));
                unsigned short dst_port = ntohs(*(unsigned short *)(udp_addr + 2));
                int tcp_head_len = ((*(udp_addr +12))&0xf0)>>4;
                printf("\t-----%hu------>%hu:%s\n",src_port,dst_port,udp_addr+8);
            }
            
        }
        else if (mac_type == 0x0806)
        {
            printf("----------ARP报文-----------\n");
        }
        else if (mac_type == 0x8035)
        {
            printf("----------RARP报文-----------\n");
        }
        
    }
    
    return 0;
}
