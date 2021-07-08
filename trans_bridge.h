#include <iostream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

/* A mac address class */
class MACADDR {
  public:
    unsigned char mac[6]={0};
    MACADDR() {}
    MACADDR(unsigned char a[6]) {
      for (int i=0; i<6; i++) {
        mac[i]=a[i];
      }
    }
    
    void set(unsigned char a[6]) {
      for (int i=0; i<6; i++) {
        mac[i]=a[i];
      }
    }
    bool operator<(const MACADDR a) const {
      for (int i=0; i<6; i++) {
        if (a.mac[i] < this->mac[i])
          return false;
        if (a.mac[i] > this->mac[i])
          return true;
      }
      return false;
    }
    bool operator=(const MACADDR a) const {
      for (int i=0; i<6; i++) {
        if (a.mac[i]!=this->mac[i])
          return false;
      }
      return true;
    }
    void print() const {
      cout << hex;
      for (int i=0; i<6; i++) {
        cout << (int) this->mac[i] << ' ';
      }
      cout << dec;
    }
    bool is_broadcast() {
      for (int i=0; i<6; i++) {
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
        for (int i=0; i<6; i++) {
          mac[i]=packet[i];
        }
    }

    void get_src_mac(unsigned char *packet)   {
        for (int i=0; i<6; i++) {
          mac[i]=packet[i+6];
        }
    }

  void random_mac() {
    int i,tp;
    for (i = 0; i < 6; i++) {
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

class trans_bridge {

 private:
    map<MACADDR,Bridge_entry> bridge;

 public:
    
    trans_bridge(); // Constructor
  
    void print();

    int bridge_packet(int pkt_src, unsigned char *packet);

};
