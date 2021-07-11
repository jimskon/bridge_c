
#include <cstring>

#include "packet.h"

//  - - - - - - - - -  //
//  S T R U C T O R S  //
//  - - - - - - - - -  //

packet::packet( size_t cap )
{
	_x   = new uint8_t[ cap ];
	_cap = cap;
	_len = 0;
}

packet::packet( uint8_t *x, size_t len )
	: _x( x ), _cap( 0 ), _len( len )
{ ; }

packet::~packet()
{
	if( _cap > 0 )
		delete[] _x;
}

//  - - - - - - -  //
//  M E T H O D S  //
//  - - - - - - -  //

std::ostream&
packet::dump( std::ostream& os ) const
{
	struct ethhdr *eth;

	eth = (struct ethhdr*)_x;

	os << "cap: " << _cap << std::endl;
	os << eth << std::endl;

	return os;
}

//  - - - - - - -  //
//  O S T R E A M  //
//  - - - - - - -  //

std::ostream& operator<< ( std::ostream& os, const packet& pkt )
{ return pkt.dump( os ); }

/*EoF*/
