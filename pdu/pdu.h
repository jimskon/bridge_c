
#ifndef _UBNG_PDU_H
#define _UBNG_PDU_H

#include <cstdlib>
#include <cstdint>
#include <ostream>

#include <linux/if_packet.h>

struct pdu
{
	enum direction_t {
		DIR_UNKNOWN = 0, // unknwon
		DIR_UPSTREAM,    // upstream
		DIR_DNSTREAM     // downstream
	};

	uint8_t    *_x;     // buffer
	size_t      _cap;   // capacity
	size_t      _len;   // length
	direction_t _dir;   // direction

	struct sockaddr_ll _sll; // link-layer info

	pdu( size_t cap, direction_t = DIR_UNKNOWN );
	pdu( uint8_t *x, size_t cap, direction_t dir = DIR_UNKNOWN );
	virtual ~pdu();

	virtual int filter( std::ostream& log );

	std::ostream& hexdump( std::ostream& log );
};

#endif /*_UBNG_PDU_H*/

/*EoF*/
