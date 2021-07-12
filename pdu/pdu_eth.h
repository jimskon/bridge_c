
#ifndef _UBNG_PDU_ETH_H
#define _UBNG_PDU_ETH_H

#include "pdu.h"

struct pdu_eth final : public pdu
{
	pdu_eth( uint8_t *x, size_t len ) : pdu( x, len )
	{ ; }

	pdu_eth( const pdu& other, size_t off )
		: pdu( other._x + off, other._len - off )
	{ ; }

	pdu_eth( const pdu_eth& other )
		: pdu( other._x, other._len )
	{ ; }

	virtual ~pdu_eth()
	{ ; }

	int filter( std::ostream& log ) override;
};

std::ostream& operator<< ( std::ostream& os, const struct ethhdr* eth );

#endif /*_UBNG_PDU_ETH_H*/

/*EoF*/
