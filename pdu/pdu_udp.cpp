
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pdu_udp.h"

//  - - - - - -  //
//  P U B L I C  //
//  - - - - - -  //

int
pdu_udp::filter( std::ostream& log )
{
#ifdef PDU_DEBUG
	struct udphdr *uh = (struct udphdr*)_x;

	log << " > UDP: "
	    << ntohs( uh->uh_sport )
	    << " -> "
	    << ntohs( uh->uh_dport )
	    << ", length: " << _len
	    << std::endl;
#else
	(void)log;
#endif /*PDU_DEBUG*/

	return 0;
}

/*EoF*/
