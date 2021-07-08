
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<assert.h>

#include<sys/select.h>
#include<sys/socket.h>
#include<sys/ioctl.h>

#include<arpa/inet.h>

#include<net/if.h>
#include<net/ethernet.h>

#include<linux/if_packet.h>

#include "brmap.h"

#define BUFLEN 4096

int debug=0;
/* iface */

struct iface {
	const char *if_name;
	uint8_t     if_hwaddr[ETH_ALEN];
	int         if_index;
	int         if_sock;
};

static int
iface_init( struct iface *ifp, const char *if_name )
{
	struct sockaddr_ll sll;
	struct packet_mreq mr;
	struct ifreq req;
	int    if_index, if_sock;

	if_sock = socket( AF_PACKET, SOCK_RAW, htons( ETH_P_ALL ));
	if( if_sock < 0 ) {
		perror( if_name );
		return -1;
	}

	/* store the interface L2 address in if_hwaddr (only used for debugging) */

	(void)strncpy( req.ifr_name, if_name, IFNAMSIZ );
	if( ioctl( if_sock, SIOCGIFHWADDR, &req ) < 0 ) {
		perror( if_name );
		(void)close( if_sock );
		return -1;
	}

	(void)memcpy( ifp->if_hwaddr, req.ifr_hwaddr.sa_data, ETH_ALEN );

	/* store the interface index in if_index */

	if( ioctl( if_sock, SIOCGIFINDEX, &req ) < 0 ) {
		perror( if_name );
		(void)close( if_sock );
		return -1;
	}

	if_index = req.ifr_ifindex;

	/* enable promiscous mode */

	(void)memset( &mr, 0, sizeof( mr ));
	mr.mr_ifindex = if_index;
	mr.mr_type    = PACKET_MR_PROMISC;

	if( setsockopt( if_sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof( mr )) < 0 ) {
		perror( if_name );
		(void)close( if_sock );
		return -1;
	}

	/*
	 * populate a sockaddr_ll instance and bind the interface to it
	 * (might set sll_protocol here as well -- see note in iface_recv())
	 */

	(void)memset( &sll, 0, sizeof( sll ));
	sll.sll_family  = AF_PACKET; /* only interested in packets from the AF_PACKET family */
	sll.sll_ifindex = if_index;  /* only interested in packets captured from this interface */

	if( bind( if_sock, (const struct sockaddr*)&sll, sizeof( sll )) < 0 ) {
		perror( if_name );
		(void)close( if_sock );
		return -1;
	}

	ifp->if_name  = if_name;
	ifp->if_index = if_index;
	ifp->if_sock  = if_sock;

	return 0;
}

static void
iface_cleanup( struct iface *ifp )
{
	struct packet_mreq mr;

	(void)memset( &mr, 0, sizeof( mr ));
	mr.mr_ifindex = ifp->if_index;
	mr.mr_type    = PACKET_MR_PROMISC;

	(void)setsockopt( ifp->if_sock, SOL_PACKET, PACKET_DROP_MEMBERSHIP, &mr, sizeof( mr ));
	(void)close( ifp->if_sock );
}

static void
iface_dump( struct iface *ifp )
{
	uint8_t *x = ifp->if_hwaddr;
	(void)printf( "%s: ",      ifp->if_name );
	(void)printf( "index: %d", ifp->if_index );
	(void)printf( ", hwaddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
		x[0], x[1], x[2], x[3], x[4], x[5] );
}

static int
iface_recv( struct iface *ifp, void *x, size_t len )
{
	struct sockaddr_ll sll;
	socklen_t fromlen;
	int n;

	fromlen = sizeof( struct sockaddr_ll );
	n = recvfrom( ifp->if_sock, x, len, 0, (struct sockaddr*)&sll, &fromlen );

	if( n < 0 )
		return n;

	/* ignore packets involving our host -- see PACKET(7) */

	switch( sll.sll_pkttype ) {
		case PACKET_HOST:      /* packet is addressed to us */
		case PACKET_OUTGOING:  /* packet originated from us */
			return 0;

		default:
			;
	}

	/* something's wrong with the interface setup if any of these fail */

	assert( sll.sll_family  == AF_PACKET );
	assert( sll.sll_ifindex == ifp->if_index );

	if (debug > 0) {
	        (void)printf( "sll_pkttype:  %d\n", sll.sll_pkttype  );
		(void)printf( "sll_hatype:   %d\n", sll.sll_hatype );
	}
	/*
	 * sll_protocol is in network byte order
	 * (also available in the packet payload)
	 *
	 * we'd expect to see:
	 *  . 0x8100 (ETH_P_8021Q) / 0x88a8 (ETH_P_8021AD) on the trunk (rXg side)
	 *  . 0x0800 (ETH_P_IP) / 0x0806 (ETH_P_ARP) / 0x86dd (ETH_P_IPV6)
	 *    on the WLAN (device side)
	 *
	 * note: can probably be used as a filter while bind()ing the trunk interface.
	 *       quoting PACKET(7): "Fields used for binding are sll_family (should be
	 *       AF_PACKET), sll_protocol, and sll_ifindex."
	 *                   ^^^^^^^^^^^^
	 */

	if (debug > 0) {
	        (void)printf( "sll_protocol: 0x%04x\n", ntohs( sll.sll_protocol ));
	}
	return n;
}

