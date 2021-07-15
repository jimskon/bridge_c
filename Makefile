
PROGRAMS  = bridge testbr runtests

SRCPDU    = $(wildcard pdu/*.cpp)
OBJPDU    = $(patsubst %.cpp,%.o,$(SRCPDU))
LIBPDU    = pdu/libpdu.a

SRCEVENT  = $(wildcard event/*.cpp)
OBJEVENT  = $(patsubst %.cpp,%.o,$(SRCEVENT))
LIBEVENT  = event/libevent.a

SRCTESTS  = $(wildcard tests/*.cpp)
OBJTESTS  = $(patsubst %.cpp,%.o,$(SRCTESTS))

SRCFILES  = $(SRCPDU) $(wildcard *.cpp)
OBJFILES  = $(patsubst %.cpp,%.o,$(SRCFILES))

LIBRARIES = $(LIBPDU)

CXX      := g++
CPPFLAGS  = -I.
#ARMFLAGS  = -marm #-march=armv7-a
ARMFLAGS  = -marm
CXXFLAGS  = -Wall -Wextra -std=c++11 -O3 $(ARMFLAGS) $(CPPFLAGS)
#CXXFLAGS  += -g
CHECK     = cppcheck -q --enable=style,warning $(CPPFLAGS)
CTAGS     = ctags -a
AR        = ar rcs
RM        = rm -f


all: $(PROGRAMS)

bridge: main.o ifbridge.o brmap.o iface.o icmp4.o macaddr.o $(LIBRARIES)
	$(CXX) -o $@ $^ -lpthread

testbr: testbr.o brmap.o
	$(CXX) -o $@ $^ -lpthread

testsend: testsend.o brmap.o
	$(CXX) -o $@ $^ -lpthread

runtests: $(OBJTESTS) macaddr.o $(LIBRARIES)
	$(CXX) -o $@ $^

$(LIBPDU): $(OBJPDU)
	$(AR) $@ $^

$(LIBEVENT): $(OBJEVENT)
	$(AR) $@ $^

clean:
	$(RM) $(PROGRAMS) $(LIBRARIES)
	$(RM) $(OBJFILES)

check:
	$(CHECK) $(SRCFILES)

tags:
	find /usr/include -type f -name "*.h" | xargs $(CTAGS)

.PHONY: all clean check tags


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

