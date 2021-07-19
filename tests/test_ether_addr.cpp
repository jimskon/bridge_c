
#include "iface.h"

#include "tests/catch.hpp"

//  - - - - - - - - - -  //
//  T E S T   C A S E S  //
//  - - - - - - - - - -  //

TEST_CASE( "comparing ether_addr instances", "operator ==" )
{
	::ether_addr a = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	::ether_addr b = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	std::cout << "comparing ether_addr: " << a << " vs " << b << std::endl;

	REQUIRE( a == b );
}

/*EoF*/
