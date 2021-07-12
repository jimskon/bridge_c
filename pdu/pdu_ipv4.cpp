
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pdu_ipv4.h"
#include "pdu_udp.h"
#include "pdu_tcp.h"

static const char*
ipv4_proto_str( unsigned proto )
{
	switch( proto ) {
		case IPPROTO_ICMP: return " (ICMP)";
		case IPPROTO_IGMP: return " (IGMP)";
		case IPPROTO_UDP:  return " (UDP)";
		case IPPROTO_TCP:  return " (TCP)";
		default:
			;
	}

	return "";
}

//  - - - - - -  //
//  P U B L I C  //
//  - - - - - -  //

int
pdu_ipv4::filter( std::ostream& log )
{
	struct iphdr *ih = (struct iphdr*)_x;
	size_t off = ( ih->ihl << 2 );

	struct in_addr saddr = { ih->saddr };
	struct in_addr daddr = { ih->daddr };

	log << " > IPv4: "
	    << inet_ntoa( saddr )
	    << " -> "
	    << inet_ntoa( daddr )
	    << ", id: "     << ntohs( ih->id )
	    << ", ttl: "    << (unsigned)ih->ttl
	    << ", proto: "  << (unsigned)ih->protocol
	                    << ipv4_proto_str( ih->protocol )
	    << ", length: " << _len
	    << std::endl;

	switch( ih->protocol ) {
		case IPPROTO_TCP:
		{
			pdu_tcp tcp( *this, off );
			return tcp.filter( log );
		}

		case IPPROTO_UDP:
		{
			pdu_udp udp( *this, off );
			return udp.filter( log );
		}

		default:
			;
	}

	return 1;
}

/*EoF*/
