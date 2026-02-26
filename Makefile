CFLAGS= -D__LINUX__ -c -Wall -O2
LIBS = -lm -lbb_scd41 -lbb_temperature -lpthread

all: sensor_scanner

sensor_scanner: main.o
	g++ main.o $(LIBS) -o sensor_scanner

main.o: main.cpp
	g++ $(CFLAGS) main.cpp

clean:
	rm *.o sensor_scanner
