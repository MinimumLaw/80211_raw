#ifndef _INCLUDE_LIBWIRELESS_H_
#define _INCLUDE_LIBWIRELESS_H_

#include <stdint.h>

#define RT_FLAG_FLAG		(1<<1)
#define RT_RATE_FLAG		(1<<2)
#define RT_CHAN_FLAG		(1<<3)
#define RT_TXDB_FLAG		(1<<10)
#define RT_ANTN_FLAG		(1<<11)

#define RT_TXCFG_FLAGS	(RT_FLAG_FLAG |\
			 RT_RATE_FLAG | RT_CHAN_FLAG |\
			 RT_TXDB_FLAG | RT_ANTN_FLAG)

#define RATE_TO_RADIOTAP(X) (X/500)
#define RADIOTAP_TO_RATE(X) (X*500)

typedef struct {
    u_int8_t	it_version;
    u_int8_t	it_pad;
    __le16	it_len;
    __le32	it_present;
} __attribute__((__packed__)) radiotap_header;

typedef struct {
    u_int8_t	it_version;
    u_int8_t	it_pad;
    __le16	it_len; /* sizeof(radiotap_tx_header) */
    __le32	it_present; /* always RT_TXCFG_FLAGS */
} __attribute__((__packed__)) radiotap_tx_header;

typedef struct {
	__le16	frame_control;
	__le16	duration_id;
	u_int8_t	dst[ETH_ALEN];
	u_int8_t	src[ETH_ALEN];
	u_int8_t	bssid[ETH_ALEN];
	__le16	seq_ctrl;
	__le16	qos_ctrl;
} __attribute__((__packed__)) ieee80211_qos_header;

typedef struct {
    u_int8_t	dsap;
    u_int8_t	ssap;
    u_int8_t	control;
    u_int8_t	code[3]; /* FixMe: why??? */
    u_int16_t	type;
} __attribute__((__packed__)) llc_header;

void dump_80211_packet(char* pkt, size_t len);

#endif
