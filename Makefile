# Make bridge_c

PROGRAMS = bridge testbr

SRCFILES = $(wildcard *.cpp)
OBJFILES = $(patsubst %.cpp,%.o,$(SRCFILES))

CXX     := g++
CPPFLAGS = -I.
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 $(CPPFLAGS)
CHECK    = cppcheck -q --enable=style,warning $(CPPFLAGS)
CTAGS    = ctags -a
RM       = rm -f


all: $(PROGRAMS)

bridge: bridge.o brmap.o iface.o packet.o packet_eth.o icmp4.o macaddr.o
	$(CXX) -o $@ $^

testbr: testbr.o brmap.o
	$(CXX) -o $@ $^

clean:
	$(RM) $(PROGRAMS) $(OBJFILES)

check:
	$(CHECK) $(SRCFILES)

tags:
	find /usr/include -type f -name "*.h" | xargs $(CTAGS)

.PHONY: all clean check tags


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

