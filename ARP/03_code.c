// =====================
// 需求：Ubuntu 手工构造 UDP 报文，发送给 Windows
// 基于原始套接字，绕开操作系统提供的标准网络层服务
// =====================

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

// =====================
// UDP 伪首部（用于 UDP 校验）
// =====================
typedef struct
{
    u_int32_t saddr;   // 源 IP
    u_int32_t daddr;   // 目的 IP
    u_int8_t  flag;    // 固定为 0
    u_int8_t  pro;     // 协议号（17 表示 UDP）
    u_int16_t tot_len; // UDP 长度
} WEI_HEAD;

// =====================
// 校验和函数（IP / UDP 通用）
// =====================
unsigned short checksum(unsigned short *buf, int len)
{
    int nword = len / 2;
    unsigned long sum = 0;

    if (len % 2 == 1)
        nword++;

    while (nword--)
        sum += *buf++;

    // 高 16 位回卷
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ~sum;
}

// =====================
// 从指定网卡发送二层数据帧
// =====================
void my_sendto(int sockfd, unsigned char *msg, int len, char *if_name)
{
    struct ifreq ethreq;

    // 设置网卡名（如 ens33）
    strncpy(ethreq.ifr_name, if_name, IFNAMSIZ);

    // 获取网卡索引
    if (ioctl(sockfd, SIOCGIFINDEX, &ethreq) == -1)
    {
        perror("ioctl");
        close(sockfd);
        _exit(-1);
    }

    // 构造链路层地址
    struct sockaddr_ll sll = {0};
    sll.sll_ifindex = ethreq.ifr_ifindex;

    // 发送整个以太网帧
    if (sendto(sockfd, msg, len, 0,
               (struct sockaddr *)&sll, sizeof(sll)) < 0)
    {
        perror("sendto");
        close(sockfd);
        _exit(-1);
    }
}

// =====================
// 网络参数（手动填写）
// =====================
u_int8_t src_mac[6] = {0x00,0x0c,0x29,0x67,0x18,0xa8}; // Ubuntu MAC
u_int8_t dst_mac[6] = {0xb0,0x25,0xaa,0x4f,0xf3,0x67}; // Windows MAC

char src_ip[] = "10.7.170.26";
char dst_ip[] = "10.7.170.10";

unsigned short src_port = 8000;
unsigned short dst_port = 8080;

int main()
{
    char data[128] = "";

    printf("请输入需要发送的数据:");
    fgets(data, sizeof(data), stdin);
    data[strlen(data) - 1] = '\0';

    // UDP 数据长度必须为偶数（方便校验）
    int data_len = strlen(data) + strlen(data) % 2;

    // 创建原始套接字（二层）
    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    unsigned char msg[1500] = {0};

    // =====================
    // 1. 以太网头
    // =====================
    struct ether_header *eth_hdr = (struct ether_header *)msg;
    memcpy(eth_hdr->ether_dhost, dst_mac, 6);
    memcpy(eth_hdr->ether_shost, src_mac, 6);
    eth_hdr->ether_type = htons(0x0800); // IP

    // =====================
    // 2. IP 头
    // =====================
    struct iphdr *ip_hdr = (struct iphdr *)(msg + 14);
    ip_hdr->version  = 4;
    ip_hdr->ihl      = 5;
    ip_hdr->tos      = 0;
    ip_hdr->tot_len = htons(20 + 8 + data_len);
    ip_hdr->id       = 0;
    ip_hdr->frag_off = 0;
    ip_hdr->ttl      = 128;
    ip_hdr->protocol = 17; // UDP
    ip_hdr->check    = 0;
    ip_hdr->saddr    = inet_addr(src_ip);
    ip_hdr->daddr    = inet_addr(dst_ip);

    // 计算 IP 头校验
    ip_hdr->check = checksum((unsigned short *)ip_hdr, 20);

    // =====================
    // 3. UDP 头
    // =====================
    struct udphdr *udp_hdr = (struct udphdr *)(msg + 14 + 20);
    udp_hdr->source = htons(src_port);
    udp_hdr->dest   = htons(dst_port);
    udp_hdr->len    = htons(8 + data_len);
    udp_hdr->check  = 0;

    // 拷贝数据
    memcpy(msg + 14 + 20 + 8, data, data_len);

    // =====================
    // 4. UDP 校验（伪首部）
    // =====================
    unsigned char wei_buf[256] = {0};
    WEI_HEAD *p = (WEI_HEAD *)wei_buf;

    p->saddr   = inet_addr(src_ip);
    p->daddr   = inet_addr(dst_ip);
    p->flag    = 0;
    p->pro     = 17;
    p->tot_len = htons(8 + data_len);

    memcpy(wei_buf + 12, msg + 14 + 20, 8 + data_len);

    udp_hdr->check = checksum((unsigned short *)wei_buf,
                              12 + 8 + data_len);

    // =====================
    // 5. 发送
    // =====================
    my_sendto(sockfd, msg, 14 + 20 + 8 + data_len, "ens33");

    close(sockfd);
    return 0;
}
