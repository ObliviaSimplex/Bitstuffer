CC = gcc
CFLAGS = -g

all : CRC bitstuff

CRC : CRC.o

bitstuff : bitstuff.o

CRC.o : bitops.h

bitstuff.o : bitops.h

clean :
	rm CRC bitstuff *.o 






























