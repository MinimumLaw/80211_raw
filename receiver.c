#include "librawsocket.h"
#include "libhexdump.h"
#include "libwireless.h"
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
	    dump_80211_packet(pkt, ret);
    }

    return 0;
}
