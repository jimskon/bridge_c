
#include <iostream>
#include "brmap.h"

void printpacket(const char* msg, const unsigned char* p, size_t len) {
    size_t i;
    cout << msg << " len=" << len << endl;
    for(i=0; i<len; ++i) {
      cout << std::hex << (int) p[i];
    }
    cout << std::dec << endl;
}


int main() {
    srand(time(NULL) + getpid());
    unsigned char pkt1[] = {0x42,0x56,0x34,0x45,0x23,0x56,0x32,0x56,0x67,0x32,0x56,0x34,0x34,0x76,0x34,0x04,0x23,0x13,0x75,0x76};  
    unsigned char pkt2[] = {0x32,0x56,0x67,0x32,0x56,0x34,0x42,0x56,0x34,0x45,0x23,0x56,0x56,0x76,0x34,0x04,0x23,0x13,0x75,0x76};  
    unsigned char pkt3[] = {0x58,0x55,0x55,0x34,0x56,0x34,0x32,0x56,0x67,0x32,0x56,0x34,0x58,0x55,0x55,0x34,0x56,0x34,0x58,0x56,0x34,0x45,0x23,0x56,0x56,0x76,0x34,0x04,0x23,0x13,0x75,0x76};  
    unsigned char pkt4[] = {0xAA,0x55,0x55,0x34,0x56,0x34,0xAC,0x56,0x34,0x45,0x23,0x56,0x56,0x76,0x34,0x04,0x23,0x13,0x75,0x76};
    unsigned char pkt[128] = {0};

    MACADDR a,b;
    brmap bridge;

    bridge.map_pkt(1,pkt1);
    bridge.map_pkt(2,pkt2);
    bridge.map_pkt(3,pkt3);
    bridge.map_pkt(4,pkt4);
    for (int i=0;i<1000000;i++){
      MACADDR a,b;
      a.random_mac();
      b.random_mac();
      //a.print();
      //b.print();
      for (int j=0; j<2;j++) {
	pkt[j]=a.mac[j];
	pkt[j+6]=b.mac[j];
      }
      short inter=rand()%16;
      bridge.map_pkt(inter,pkt);
      //printpacket("pkt",pkt,12);
      //cout << "Inter:" << inter << endl;
      //cout << "TO:" << bridge.map_pkt(inter,pkt) << endl;
    }
    //printpacket("pkt1",pkt,12);
    //cout << bridge.map_pkt(3,pkt1) << endl;
    //printpacket("pkt2",pkt,12);
    //cout << bridge.map_pkt(2,pkt2) << endl;
    //printpacket("pkt3",pkt,12);
    //cout << bridge.map_pkt(6,pkt3) << endl;
    //printpacket("pkt4",pkt,12);
    //cout << bridge.map_pkt(2,pkt4) << endl;

    //bridge.print();
}
