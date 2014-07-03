CC=${CROSS_COMPILE}gcc

all: sender receiver

sender: sender.c librawsocket.c libwireless.c libhexdump.c

receiver: receiver.c librawsocket.c libwireless.c libhexdump.c

clean:
	rm -rf *.o sender receiver