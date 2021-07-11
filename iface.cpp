
#include <cstdio>
#include <cstring>
#include <cassert>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include "brmap.h"
#include "iface.h"

//  - - - - - - - - -  //
//  S T R U C T O R S  //
//  - - - - - - - - -  //

iface::iface( logger& log ) : if_log( log )
{
	(void)::memset( if_hwaddr, 0, sizeof( if_hwaddr ));
	if_name  = "(null)";
	if_index =  0;
	if_sock  = -1;
	if_mtu   =  0;
}

iface::~iface()
{
	(void)promisc( false );
	(void)::close( if_sock );
}

//  - - - - - - -  //
//  M E T H O D S  //
//  - - - - - - -  //

int
iface::bind( const char *name )
{
	struct sockaddr_ll sll;
	struct ifreq req;

	if_name = name;

	if_sock = ::socket( AF_PACKET, SOCK_RAW, ::htons( ETH_P_ALL ));
	if( if_sock < 0 ) {
		::perror( if_name );
		return -1;
	}

	/* store the interface L2 address in if_hwaddr (only used for debugging) */

	(void)::strncpy( req.ifr_name, if_name, IFNAMSIZ-1 );
	if( ::ioctl( if_sock, SIOCGIFHWADDR, &req ) < 0 ) {
		::perror( if_name );
		(void)::close( if_sock );
		return -1;
	}

	(void)::memcpy( if_hwaddr, req.ifr_hwaddr.sa_data, ETH_ALEN );

	/* store the interface index in if_index */

	if( ::ioctl( if_sock, SIOCGIFINDEX, &req ) < 0 ) {
		::perror( if_name );
		(void)::close( if_sock );
		return -1;
	}

	if_index = req.ifr_ifindex;

	/* store the interface MTU in if_mtu */

	if( ::ioctl( if_sock, SIOCGIFMTU, &req ) < 0 ) {
		::perror( if_name );
		(void)::close( if_sock );
		return -1;
	}

	if_mtu = req.ifr_mtu;

	/*
	 * populate a sockaddr_ll instance and bind the interface to it
	 * (might set sll_protocol here as well -- see note in iface_recv())
	 */

	(void)::memset( &sll, 0, sizeof( sll ));
	sll.sll_family  = AF_PACKET; /* only interested in packets from the AF_PACKET family */
	sll.sll_ifindex = if_index;  /* only interested in packets captured from this interface */

	if( ::bind( if_sock, (const struct sockaddr*)&sll, sizeof( sll )) < 0 ) {
		::perror( if_name );
		(void)::close( if_sock );
		return -1;
	}

	return 0;
}

int
iface::promisc( bool enable )
{
	struct packet_mreq mr;
	int    op;

	(void)::memset( &mr, 0, sizeof( mr ));
	mr.mr_ifindex = if_index;
	mr.mr_type    = PACKET_MR_PROMISC;

	op = enable ? PACKET_ADD_MEMBERSHIP : PACKET_DROP_MEMBERSHIP;

	return
		::setsockopt( if_sock, SOL_PACKET, op, &mr, sizeof( mr ));
}

/*
void
iface::cleanup()
{
	(void)promisc( false );
	(void)::close( ifp->if_sock );
}
*/

void
iface::dump()
{
	uint8_t *x = if_hwaddr;
	(void)::printf( "%s: ",      if_name );
	(void)::printf( "index: %d", if_index );
	(void)::printf( ", hwaddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
		x[0], x[1], x[2], x[3], x[4], x[5] );
}

int
iface::recv( void *x, size_t len )
{
	struct sockaddr_ll sll;
	socklen_t fromlen;
	int n;

	fromlen = sizeof( struct sockaddr_ll );
	n = ::recvfrom( if_sock, x, len, 0, (struct sockaddr*)&sll, &fromlen );

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
	assert( sll.sll_ifindex == if_index );

	if_log << "sll_pkttype:  " << sll.sll_pkttype;
	if_log << "sll_hatype:   " << sll.sll_hatype;

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

	if_log << "sll_protocol: " << ::ntohs( sll.sll_protocol );

	return n;
}

int
iface::send( const void *x, size_t len )
{
	struct sockaddr_ll sll;
	struct ethhdr *eh;

	/*
	 * Quoting PACKET(7):
	 * "When you send packets, it is enough to specify sll_family, sll_addr,
	 *  sll_halen, sll_ifindex, and sll_protocol. The other fields should be 0."
	 */

	eh = (struct ethhdr*)x;

	(void)::memset( &sll, 0, sizeof( sll ));
	(void)::memcpy( sll.sll_addr, eh->h_source, ETH_ALEN );
	sll.sll_halen    = ETH_ALEN;
	sll.sll_family   = AF_PACKET;
	sll.sll_ifindex  = if_index;
	sll.sll_protocol = eh->h_proto;

	return
		::sendto( if_sock, x, len, 0, (struct sockaddr*)&sll, sizeof( sll ));
}

/*EoF*/
