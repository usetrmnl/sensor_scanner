CFLAGS= -D__LINUX__ -c -Wall -O2
LIBS = -lm -lbb_scd41 -lpthread

all: sensor2json

sensor2json: main.o
	g++ main.o $(LIBS) -o sensor2json

main.o: main.cpp
	g++ $(CFLAGS) main.cpp

clean:
	rm *.o sensor2json
