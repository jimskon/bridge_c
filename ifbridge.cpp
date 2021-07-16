
#include <cstdio>
#include <cstdlib>
#include <csignal>

#include <arpa/inet.h>

#include "brmap.h"
#include "iface.h"
#include "pdu/pdu.h"
#include "icmp4.h"
#include "logger.h"

#include "ifbridge.h"

#define BUFLEN 16384

#ifdef PROFILE
extern volatile bool ctrlc;
#endif

//  - - - - - - - - -  //
//  S T R U C T O R S  //
//  - - - - - - - - -  //

ifbridge::ifbridge( logger& log, iface& trunk, int maxev )
	: _log( log ), _trunk( trunk ), _epollfd( -1 ), _maxev( maxev )
{ ; }

ifbridge::~ifbridge()
{ ; }


//  - - - - - - -  //
//  M E T H O D S  //
//  - - - - - - -  //

int
ifbridge::init()
{
	if( _epollfd >= 0 ) {
		_log(1) << "ifbridge already initialized" << std::endl;
		return 0;
	}

	_epollfd = ::epoll_create1( 0 );
	if( _epollfd == -1 ) {
		_log(0) << "epoll_create() failed" << std::endl;
		return -1;
	}

	if( add_iface( &_trunk ) != &_trunk ) {
		_log(0) << "can't add trunk" << std::endl;
		return -1;
	}

	return 0;
}

iface*
ifbridge::add_iface( const char *ifname )
{
	auto kv = _name2if.find( ifname );

	if( kv != _name2if.end() ) {
		_log(1) << "interface already registered" << std::endl;
		return kv->second;
	}

	iface *ifp = new iface();

	/* bind the interface */

	if( ifp->bind( ifname ) != 0 ) {
		delete ifp;
		return (iface*)0;
	}

	if( add_iface( ifp ) != ifp ) {
		delete ifp;
		return (iface*)0;
	}

	_name2if.insert({ ifp->name(), ifp }); // XXX emplace ?
	_log(2) << "ADD: " << *ifp << std::endl;

	return ifp;
}

iface*
ifbridge::add_iface( iface *ifp )
{
	struct epoll_event ev;

	/* enable promiscuous mode */

	if( ifp->promisc( true ) != 0 ) {
		_log(1) << "can't enable promiscuous mode" << std::endl;
		// XXX do something about it
	}

	/* register the interface with epoll */

	ev.events   = EPOLLIN;
	ev.data.ptr = ifp;

	if( ::epoll_ctl( _epollfd, EPOLL_CTL_ADD, ifp->socket(), &ev ) == -1 ) {
		_log(1) << "epoll_ctl() failed" << std::endl;
		return (iface*)0;
	}

	return ifp;
}

int
ifbridge::del_iface( const char *ifname )
{
	auto kv = _name2if.find( ifname );

	if( kv == _name2if.end() )
		return -1;

	iface *ifp = kv->second;
	(void)ifp->promisc( false );
	ifp->close();

	(void)_name2if.erase( kv );
	delete ifp;

	return 0;
}

iface*
ifbridge::get_iface( const char *ifname ) const
{
	auto kv = _name2if.find( ifname );

	if( kv != _name2if.end() )
		return (iface*)0;

	return kv->second;
}

void
ifbridge::dump() const
{
	for( const auto& kv : _name2if ) {
		iface *ifp = kv.second;
		cout << '[' << ifp->name() << "] mtu: " << ifp->mtu() << std::endl;
	}
}

void
ifbridge::run()
{
	struct epoll_event *ev;
	struct epoll_event *ep;
	iface              *ifp;

	int nev;
	int i, n;

	ev = new struct epoll_event[ _maxev ];
	pdu pkt( BUFLEN );

	for( ;; ) {
		nev = epoll_wait( _epollfd, ev, _maxev, 1000 );
		if( nev < 0 ) {
#ifdef PROFILE
			if( errno == EINTR ) {
				if( ctrlc ) {
					_log(0) << "interrupted by user" << std::endl;
					break;
				}
				continue;
			}
#endif
			perror( "epoll_wait()" );
			break;
		}

		if( nev == 0 ) {
			/*
			 * timeout on epoll_wait() -- for now, just signal that we're still alive.
			 * this would be a nice place to delete expired BNG entries though
			 * (no need for timers/threads/locks since it'a all sequential).
			 */
			_log( 3 ) << "waiting..." << std::endl;
			continue;
		}

		for( i = 0; i < nev; ++i ) {
			ep  = &ev[ i ];
			ifp = (iface*)ep->data.ptr;

			n = ifp->recv( pkt );
			if( n < 0 ) {
				//perror( ifp->name() );
				break;
			}

			if( n == 0 )
				continue;

			(void)pkt.filter( _log( 1 ));

			if( ifp == &_trunk ) {
				(void)fwd_dnstream( pkt );
			} else {
				(void)fwd_upstream( pkt, ifp );
			}
		}
	}

	delete[] ev;
}

int
ifbridge::fwd_dnstream( pdu& pkt )
{
	iface *ifp;

	switch( pkt._sll.sll_pkttype ) {
		case PACKET_BROADCAST:
		case PACKET_MULTICAST:
			for( const auto& kv : _name2if ) {
				(void)kv.second->send( pkt );
			}
			break;

		case PACKET_OTHERHOST:
		{
			macaddr addr( pkt._x );
			auto kv = _addr2if.find( addr );
			if( kv == _addr2if.end() ) {
				_log(3) << "MAC " << addr << " unknown host" << std::endl;
				break;
			}
			ifp = kv->second;
			(void)ifp->send( pkt );
			break;
		}

		default:
			_log(3) << "unsupported packet type (dnstream)" << std::endl;
	}

	return 0;
}

int
ifbridge::fwd_upstream( pdu& pkt, iface *src )
{
	switch( pkt._sll.sll_pkttype ) {
		case PACKET_BROADCAST:
		case PACKET_MULTICAST:
		case PACKET_OTHERHOST:
		{
			macaddr addr( pkt._x + ETH_ALEN );
			auto kv = _addr2if.find( addr );
			if( kv == _addr2if.end() ) {
				_log(2) << '[' << src->name() << "] add host " << addr << std::endl;
				_addr2if.emplace( addr, src );
				break;
			}

			// XXX also remember to update existing entries
			// XXX (case of mobile devices roaming from one interface to another)

			_trunk.send( pkt );
			break;
		}

		default:
			_log(3) << "unsupported packet type (upstream)" << std::endl;
	}

	return 0;
}

/*EoF*/
