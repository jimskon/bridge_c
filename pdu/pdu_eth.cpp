
#include <cstdio>
#include <arpa/inet.h>

#include "pdu_eth.h"
#include "pdu_ipv4.h"
#include "macaddr.h"

//  - - - - - -  //
//  P U B L I C  //
//  - - - - - -  //

int
pdu_eth::filter( std::ostream& log )
{
	char strproto[8];

	struct ethhdr *eth = (struct ethhdr*)_x;
	uint16_t proto = ntohs( eth->h_proto );
	(void)snprintf( strproto, sizeof( strproto ), "0x%04x", proto );

#ifdef PDU_DEBUG
	log << " > ETH: "
	    << macaddr( eth->h_source )
	    << " -> "
	    << macaddr( eth->h_dest )
	    << ", proto: "  << strproto
	    << ", length: " << _len
	    << std::endl;
	;
#endif /*PDU_DEBUG*/

	switch( proto ) {
		case ETHERTYPE_IP:
		{
			pdu_ipv4 ipv4( *this, ETH_HLEN );
			return ipv4.filter( log );
		}

		case ETHERTYPE_IPV6:
		{
			return 0;
		}

		default:
			;
	}

	return 1;
}

/*EoF*/
