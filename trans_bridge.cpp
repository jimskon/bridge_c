#include <iostream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "trans_bridge.h"

using namespace std;



  
trans_bridge::trans_bridge() {}
  
void trans_bridge::print() {
    map<MACADDR,Bridge_entry>::iterator it = bridge.begin();
    cout << "Bridge table (" << bridge.size() << ")\n";
    for(; it != bridge.end(); it++)
      {
	it->first.print();
	cout << " :: " << it->second.src_interface << endl;
      }
  }

  /* Transparent learning bridge
     pkt_src    - source interface 
     packet     - The packet to bridge
     returns:   0: broadcast
     1-n - dest interface
     -1 - not found, drop
  */
int trans_bridge::bridge_packet(int pkt_src, unsigned char *packet) {

    short dest_i = 0;
    MACADDR src_mac,dest_mac;

    // Get the addresses
    src_mac.get_src_mac(packet);
    dest_mac.get_dest_mac(packet);

    /*  Process the source address.  Add if not in bridge. 
	Update bridge if interface is different 
	drop if broadcast from this interface */
    if (bridge.find(src_mac) != bridge.end()) {
      // check if changed 
      Bridge_entry b = bridge.find(src_mac)->second;

      int src_i = b.src_interface;
      if (src_i != pkt_src) {
	// update source
	b.src_interface=pkt_src;
	bridge[src_mac]=b;
      }
    } else { 
      Bridge_entry new_entry(pkt_src);
      bridge.emplace(src_mac,new_entry);
    }
    
    if (dest_mac.is_multicast() || dest_mac.is_broadcast()) {
      dest_i = 0;
    } else {

      /* find where to send packet in the bridge */
      if (bridge.find(dest_mac) != bridge.end()) {
	dest_i=bridge[dest_mac].src_interface;
      } else {
	// If we don't find the destination, just drop
	dest_i=-1;
      }
    }
    return dest_i;
}
