#include "librawsocket.h"
#include "libwireless.h"
#include "libhexdump.h"
#include <time.h> // not ned, for nanosleep() only
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

/* global definition */
char*	ifname;
int	rwsock;

/* FixMe: need got this info from socket */
const size_t	iface_mtu = 1500;
#ifdef ACKGEN
unsigned char loc_mac[]	= {0x12,0x34,0x56,0x78,0x9a,0xbc};
#else
unsigned char loc_mac[]	= {0x01,0x23,0x45,0x67,0x89,0xab};
#endif

#ifdef ACKGEN
unsigned char rem_mac[]	= {0x01,0x23,0x45,0x67,0x89,0xab};
#else
unsigned char rem_mac[]	= {0x12,0x34,0x56,0x78,0x9a,0xbc};
#endif
unsigned char bssid[] 	= {0xde,0xad,0xca,0xfe,0xbe,0xaf};

pthread_t pid_send, pid_recv;
int ret;

void* receiver(void* ifname)
{
    char*	rcv_buff;
    size_t	pkt_size;
    radiotap_header*	rtap;

    rcv_buff = malloc(iface_mtu);
    if(rcv_buff == NULL) {
	perror("Recieve buffer:");
	exit(errno);
    }

    do { /* recever loop */
	radiotap_header*	rt = (radiotap_header*)rcv_buff;
	ieee80211_qos_header*	wl;
	llc_header*		llc;
	char*			payload;
	size_t			payload_size;

	pkt_size = recv_rawpacket(rwsock, rcv_buff, iface_mtu);
	wl = (ieee80211_qos_header*)(rcv_buff + rt->it_len);
	llc = (llc_header *)(rcv_buff + rt->it_len + sizeof(ieee80211_qos_header));
	payload = (char *)(rcv_buff + rt->it_len + sizeof(ieee80211_qos_header) + sizeof(llc_header));
	payload_size = pkt_size - rt->it_len - sizeof(ieee80211_qos_header) - sizeof(llc_header);
	if( payload_size > 0 ) {
	    if( (llc->dsap == 0xaa) && (llc->ssap == 0xaa) && (llc->type == htons(0x1234)) ) {
		fprintf(stderr,"R");
		hexdump("Packet", rcv_buff, 0x50);
	    } else {
		fprintf(stderr,"r");
	    }
	} else {
	    fprintf(stderr,"r");
	}
    } while(pkt_size != -1);
    fprintf(stderr,"We _MUST_ not run this code, if no error present\r\n");
    exit(-1);
}

void* transmitter(void* ifname)
{
    char*	snd_buff;
    int		fd;

    snd_buff = malloc(iface_mtu);
    if(snd_buff == NULL) {
	perror("Send buffer:");
	exit(errno);
    }

    fd = open("/dev/urandom",O_RDONLY);
    if(fd == -1) {
	perror("Random data file:");
	exit(errno);
    }

    while(1) {
	/* Basic headers for wireless link */
	radiotap_tx_header*	rt = (radiotap_tx_header*)snd_buff;
	ieee80211_qos_header*	wl = \
	    (ieee80211_qos_header*)(snd_buff + 
	    sizeof(radiotap_tx_header));
	llc_header*		llc = (llc_header *)(snd_buff + 
	    sizeof(radiotap_tx_header) + 
	    sizeof(ieee80211_qos_header));
	char*			payload = (char *)(snd_buff + 
	    sizeof(radiotap_tx_header) + 
	    sizeof(ieee80211_qos_header) +
	    sizeof(llc_header));
	size_t			payload_size = \
	    iface_mtu - sizeof(radiotap_tx_header) - \
	    sizeof(ieee80211_qos_header) - \
	    sizeof(llc_header);

	bzero(snd_buff, iface_mtu);
	/* radiotap configuration */
	rt->it_len = sizeof(radiotap_tx_header);
	rt->it_present = RT_RATE_FLAG;
	rt->rate = RATE_TO_RADIOTAP(5500);
	/* ieee802.11 header */
	wl->frame_control = htons(0x8800); /*!!!*/
	memcpy(wl->src, loc_mac, ETH_ALEN);
	memcpy(wl->dst, rem_mac, ETH_ALEN);
	memcpy(wl->bssid, bssid, ETH_ALEN);
	/* llc header */
	llc->dsap = 0xaa; /*!!!*/
	llc->ssap = 0xaa; /*!!!*/
	llc->control = 0x03; /*!!!*/
	llc->type = htons(0x1234);

	/* put data payload */
	if( read(fd, payload, payload_size) != payload_size ) {
	    perror("Random data read");
	    exit(errno);
	}

	ret = send_rawpacket(rwsock, snd_buff, iface_mtu);
	if(ret != iface_mtu) {
	    perror("Packet send");
	    exit(-2);
	}
	fprintf(stderr,"t");

	/* sleep for 1 sec. */
	struct timespec rqtp, rmtp;
	rqtp.tv_sec = 1;
	rqtp.tv_nsec = 0;
	nanosleep(&rqtp, &rmtp);
    }
    fprintf(stderr,"We _MUST_ not run this code, if no error present\r\n");
    exit(-2);
}

int main(char argc, char** argv, char** env)
{
    if (argc < 2) {
	fprintf(stderr,"no iface name as arg present\n");
	exit(-4);
    }

    rwsock = create_rawsocket(ETH_P_ALL);
    if(rwsock == -1) {
	perror("Creat raw socket:");
	exit(-5);
    }

    ifname = argv[1];
    ret = bind_rawsocket(ifname, rwsock, ETH_P_ALL);
    if(ret == -1) {
	perror("Bind raw socket:");
	exit(-6);
    }

    ret = pthread_create(&pid_recv, NULL, &receiver, (void*)argv[1]);
    if(ret){
	perror("Create receive pthread");
	return errno;
    }
    pid_send = pthread_self();
    fprintf(stdout,"Starting receive (pid: %lu) and send (pid: %lu) pthreads\n",
	(unsigned long)pid_recv, (unsigned long)pid_send);
    transmitter((void*)argv[1]);

    fprintf(stderr,"We _MUST_ not run this code, if no error present\r\n");
    return -3;
}