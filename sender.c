#include <stdio.h>
#include "librawsocket.h"
#include "libwireless.h"
#include <linux/if_ether.h>

char ifname[16];
int sockraw;
int ret;

typedef struct {
    radiotap_header		rtp;
    u_int8_t			rate; /* on send we fill only rate field in this example */
    u_int32_t			reserved;
    ieee80211_qos_header	wlan;
    llc_header			snap;
    char		*payload;
} __attribute__((__packed__)) wlan_packet;

char buffer[128]; // min 46+14=60, max 1500+14=1524 (14 - ethernet header 2xMAC + TYPE)

wlan_packet *send_packet = (wlan_packet *)buffer;

unsigned char loc_ethaddr[ETH_ALEN];
unsigned char rem_ethaddr[ETH_ALEN];
unsigned char bssid[ETH_ALEN];

int main(char argc, char** argv, char** env)
{
    if(argc < 4)
    {
	fprintf(stderr,"Usage:\n\t%s <ifname> <remote_mac> <bssid>\n", argv[0]);
	return -1;
    }
    strncpy(ifname,argv[1],strlen(argv[1]));
    ret = sscanf(argv[2],"%02x:%02x:%02x:%02x:%02x:%02x",
	&rem_ethaddr[0],&rem_ethaddr[1],&rem_ethaddr[2],
	&rem_ethaddr[3],&rem_ethaddr[4],&rem_ethaddr[5]);
    if(ret<6)
    {
	fprintf(stderr,"Remote mac MUST be in human "
	    "readable form like 01:02:03:04:05:06\r\n");
	return -1;
    }

    ret = sscanf(argv[3],"%02x:%02x:%02x:%02x:%02x:%02x",
	&bssid[0],&bssid[1],&bssid[2],
	&bssid[3],&bssid[4],&bssid[5]);
    if(ret<6)
    {
	fprintf(stderr,"bssid MUST be in human "
	    "readable form like 12:24:56:78:9a:bc\r\n");
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
    bzero(buffer,sizeof(buffer));
    /* RADIOTAP */
    send_packet->rtp.it_version = 0;
    send_packet->rtp.it_pad = 0;
    send_packet->rtp.it_len = sizeof(radiotap_header) + 1 + 4; /* only rate append, se next line */
    send_packet->rtp.it_present = 0x00028004; /* we need only rate flag here */
    send_packet->rate = 0x02; /* 1Mbit/s - need to be preprocessor macros */
    send_packet->reserved = 0x00; /* 1Mbit/s - need to be preprocessor macros */
    /* IEEE802.11 with QOS */
    send_packet->wlan.frame_control = htons(0x8800);
    send_packet->wlan.duration_id = htons(0x3a01); /* ??? 314 microseconds ??? */
    memcpy(send_packet->wlan.dst, rem_ethaddr, ETH_ALEN);
    memcpy(send_packet->wlan.src, loc_ethaddr, ETH_ALEN);
    memcpy(send_packet->wlan.bssid, bssid, ETH_ALEN);
    send_packet->wlan.seq_ctrl = 0; /* ??? */
    send_packet->wlan.qos_ctrl = 0; /* QOS */
    /* LLC */
    send_packet->snap.dsap = 0xaa; /* ??? */
    send_packet->snap.ssap = 0xaa; /* ??? */
    send_packet->snap.control = 0x03; /* unnumbered frame */
    send_packet->snap.code[0] = 0x00; /* ??? */
    send_packet->snap.code[1] = 0x00; /* ??? */
    send_packet->snap.code[2] = 0x00; /* ??? */
    /* FixMe: snap.type _MUST_ be unspecified */
    send_packet->snap.type = htons(0x1234);
    sprintf(buffer + sizeof(radiotap_header) + 1 + 4 
		    + sizeof(ieee80211_qos_header) 
		    + sizeof(llc_header),
	"Hello world from wireless raw socket world!!! "
	"Be happy, don't worry!!!");
    ret = send_rawpacket(sockraw, buffer, sizeof(buffer));
    dump_80211_packet(buffer, sizeof(buffer));
    printf("Send %d bytes data to network\n", ret);

    return 0;
}