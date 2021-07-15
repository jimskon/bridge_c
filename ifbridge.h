
#ifndef _UBNG_IFBRIDGE_H
#define _UBNG_IFBRIDGE_H

#include <map>

#include "iface.h"
#include "logger.h"
#include "macaddr.h"

#include <sys/epoll.h>

class ifbridge
{
	private:

		logger& _log;
		iface&  _trunk;

		int _epollfd;
		int _maxev;

		std::map<std::string, iface*> _name2if;
		std::map<macaddr,     iface*> _addr2if;

		iface* add_iface ( iface* ifp );

		int fwd_dnstream ( pdu& pkt             );
		int fwd_upstream ( pdu& pkt, iface *src );

	public:

		ifbridge( logger& log, iface& trunk, int maxev = 8 );
		virtual ~ifbridge();

		int    init ( void );
		void   run  ( void );
		void   dump ( void ) const;

		iface* add_iface ( const char *ifname );
		int    del_iface ( const char *ifname );
		iface* get_iface ( const char *ifname ) const;
};

#endif /*_UBNG_IFBRIDGE_H*/

/*EoF*/
