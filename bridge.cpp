
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/select.h>

#include "brmap.h"
#include "iface.h"
#include "pdu/pdu.h"
#include "icmp4.h"
#include "logger.h"

#define BUFLEN 9000
/*  Port used to recieve messages from hostapd */
#define MESSPORT 7448
#define VLAN_SIZE 4  /* Number of bytes for vlan tagging */

void printpacket(const char* msg, const unsigned char* p, size_t len) {
    size_t i;
    cout << msg << " len=" << len << endl;
    for(i=0; i<len; ++i) {
      cout << std::hex << (int) p[i] << ":";
    }
    cout << std::dec << endl;
}


int
main( int argc, char *argv[] )
{
	logger  log;
	iface  if1, if2;
	struct timeval tv;
	fd_set rfds;
	int    fd1, fd2;
	int    maxfd;
	int    n;
	int    len;
	int    debug;
	brmap  a_brmap( log );

	uint8_t *x;

	if( argc != 3 ) {
		std::cerr << "usage: " << argv[0] << " if1 if2" << std::endl
		          << "FIrst interface is the tagged interface" << std::endl
		          << "DEBUG=1 or 2 for debug output"    << std::endl
		;
		exit(1);
	}

	debug = 0;
	if(getenv("DEBUG")) {
		debug = atoi(getenv("DEBUG"));
	}
	log.level( debug );

	assert( if1.bind( argv[1] ) == 0 );
	assert( if2.bind( argv[2] ) == 0 );

	assert( if1.promisc( true ) == 0 );
	assert( if2.promisc( true ) == 0 );

	log(1) << if1 << std::endl;
	log(1) << if2 << std::endl;

	/*  Start bridge listener (for vlan assignments) */
	int port = MESSPORT;
        a_brmap.start_listener(port);
	
	x = (uint8_t *) malloc( BUFLEN );
	assert( x != NULL );

	fd1 = if1.socket();
	fd2 = if2.socket();
	maxfd = ( fd1 > fd2 ) ? fd1 : fd2;

	for( ;; ) {
	        pdu pkt( BUFLEN ); 
	
		FD_ZERO( &rfds );
		FD_SET( fd1, &rfds );
		FD_SET( fd2, &rfds );

		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		n = select( maxfd+1, &rfds, NULL, NULL, &tv );
		if( n < 0 ) {
			perror( "select()" );
			break;
		}

		if( n == 0 ) {
			/*
			 * timeout on select() -- for now, just signal that we're still alive.
			 * this would be a nice place to delete expired BNG entries though
			 * (no need for timers/threads/locks since it'a all sequential).
			 */
			log( 3 ) << "waiting..." << std::endl;
			continue;
		}

		/* This interface will add vlan tags on egress, and remove them on ingress.*/
		if( FD_ISSET( fd1, &rfds )) {
			n = if1.recv( pkt );

	                if( n < 0 ) {
				perror( if1.name() );
				break;
			}

			if( n == 0 )
				continue;

			/* Handle vlan tags on ingress */
			int s_vid = pkt.vlan_untag();
			if (s_vid >= 0) {			 
			  cout << "In VID: " << s_vid << endl;
			}
			(void)pkt.filter( log( 1 ));
			

/*
			log(1) << pkt << std::endl;

			off = pkt.payload_off();
			pdu_eth pkt_eth( pkt, off );
			log(1) << " > " << pkt_eth << std::endl;

			off = pkt_eth.payload_off();
			pdu_ipv4 pkt_ipv4( pkt_eth, off );
			log(1) << "  > " << pkt_ipv4 << std::endl;
*/

			int vid;
			short d = a_brmap.map_pkt(1,pkt._x,&vid);
			//short d = 0;
			if ( n > if2.mtu() ) {
				log(1) << "Packet too long: " << n << std::endl;
				//log(1) << pkt;
				log(1) << "Sending ICMP fragmentation needed" << std::endl;
				n = icmp4_gen_needfrag( x+128, BUFLEN-128, x, if2.mtu() );
				//(void)if1.send( x+128, n, sll );
				continue;
			}

			// Send to interface 2 if dest is 2 or broadcast
			if( d == 2 || d == 0) {

			  if( (len = if2.send( pkt )) < 0 ) {
			               perror( if2.name() );
			               break;
			  }
			    log << "1>2 " << len << "  " << std::endl;
			}
		}
		/* This interface will add vlan tags on ingress, and remove on egress. */
		if( FD_ISSET( fd2, &rfds )) {
			n = if2.recv( pkt );

			if( n < 0 ) {
				perror( if2.name() );
				break;
			}

			if( n == 0 )
				continue;

			(void)pkt.filter( log( 1 ));

			int vid;
			short d = a_brmap.map_pkt(2,pkt._x,&vid);
			//short d = 0;

			if ( n > if1.mtu() ) {
				log(1) << "Packet too long: " << n << std::endl;
				//log(1) << pkt;
				log(1) << "Sending ICMP fragmentation needed" << std::endl;
				n = icmp4_gen_needfrag( x+128, BUFLEN-128, x, if1.mtu() );
				//(void)if1.send( x+128, n, sll );
				continue;
			}

			//log(1) << pkt << std::endl;
			if (vid>=0) {
			  pkt.vlan_tag((uint32_t) vid);
			}
			// Send to interface 1 if dest is 1 or broadcast
			if( d == 1 || d == 0) {
			        if( (len = if1.send( pkt )) < 0 ) {
			               perror( if1.name() );
			               break;
				}
				log << "2>1 " << len << "  " << std::endl;
			}
		        if ( debug > 1 )
			     a_brmap.print();
		}
	}

	free( x );

	return 0;
}

/*EoF*/
