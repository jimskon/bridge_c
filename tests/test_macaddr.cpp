
#include <iostream>

#include "macaddr.h"

#include "tests/catch.hpp"


//  - - - - - - - - - -  //
//  T E S T S   S E T S  //
//  - - - - - - - - - -  //

static const uint8_t zero[] = { 0, 0, 0, 0, 0, 0 };
macaddr addr_zero( zero );

static const uint8_t one[] = { 0, 0, 0, 0, 0, 1 };
macaddr addr_one( one );


//  - - - - - - - - - - -  //
//  T E S T S   C A S E S  //
//  - - - - - - - - - - -  //

TEST_CASE( "compare macaddr instances", "macaddr::operator<()" )
{
	REQUIRE( addr_zero < addr_one );
	REQUIRE_FALSE( addr_one < addr_zero );
}

/*EoF*/
