
#include <cstdio>

#include "logger.h"

//  - - - - - - - - -  //
//  S T R U C T O R S  //
//  - - - - - - - - -  //

logger::logger() : _level(0), _user(0)
{ ; }

logger::~logger()
{ ; }

//  - - - - - -  //
//  P U B L I C  //
//  - - - - - -  //

void
logger::hexdump( const uint8_t *x, size_t len )
{
	size_t i;

	for( i = 0; i < len; i++ )
		(void)::fprintf( ::stderr, "%02x%c", x[i], ((i+1) & 0xf) ? ' ' : '\n' );

	if( i & 0xf )
		(void)::fprintf( ::stderr, "\n" );
}

//  - - - - - - -  //
//  P R I V A T E  //
//  - - - - - - -  //

/*EoF*/
