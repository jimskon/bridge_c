#include <iostream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include<net/ethernet.h>
using namespace std;

/* A mac address class */
class MACADDR {
  public:
    unsigned char mac[ETH_ALEN]={0};
    MACADDR() {}
    MACADDR(unsigned char a[ETH_ALEN]) {
      for (int i=0; i<ETH_ALEN; i++) {
        mac[i]=a[i];
      }
    }
    
    void set(unsigned char a[ETH_ALEN]) {
      for (int i=0; i<ETH_ALEN; i++) {
        mac[i]=a[i];
      }
    }
    bool operator<(const MACADDR a) const {
      for (int i=0; i<ETH_ALEN; i++) {
        if (a.mac[i] < this->mac[i])
          return false;
        if (a.mac[i] > this->mac[i])
          return true;
      }
      return false;
    }
    bool operator=(const MACADDR a) const {
      for (int i=0; i<ETH_ALEN; i++) {
        if (a.mac[i]!=this->mac[i])
          return false;
      }
      return true;
    }
    void print() const {
      cout << hex;
      for (int i=0; i<ETH_ALEN; i++) {
        cout << (int) this->mac[i] << ' ';
      }
      cout << dec;
    }
    bool is_broadcast() {
      for (int i=0; i<ETH_ALEN; i++) {
        if (mac[i]!=0xff) return false;
      }
      return true;
    }

    bool is_multicast() {

      if (mac[0] & 0x01) {
      return true;
      }
    return false;
    }

    void get_dest_mac(unsigned char *packet)   {
        for (int i=0; i<ETH_ALEN; i++) {
          mac[i]=packet[i];
        }
    }

    void get_src_mac(unsigned char *packet)   {
        for (int i=0; i<ETH_ALEN; i++) {
          mac[i]=packet[i+ETH_ALEN];
        }
    }

  void random_mac() {
    int i;
    for (i = 0; i < ETH_ALEN; i++) {
      mac[i] = rand() % 256;
    }
  }
};


/* A bridge entry class */
class Bridge_entry {
  public:
    short src_interface;
    short vid;
    MACADDR mac;
    unsigned int ttl;
    Bridge_entry(short src, short v) {
      src_interface = src;
      vid = v;
    }
    Bridge_entry(short src) {
      src_interface = src;
      vid = 0;
    }
    Bridge_entry() {
      src_interface = 0;
      vid = 0;
    }

};

class brmap {

 private:
    map<MACADDR,Bridge_entry> bridge;

 public:
    
    brmap(); // Constructor
  
    void print();

    int map_pkt(int pkt_src, unsigned char *packet);

};