static int
iface_send( struct iface *ifp, void *x, size_t len )
{
	struct sockaddr_ll sll;
	struct ethhdr *eh;

	/*
	 * Quoting PACKET(7):
	 * "When you send packets, it is enough to specify sll_family, sll_addr,
	 *  sll_halen, sll_ifindex, and sll_protocol.  The other fields should be 0."
	 */

	eh = (struct ethhdr*)x;

	(void)memset( &sll, 0, sizeof( sll ));
	(void)memcpy( sll.sll_addr, eh->h_source, ETH_ALEN );
	sll.sll_halen    = ETH_ALEN;
	sll.sll_family   = AF_PACKET;
	sll.sll_ifindex  = ifp->if_index;
	sll.sll_protocol = eh->h_proto;

	return
		sendto( ifp->if_sock, x, len, 0, (struct sockaddr*)&sll, sizeof( sll ));
}

/* debug */

static void
pkt_dump( struct iface *ifp, const uint8_t *x, int n )
{
	int i;

	(void)printf( "[%s] ", ifp->if_name );
	(void)printf( "%02x:%02x:%02x:%02x:%02x:%02x -> ", x[6], x[7], x[8], x[9], x[10], x[11] );
	(void)printf( "%02x:%02x:%02x:%02x:%02x:%02x",     x[0], x[1], x[2], x[3],  x[4],  x[5] );
	(void)printf( ", len: %d\n", n );

	for( i = 0; i < n; i++ )
		(void)printf( "%02x%c", x[i], ((i+1) & 0xf) ? ' ' : '\n' );

	if( i & 0xf )
		(void)printf( "\n" );
}

/* main */

int
main( int argc, char *argv[] )
{
	struct iface if1, if2;
	struct timeval tv;
	fd_set rfds;
	int    maxfd;
	int    n;
	brmap a_brmap;
	
	uint8_t *x;
	
	if( argc != 3 ) {
		(void)fprintf( stderr, "usage: %s if1 if2\n", argv[0] );
		(void)fprintf( stderr, "DEBUG=1 or 2 for debug output" );
		exit(1);
	}

	if(getenv("DEBUG")) { debug = atoi(getenv("DEBUG")); }

	assert( iface_init( &if1, argv[1] ) == 0 );
	assert( iface_init( &if2, argv[2] ) == 0 );

	if ( debug > 0 ) {
	     iface_dump( &if1 );
	     iface_dump( &if2 );
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
		        if ( debug > 0 )
			      (void)fprintf( stderr, "waiting...\n" );
			continue;
		}

		if( FD_ISSET( if1.if_sock, &rfds )) {
			n = iface_recv( &if1, x, BUFLEN );

	                if( n < 0 ) {
				perror( if1.if_name );
				break;
			}

			if( n == 0 )
				continue;

			short d = a_brmap.map_pkt(1,x);
			
			if ( n > 1518 ) {
			        fprintf( stderr, "Packet too long: %d\n",n );
				continue;
			}

			if ( debug>1 )
			  pkt_dump( &if1, x, n );

			// Send to interface 2 if dest is 2 or broadcast
			if( d == 2 || d == 0) {
			        if( iface_send( &if2, x, n ) < 0 ) {
			               perror( if2.if_name );
			               break;
			        }
			}
		}

		if( FD_ISSET( if2.if_sock, &rfds )) {
			n = iface_recv( &if2, x, BUFLEN );
			if( n < 0 ) {
				perror( if2.if_name );
				break;
			}

			if( n == 0 )
				continue;

			short d = a_brmap.map_pkt(2,x);

			if ( n > 1518 ) {
			        fprintf( stderr, "Packet too long: %d\n",n );
				continue;
			}

			if ( debug>1 )
			      pkt_dump( &if2, x, n );

			// Send to interface 1 if dest is 1 or broadcast
			if( d == 1 || d == 0) {
			        if( iface_send( &if1, x, n ) < 0 ) {
			               perror( if1.if_name );
			               break;
			        }
			}
		        if ( debug > 1 )
			     a_brmap.print();
		}
	}

	free( x );

	iface_cleanup( &if2 );
	iface_cleanup( &if1 );

	return 0;
}

/*EoF*/
