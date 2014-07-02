#ifndef _INCLUDE_LIBRAWSOCKET_H_
#define _INCLUDE_LIBRAWSOCKET_H_

#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/sockios.h>
#include <string.h>

int create_rawsocket(int protocol_to_sniff);
int bind_rawsocket(char *device, int rawsock, int protocol);
int send_rawpacket(int rawsock, unsigned char *pkt, size_t pkt_len);
int recv_rawpacket(int rawsock, unsigned char *pkt, size_t pkt_len);

#endif