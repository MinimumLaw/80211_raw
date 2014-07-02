CC=${CROSS_COMPILE}gcc

all: sender receiver

sender: sender.c librawsocket.c 
# libhexdump.c

receiver: receiver.c librawsocket.c 
# libhexdump.c

clean:
	rm -rf *.o sender receiver