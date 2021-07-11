
#include <cstdio>
#include <arpa/inet.h>

#include "packet.h"
#include "macaddr.h"

//  - - - - - - -  //
//  M E T H O D S  //
//  - - - - - - -  //

std::ostream&
packet_eth::dump( std::ostream& os ) const
{
	struct ethhdr *eth;

	eth = (struct ethhdr*)_x;

	os << eth << ", cap: " << _cap << std::endl;
	return os;
}

//  - - - - - - -  //
//  O S T R E A M  //
//  - - - - - - -  //

std::ostream&
operator<< ( std::ostream& os, const struct ethhdr* eth )
{
	char strproto[8];

	uint16_t proto = ntohs( eth->h_proto );
	(void)snprintf( strproto, sizeof( strproto ), "0x%04x", proto );

	os << macaddr( eth->h_source )
	   << " -> "
	   << macaddr( eth->h_dest )
	   << ", proto: " << strproto;

	switch( proto ) {
		case ETHERTYPE_ARP:    os << " (ARP)";    break;
		case ETHERTYPE_REVARP: os << " (REVARP)"; break;
		case ETHERTYPE_IP:     os << " (IPv4)";   break;
		case ETHERTYPE_IPV6:   os << " (IPv6)";   break;
		case ETHERTYPE_VLAN:   os << " (VLAN)";   break;
		case 0x88a8:           os << " (QinQ)";   break;
		case 0x88cc:           os << " (LLDP)";   break;

		default:
			if( proto <= 0x5dc ) {
				os << " (802.3)";
			} else {
				os << " (unknown)";
			}
	}

	return os;
}

/*EoF*/
