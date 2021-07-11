
#ifndef _UBNG_IFACE_H
#define _UBNG_IFACE_H

#include <ostream>

#include "logger.h"
#include "packet.h"

class iface
{
	private:

		logger&     _log;
		const char *_name;
		uint8_t     _hwaddr[ETH_ALEN];
		int         _index;
		int         _sock;
		int         _mtu;

	public:

		iface( logger& log );
		virtual ~iface();

		int bind    ( const char *name );
		int promisc ( bool enable );

/*
		int recv    (       void *x, size_t len, struct sockaddr_ll& sll );
		int send    ( const void *x, size_t len, struct sockaddr_ll& sll );
*/

		int recv    ( struct packet& pkt );
		int send    ( struct packet& pkt );

		int socket( void ) const
		{ return _sock; }

		int mtu( void ) const
		{ return _mtu; }

		const char* name() const
		{ return _name; }

		friend std::ostream& operator<< ( std::ostream& os, const iface& obj );
};

#endif /*_UBNG_IFACE_H*/

/*EoF*/
