
#ifndef _UBNG_MACADDR_H
#define _UBNG_MACADDR_H

#include <cstdint>
#include <cstring>
#include <iostream>

#include <net/ethernet.h>

class macaddr
{
	private:

		constexpr static uint8_t _zero[ETH_ALEN] = { 0, 0, 0, 0, 0, 0 };

		uint8_t _x[ ETH_ALEN ];

	public:

		macaddr( const uint8_t *x = macaddr::_zero );

		bool operator< ( const macaddr& other ) const
		{
			return
				( ::memcmp( _x, other._x, ETH_ALEN ) < 0 );
		}

		friend std::ostream& operator<< ( std::ostream& os, const macaddr& mac );
};

#endif /*_UBNG_MACADDR_H*/

/*EoF*/
