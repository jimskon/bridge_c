
#ifndef _UBNG_PDU_IPv6_H
#define _UBNG_PDU_IPv6_H

#include "pdu.h"

struct pdu_ipv6 final : public pdu
{
	pdu_ipv6( uint8_t *x, size_t cap ) : pdu( x, cap )
	{ ; }

	pdu_ipv6( const pdu& other, size_t off )
		: pdu( other._x + off, other._len - off, other._dir )
	{ ; }

	virtual ~pdu_ipv6()
	{ ; }

	int filter( std::ostream& log ) override;
};

#endif /*_UBNG_PDU_IPv6_H*/

/*EoF*/
