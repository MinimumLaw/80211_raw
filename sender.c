#include <stdio.h>
#include "librawsocket.h"
#include "libwireless.h"
#include <linux/if_ether.h>

char *ifname;
int sockraw;
int ret;

typedef struct {
    radiotap_header		rtp;
    u_int8_t			rate; /* on send we fill only rate field in this example */
    u_int8_t			tx_pwr;
    u_int32_t			reserved;
    ieee80211_qos_header	wlan;
    llc_header			snap;
    char		*payload;
} __attribute__((__packed__)) wlan_packet;

char buffer[128];

wlan_packet *send_packet = (wlan_packet *)buffer;

unsigned char loc_ethaddr[ETH_ALEN];
unsigned char rem_ethaddr[ETH_ALEN];
unsigned char bssid[ETH_ALEN];

int main(char argc, char** argv, char** env)
{
    if(argc < 2)
    {
	fprintf(stderr,"Usage:\n\t%s <ifname> <remote_mac> <bssid>\n", argv[0]);
	return -1;
    }
    ifname = argv[1];

    /* no need in this address - we may use this fields as need */
    bzero(loc_ethaddr,ETH_ALEN);
    bzero(rem_ethaddr,ETH_ALEN);
    bzero(bssid,ETH_ALEN);

    if((sockraw = create_rawsocket(ETH_P_ALL)) == -1)
	return sockraw;

    if((ret = bind_rawsocket(ifname, sockraw,ETH_P_ALL)) == -1)
	return ret;

    /* send to client */
    bzero(buffer,sizeof(buffer));
    /* RADIOTAP */
    send_packet->rtp.it_version = 0;
    send_packet->rtp.it_pad = 0;
    send_packet->rtp.it_len = sizeof(radiotap_header) + 2 + 4; /* only rate append, se next line */
    send_packet->rtp.it_present = RT_RATE_FLAG | RT_TXDB_FLAG; /* we need only rate flag here */
    send_packet->rate = RATE_TO_RADIOTAP(1000); /* 1Mbit/s or 1000Kbit/s */
    send_packet->tx_pwr = 20; /* 20 dBm */
    send_packet->reserved = 0x00; /* ??? */
    /* IEEE802.11 with QOS */
    send_packet->wlan.frame_control = htons(0x8800);
    send_packet->wlan.duration_id = 0; /* htons(0x3a01);  ??? 314 microseconds ??? */
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
    sprintf(buffer + sizeof(radiotap_header) + 2 + 4 
		    + sizeof(ieee80211_qos_header) 
		    + sizeof(llc_header),
	"Hello world from wireless raw socket world!!! "
	"Be happy, don't worry!!!");
    ret = send_rawpacket(sockraw, buffer, sizeof(buffer));
    dump_80211_packet(buffer, sizeof(buffer));
    printf("Send %d bytes data to network\n", ret);

    return 0;
}