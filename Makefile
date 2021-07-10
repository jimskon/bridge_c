# Make bridge_c

CXX = g++
RM  = rm -f

PROGRAMS = testbr bridge_c
CXXFLAGS = -Wall -Wextra -std=c++11 -O2


all: $(PROGRAMS)

bridge_c: bridge_c.o brmap.o
	$(CXX) -o $@ $^

testbr: testbr.o brmap.o
	$(CXX) -o $@ $^

clean:
	$(RM) *.o $(PROGRAMS)

.PHONY: clean


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I. -o $@ -c $<

