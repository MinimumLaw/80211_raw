#include "libhexdump.c"
#include "librawsocket.h"

int create_rawsocket(int protocol_to_sniff)
{
    int raw_sock;

    if((raw_sock = socket(PF_PACKET, SOCK_RAW, htons(protocol_to_sniff))) == -1)
	perror("Error creating raw socket: ");
    return raw_sock;
}

int bind_rawsocket(char *device, int rawsock, int protocol)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;
    int ret;

    bzero(&sll, sizeof(struct sockaddr_ll));
    bzero(&ifr, sizeof(struct ifreq));

    /* get interface index */
    strncpy((char *)ifr.ifr_name, device, IFNAMSIZ);
    if((ret = ioctl(rawsock, SIOCGIFINDEX, &ifr)) < 0)
    {
	perror("Error get interface index: ");
	goto out;
    }

    /* bind to interface */
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);
    if((ret = bind(rawsock, (struct sockaddr *)&sll, sizeof(sll))) == -1)
    {
	perror("Error binding raw socket to interface: ");
	goto out;
    }

out:
    return ret;
}

int send_rawpacket(int rawsock, unsigned char *pkt, size_t pkt_len)
{
    size_t sent = 0;

    if((sent=write(rawsock, pkt, pkt_len)) != pkt_len)
    {
	perror("Eror sending raw socket data: ");
    }
    return sent;
}

int recv_rawpacket(int rawsock, unsigned char *pkt, size_t pkt_len)
{
    size_t recv = 0;

    recv = read(rawsock, pkt, pkt_len);
    return recv;
}
