CC      = g++
PREFIX  = /usr/local
#CFLAGS  =  -I ./include -I ../../libairware/include -O3 -fno-strict-aliasing -DJC_DEBUG
CFLAGS  =  -I ./include -I ../libairware/include -O3 -fno-strict-aliasing 
TARGET  = libjc-wepcrack.a
LDFLAGS = -lpcap -lcrypto -lairware

all: libjc-wepcrack.a

libjc-wepcrack.a: key.o keyspacechunk.o jcprotocol.o
	ar -cru libjc-wepcrack.a  key.o keyspacechunk.o jcprotocol.o
	ranlib ./libjc-wepcrack.a

key.o: ./src/key.cpp
	$(CC) $(CFLAGS) -c ./src/key.cpp -o key.o 
keyspacechunk.o: ./src/keyspacechunk.cpp
	$(CC) $(CFLAGS) -c ./src/keyspacechunk.cpp -o keyspacechunk.o 
jcprotocol.o: ./src/jcprotocol.cpp
	$(CC) $(CFLAGS) -c ./src/jcprotocol.cpp -o jcprotocol.o

install:
	install -m 755 -o root -g wheel $(TARGET) $(PREFIX)/bin 

clean:
	rm -f *.o $(TARGET)
