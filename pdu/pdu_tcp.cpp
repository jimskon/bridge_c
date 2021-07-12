
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pdu_tcp.h"

static const char *tcp_flags[6] = { "URG", "ACK", "PSH", "RST", "SYN", "FIN" };

static void
tcp_flags_dump( std::ostream& log, unsigned flags )
{
	int i, n;
	unsigned mask = 0x20;

	for( i = 0, n = 0; i < 6; i++ ) {
		if( flags & mask ) {
			if( n++ > 0 ) log << ',';
			log << tcp_flags[i];
		}
		mask >>= 1;
	}
}

//  - - - - - -  //
//  P U B L I C  //
//  - - - - - -  //

int
pdu_tcp::filter( std::ostream& log )
{
	struct tcphdr *th = (struct tcphdr*)_x;
	size_t off = ( th->th_off << 2);

	log << " > TCP: "
	    << ntohs( th->th_sport )
	    << " -> "
	    << ntohs( th->th_dport )
	    << ", off: " << off
	    << ", flags: <"
	;

	tcp_flags_dump( log, th->th_flags );

	log << '>' << std::endl;

	return 1;
}

/*EoF*/
