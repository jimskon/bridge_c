
#ifndef _UBNG_IFACE_H
#define _UBNG_IFACE_H

#include <string>
#include <ostream>

#include "logger.h"
#include "pdu/pdu.h"

#include <net/ethernet.h>

class iface
{
	private:

		std::string _name;
		uint8_t     _hwaddr[ETH_ALEN];
		int         _index;
		int         _sock;
		int         _mtu;

	public:

		iface();
		virtual ~iface();

		int  bind    ( const char *ifname );
		int  promisc ( bool enable );
		void close   ( void );

		int  recv    ( struct pdu& pkt );
		int  send    ( struct pdu& pkt );

		int socket( void ) const
		{ return _sock; }

		int mtu( void ) const
		{ return _mtu; }

		const std::string name() const
		{ return _name; }

		friend std::ostream& operator<< ( std::ostream& os, const iface& obj );
};

#endif /*_UBNG_IFACE_H*/

/*EoF*/
