
#ifndef _UBNG_PDU_H
#define _UBNG_PDU_H

#include <cstdlib>
#include <cstdint>
#include <ostream>

#include <linux/if_packet.h>

struct pdu
{
	uint8_t *_x;   // buffer
	size_t   _cap; // capacity
	size_t   _len; // length

	struct sockaddr_ll _sll; // link-layer info

	pdu( size_t cap );
	pdu( uint8_t *x, size_t cap );
	virtual ~pdu();

	virtual int filter( std::ostream& log );

	std::ostream& hexdump( std::ostream& log );
};

#endif /*_UBNG_PDU_H*/

/*EoF*/
