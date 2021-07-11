
#ifndef _UBNG_IFACE_H
#define _UBNG_IFACE_H

#include <net/ethernet.h>

#include "logger.h"

struct iface
{
	logger&     if_log;
	const char *if_name;
	uint8_t     if_hwaddr[ETH_ALEN];
	int         if_index;
	int         if_sock;
	int         if_mtu;

	iface( logger& log );
	virtual ~iface();

	int  bind    ( const char *name );
	int  promisc ( bool enable );
	void dump    ( void );
	int  recv    ( void *x, size_t len );
	int  send    ( const void *x, size_t len );
};

#endif /*_UBNG_IFACE_H*/

/*EoF*/
