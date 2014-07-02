#include "librawsocket.h"
#include "libhexdump.h"
#include <linux/if_ether.h>

char* ifname;
int sockraw;
int ret;

char pkt[8192];

int main(char argc, char** argv, char** env)
{
    if(argc < 2)
    {
	fprintf(stderr,"Usage:\n\t%s <ifname>\n", argv[0]);
	return -1;
    }
    ifname = argv[1];

    if((sockraw = create_rawsocket(ETH_P_ALL)) == -1)
	return sockraw;


    if((ret = bind_rawsocket(ifname, sockraw,ETH_P_ALL)) == -1)
	return ret;

    while(1){
	if ((ret = recv_rawpacket(sockraw, pkt, sizeof(pkt))) > -1)
	{
	    struct ethhdr *hdr = (struct ethhdr *)pkt;
	    if(hdr->h_proto == htons(0x1234)) {
		printf("Receive packet (len %d)\n", ret);
		printf("\tFrom: %02X:%02X:%02X:%02X:%02X:%02X\n",
		    hdr->h_source[0],hdr->h_source[1],hdr->h_source[2],
		    hdr->h_source[3],hdr->h_source[4],hdr->h_source[5]);
		printf("\tTo: %02X:%02X:%02X:%02X:%02X:%02X\n",
		    hdr->h_dest[0],hdr->h_dest[1],hdr->h_dest[2],
		    hdr->h_dest[3],hdr->h_dest[4],hdr->h_dest[5]);
		printf("\tPROTO: 0x%04X\n", htons(hdr->h_proto));
		hexdump("packet:", &pkt, ret);
	    }
	}
    }

    return 0;
}
