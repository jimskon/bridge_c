# Make bridge_c

cc= g++
cflags= -O2

all: bridge_c testbr

brmap.o: brmap.cpp brmap.h
	$(cc) $(cflags) brmap.cpp -c

bridge_c.o: bridge_c.cpp brmap.h
	$(cc) $(cflags) bridge_c.cpp -c

bridge_c: brmap.o bridge_c.o
	$(cc) brmap.o bridge_c.o -o bridge_c -lpthread

testbr.o: testbr.cpp brmap.h
	$(cc) $(cflags) testbr.cpp -c

testbr: testbr.o brmap.o
	$(cc) testbr.o brmap.o -o testbr

clean:
	rm -f *.o testbr bridge_c
