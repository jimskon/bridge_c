
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/select.h>

#include "brmap.h"
#include "iface.h"
#include "logger.h"

#define BUFLEN 9000

int
main( int argc, char *argv[] )
{
	logger  log;
	iface if1( log ), if2( log );
	struct timeval tv;
	fd_set rfds;
	int    maxfd;
	int    n;
	int    len;
	int    debug;
	brmap  a_brmap( log );

	uint8_t *x;

	if( argc != 3 ) {
		(void)fprintf( stderr, "usage: %s if1 if2\n", argv[0] );
		(void)fprintf( stderr, "DEBUG=1 or 2 for debug output\n" );
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

	if ( debug > 0 ) {
		if1.dump();
		if2.dump();
	}

	x = (uint8_t *) malloc( BUFLEN );
	assert( x != NULL );

	maxfd = ( if1.if_sock > if2.if_sock ) ? if1.if_sock : if2.if_sock;

	for( ;; ) {
		FD_ZERO( &rfds );
		FD_SET( if1.if_sock, &rfds );
		FD_SET( if2.if_sock, &rfds );

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
			log(0) << "waiting...";
			continue;
		}

		if( FD_ISSET( if1.if_sock, &rfds )) {
			n = if1.recv( x, BUFLEN );

	                if( n < 0 ) {
				perror( if1.if_name );
				break;
			}

			if( n == 0 )
				continue;

			short d = a_brmap.map_pkt(1,x);

			if ( n > 1518 ) {
				log(1) << "Packet too long: " << n;
				log(1).hexdump( x, n );
				continue;
			}

			// XXX improve packet logging
			log(1).hexdump( x, n );

			// Send to interface 2 if dest is 2 or broadcast
			if( d == 2 || d == 0) {

			  if( (len = if2.send( x, n )) < 0 ) {
			               perror( if2.if_name );
			               break;
			  }
			    log << "1>2 " << len << "  ";
			}
		}

		if( FD_ISSET( if2.if_sock, &rfds )) {
			n = if2.recv( x, BUFLEN );
			if( n < 0 ) {
				perror( if2.if_name );
				break;
			}

			if( n == 0 )
				continue;

			short d = a_brmap.map_pkt(2,x);

			if ( n > 1518 ) {
				log(1) << "Packet too long: " << n;
				log(1).hexdump( x, n );
				continue;
			}

			// XXX improve packet logging
			log(1).hexdump( x, n );

			// Send to interface 1 if dest is 1 or broadcast
			if( d == 1 || d == 0) {
			        if( (len = if1.send( x, n )) < 0 ) {
			               perror( if1.if_name );
			               break;
				}
				log << "2>1 " << len << "  ";
			}
		        if ( debug > 1 )
			     a_brmap.print();
		}
	}

	free( x );

	//iface_cleanup( &if2 );
	//iface_cleanup( &if1 );

	return 0;
}

/*EoF*/
