CC      = g++
PREFIX  = /usr/local/lib
#CFLAGS  =  -I./include -Wno-deprecated -g  -DJC_DEBUG -Wall -fno-strict-aliasing
CFLAGS  =   -I./include -O3 -fno-strict-aliasing 
TARGET  = libairware.a

all: libairware.a

libairware.a: airware-radiotap.o pcap-packet.o packet80211.o packet80211-data.o packet80211-ctrl.o packet80211-mgmt.o packet80211-data-crafter.o   packet80211-ctrl-crafter.o packet80211-mgmt-crafter.o packet80211-crafter.o jc-util.o wep-crypt.o misc.o crack.o timepiece.o packet_list.o basic-pcap-thread.o
	ar -cru libairware.a airware-radiotap.o pcap-packet.o packet80211.o  wep-crypt.o\
   	packet80211-data.o  packet80211-ctrl.o packet80211-mgmt.o  packet80211-data-crafter.o packet80211-ctrl-crafter.o packet80211-mgmt-crafter.o packet80211-crafter.o jc-util.o misc.o crack.o timepiece.o packet_list.o basic-pcap-thread.o
	ranlib ./libairware.a

airware-radiotap.o: ./src/airware-radiotap.cpp
	$(CC) $(CFLAGS) -c ./src/airware-radiotap.cpp -o airware-radiotap.o 
packet_list.o: ./src/packet_list.cpp
	$(CC) $(CFLAGS) -c ./src/packet_list.cpp -o packet_list.o 
timepiece.o: ./src/timepiece.cpp
	$(CC) $(CFLAGS) -c ./src/timepiece.cpp -o timepiece.o 
jc-util.o: ./src/jc-util.cpp
	$(CC) $(CFLAGS) -c ./src/jc-util.cpp -o jc-util.o 
pcap-packet.o: ./src/pcap-packet.cpp
	$(CC) $(CFLAGS) -c ./src/pcap-packet.cpp -o pcap-packet.o
packet80211.o: ./src/packet80211.cpp
	$(CC) $(CFLAGS) -c ./src/packet80211.cpp -o packet80211.o
packet80211-data.o: ./src/packet80211-data.cpp
	$(CC) $(CFLAGS) -c ./src/packet80211-data.cpp -o packet80211-data.o 
packet80211-ctrl.o: ./src/packet80211-ctrl.cpp
	$(CC) $(CFLAGS) -c ./src/packet80211-ctrl.cpp -o packet80211-ctrl.o 
packet80211-mgmt.o: ./src/packet80211-mgmt.cpp
	$(CC) $(CFLAGS) -c ./src/packet80211-mgmt.cpp -o packet80211-mgmt.o 
packet80211-data-crafter.o: ./src/packet80211-data-crafter.cpp
	$(CC) $(CFLAGS) -c ./src/packet80211-data-crafter.cpp -o packet80211-data-crafter.o 
packet80211-ctrl-crafter.o: ./src/packet80211-ctrl-crafter.cpp
	$(CC) $(CFLAGS) -c ./src/packet80211-ctrl-crafter.cpp -o packet80211-ctrl-crafter.o 
packet80211-mgmt-crafter.o: ./src/packet80211-mgmt-crafter.cpp
	$(CC) $(CFLAGS) -c ./src/packet80211-mgmt-crafter.cpp -o packet80211-mgmt-crafter.o 
packet80211-crafter.o: ./src/packet80211-crafter.cpp
	$(CC) $(CFLAGS) -c ./src/packet80211-crafter.cpp -o packet80211-crafter.o 
wep-crypt.o: ./src/wep-crypt.cpp
	$(CC) $(CFLAGS) -c ./src/wep-crypt.cpp -o wep-crypt.o
misc.o: ./src/misc.cpp
	$(CC) $(CFLAGS) -c ./src/misc.cpp -o misc.o
crack.o: ./src/crack.cpp
	$(CC) $(CFLAGS) -c ./src/crack.cpp -o crack.o
basic-pcap-thread.o: ./src/basic-pcap-thread.cpp
	$(CC) $(CFLAGS) -c ./src/basic-pcap-thread.cpp -o basic-pcap-thread.o

install:
	install -m 755 $(TARGET) $(PREFIX)

clean:
	rm -f *.o $(TARGET)
