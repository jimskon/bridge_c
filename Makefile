# Make bridge_c

PROGRAMS = bridge testbr

SRCFILES = $(wildcard *.cpp)
OBJFILES = $(patsubst %.cpp,%.o,$(SRCFILES))

CXX      = g++
CPPFLAGS = -I.
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 $(CPPFLAGS)
CHECK    = cppcheck -q --enable=warning $(CPPFLAGS)
RM       = rm -f


all: $(PROGRAMS)

bridge: bridge.o brmap.o iface.o logger.o
	$(CXX) -o $@ $^

testbr: testbr.o brmap.o logger.o
	$(CXX) -o $@ $^

clean:
	$(RM) $(PROGRAMS) $(OBJFILES)

check:
	$(CHECK) $(SRCFILES)

.PHONY: all clean check


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

