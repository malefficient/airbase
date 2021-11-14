CC      = g++
PREFIX  = /usr/local/
AIRWAREPATH = ../libairware
#CFLAGS  =   -I./include -O3 -fno-strict-aliasing  -I/
CFLAGS  =   -I./include -O3 -fno-strict-aliasing  -I$(AIRWAREPATH)/include
TARGET  = lib802finger.a



all: lib802finger.a 802finger-test

lib802finger.a: fingerlib-datatypes.o duration-print.o comparators.o databasewidestats.o  coefficient-table.o implementation-record.o station.o
	ar -cru lib802finger.a fingerlib-datatypes.o duration-print.o comparators.o databasewidestats.o coefficient-table.o implementation-record.o station.o
	ranlib ./lib802finger.a

station.o: ./src/station.cpp
	$(CC) $(CFLAGS) -c ./src/station.cpp -o station.o 

duration-print.o: ./src/duration-print.cpp
	$(CC) $(CFLAGS) -c ./src/duration-print.cpp -o duration-print.o 

comparators.o: ./src/comparators.cpp
	$(CC) $(CFLAGS) -c ./src/comparators.cpp -o comparators.o 


implementation-record.o: ./src/implementation-record.cpp
	$(CC) $(CFLAGS) -c ./src/implementation-record.cpp -o implementation-record.o 

databasewidestats.o: ./src/databasewidestats.cpp
	$(CC) $(CFLAGS) -c ./src/databasewidestats.cpp -o databasewidestats.o 

coefficient-table.o: ./src/coefficient-table.cpp
	$(CC) $(CFLAGS) -c ./src/coefficient-table.cpp -o coefficient-table.o 

fingerlib-datatypes.o: ./src/fingerlib-datatypes.cpp
	$(CC) $(CFLAGS) -c ./src/fingerlib-datatypes.cpp -o fingerlib-datatypes.o 


802finger-test: ./src/802fingertest.cpp
	$(CC) $(CFLAGS)  ./src/802fingertest.cpp -o 802fingertest -L. -L$(AIRWAREPATH) -l802finger -lairware -lcrypto -lpcap 
station-lister: ./src/station-lister.cpp
	$(CC) $(CFLAGS)  ./src/station-lister.cpp -o station-lister -L. -L$(AIRWAREPATH) -l802finger -lairware -lcrypto -lpcap 

clean:
	rm -f *.o 802fingertest $(TARGET)
