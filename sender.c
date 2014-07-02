#include <stdio.h>
#include "librawsocket.h"
#include <linux/if_ether.h>

char* ifname;
int sockraw;
int ret;

typedef struct {
    struct ethhdr	header;
    char		*payload;
} eth_packet;

char buffer[128]; // min 46+14=60, max 1500+14=1524 (14 - ethernet header 2xMAC + TYPE)

eth_packet *send_packet = (eth_packet *)buffer;

unsigned char loc_ethaddr[ETH_ALEN];
unsigned char rem_ethaddr[ETH_ALEN];

int main(char argc, char** argv, char** env)
{
    if(argc < 3)
    {
	fprintf(stderr,"Usage:\n\t%s <ifname> <remote_mac>\n", argv[0]);
	return -1;
    }
    ifname = argv[1];
    ret = sscanf(argv[2],"%02x:%02x:%02x:%02x:%02x:%02x",
	&rem_ethaddr[0],&rem_ethaddr[1],&rem_ethaddr[2],
	&rem_ethaddr[3],&rem_ethaddr[4],&rem_ethaddr[5]);
    if(ret<6)
    {
	fprintf(stderr,"Remote mac MUST be in human "
	    "readable form like 01:02:03:04:05:06\r\n");
	return -1;
    }

    if((sockraw = create_rawsocket(ETH_P_ALL)) == -1)
	return sockraw;

    { /* get local mac address */
	struct sockaddr_ll sll;
	struct ifreq ifr;
	int ret;

	bzero(&sll, sizeof(struct sockaddr_ll));
	bzero(&ifr, sizeof(struct ifreq));

	strncpy((char *)ifr.ifr_name, ifname, IFNAMSIZ);
	if((ret = ioctl(sockraw, SIOCGIFHWADDR, &ifr)) < 0)
	{
	    perror("Error get interface hardware address: ");
	    return ret;
	}
	memcpy(loc_ethaddr,ifr.ifr_addr.sa_data,ETH_ALEN);
	printf("Local ethernet address: %02x:%02x:%02x:%02x:%02x:%02x, "
	    "Remote ethernet address: %02x:%02x:%02x:%02x:%02x:%02x\n",
	    loc_ethaddr[0],loc_ethaddr[1],loc_ethaddr[2],
	    loc_ethaddr[3],loc_ethaddr[4],loc_ethaddr[5],
	    rem_ethaddr[0],rem_ethaddr[1],rem_ethaddr[2],
	    rem_ethaddr[3],rem_ethaddr[4],rem_ethaddr[5]);
    }

    if((ret = bind_rawsocket(ifname, sockraw,ETH_P_ALL)) == -1)
	return ret;

    /* send to client */
    memcpy(send_packet->header.h_source, loc_ethaddr, ETH_ALEN),
    memcpy(send_packet->header.h_dest, rem_ethaddr, ETH_ALEN),
    /* FixMe: h_proto _MUST_ be unspecified */
    send_packet->header.h_proto = htons(0x1234);
    ret = send_rawpacket(sockraw, send_packet, sizeof(buffer));
    printf("Send %d bytes data to network\n", ret);

    return 0;
}