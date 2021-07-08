# Make bridge_c

cc= g++
cflags= -O2

all: bridge_c testbr

trans_bridge.o: trans_bridge.cpp trans_bridge.h
	$(cc) $(cflags) trans_bridge.cpp -c

bridge_c.o: bridge_c.cpp trans_bridge.h
	$(cc) $(cflags) bridge_c.cpp -c

bridge_c: trans_bridge.o bridge_c.o
	$(cc) trans_bridge.o bridge_c.o -o bridge_c

testbr.o: testbr.cpp trans_bridge.h
	$(cc) $(cflags) testbr.cpp -c

testbr: testbr.o trans_bridge.o
	$(cc) testbr.o trans_bridge.o -o testbr

clean:
	rm -f *.o testbr bridge_c
