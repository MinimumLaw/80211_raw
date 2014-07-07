CC=${CROSS_COMPILE}gcc
CFLAGS=-lpthread

all: sender receiver transmitter

sender: sender.c librawsocket.c libwireless.c libhexdump.c

receiver: receiver.c librawsocket.c libwireless.c libhexdump.c

transmitter: transmitter.c librawsocket.c libhexdump.c

clean:
	rm -rf *.o sender receiver transmitter