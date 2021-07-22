#include <iostream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "MACADDR.h"

#include "logger.h"

using namespace std;

#define ETHERTYPE_VLAN 0x8100
#define ETHERTYPE_QINQ 0x88a8
//#define VLAN_ASSIGN_CONN_TYPE AF_UNIX
#define VLAN_ASSIGN_CONN_TYPE AF_INET

struct vid_mess
{
  uint8_t be_haddr[ETHER_ADDR_LEN];	// MAC address
  uint32_t be_proto:16;		// protocol: either ETHERTYPE_VLAN (0x8100) or ETHERTYPE_QINQ (0x88a8)
  uint32_t be_vid:12;		// VLAN id
  uint32_t be_flags:4;		// flags - only one defined at the moment (differentiate between static/dynamic entry)
  int be_updat;			// last update time (unit: system ticks)
};

/* A bridge entry class */
class Bridge_entry
{
public:
  int src_interface;
  int vid;   // vid = -1 if unassigned
  MACADDR mac;    
  unsigned int ttl;
  Bridge_entry ()
  {
    src_interface = 0;
    vid = -1;
  }
  Bridge_entry (uint32_t src, uint32_t v)
  {
    src_interface = src;
    vid = v;
  }
};

/*  A structure for for a transparent bridge map with vlans */ 
class brmap
{

private:
  map < MACADDR, Bridge_entry > bridge;
  logger & _log;

public:
  int sockfd;
  int readsockfd;
  static brmap *thismap;
  brmap (logger & log);		// Constructor

  void print ();

  int map_pkt (int pkt_src, unsigned char *packet, int *vid);

  void add_vid (unsigned char *mac, uint32_t v);

  void start_listener (int port);
};
