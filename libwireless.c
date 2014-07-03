#include "librawsocket.h"
#include "libwireless.h"
#include "libhexdump.h"

void dump_80211_packet(char* pkt, size_t len)
{
    radiotap_header *rtp = (radiotap_header *)pkt;
    ieee80211_qos_header *wlan = (ieee80211_qos_header *)(pkt+rtp->it_len);
    llc_header *llc = NULL;
    if (wlan->frame_control == 0x0088)
    {
	llc = (llc_header *)(pkt+rtp->it_len+sizeof(ieee80211_qos_header));
	if( htons(llc->type) == 0x1234 ) {
		size_t headers_size = rtp->it_len +
			sizeof(ieee80211_qos_header) +
			sizeof(llc_header);

		printf("\nRADIOTAP: vers:0x%02X, len: %d, flags: 0x%04X\n",
		rtp->it_version, rtp->it_len, rtp->it_present);
		printf("IEEE802.11 src=%02x:%02x:%02x:%02x:%02x:%02x, "
		"dst=%02x:%02x:%02x:%02x:%02x:%02x bssid==%02x:%02x:%02x:%02x:%02x:%02x\n",
		wlan->src[0],wlan->src[1],wlan->src[2],wlan->src[3],wlan->src[4],wlan->src[5],
		wlan->dst[0],wlan->dst[1],wlan->dst[2],wlan->dst[3],wlan->dst[4],wlan->dst[5],
		wlan->bssid[0],wlan->bssid[1],wlan->bssid[2],wlan->bssid[3],wlan->bssid[4],wlan->bssid[5]);
		printf("LLC: type 0x%04X\n", htons(llc->type));
		hexdump("data:", pkt+headers_size, len-headers_size);
		hexdump("full packet:", pkt, len);
		return;
	}
	/* Data with not our link type - simple show asterisk */
	printf("*");
    } else {
	/* No data (management) frame - simple show dot */
	printf("!");
    }
    fflush(stdout);
}
