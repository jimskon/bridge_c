
#include <iomanip>

#include "pdu.h"
#include "pdu_eth.h"

#ifdef PDU_DEUBG
static const char* pkttypes[] = {
	"HOST",      // PACKET_HOST
	"BROADCAST", // PACKET_BROADCAST
	"MULTICAST", // PACKET_MULTICAST
	"OTHERHOST", // PACKET_OTHERHOST
	"OUTGOING",  // PACKET_OUTGOING
	"LOOPBACK",  // PACKET_LOOPBACK
	"USER",      // PACKET_USER
	"KERNEL"     // PACKET_KERNEL
};
#endif /*PDU_DEBUG*/

//  - - - - - - - - -  //
//  S T R U C T O R S  //
//  - - - - - - - - -  //

pdu::pdu( size_t cap )
{
	_x   = new uint8_t[ cap ];
	_cap = cap;
	_len = 0;
}

pdu::pdu( uint8_t *x, size_t len )
	: _x( x ), _cap( 0 ), _len( len )
{ ; }

pdu::~pdu()
{
	if( _cap > 0 )
		delete[] _x;
}

//  - - - - - - -  //
//  M E T H O D S  //
//  - - - - - - -  //

int
pdu::filter( std::ostream& os )
{
	pdu_eth eth( *this, 0 );

#ifdef PDU_DEUBG
	os << "pkttype: "  << (int)_sll.sll_pkttype
	                   << " (" << pkttypes[_sll.sll_pkttype & 0x07] << ')'
	   << ", length: " << _len
	   << std::endl;
#endif /*PDU_DEBUG*/

	return
		eth.filter( os );
}

std::ostream&
pdu::hexdump( std::ostream& os )
{
	size_t i;

	std::ios_base::fmtflags flags( os.flags() );
	os << std::hex << std::setfill('0') << std::setw(2);

	for( i = 0; i < _len; i++ ) {
		os << (unsigned)_x[i];
		if( (i+1) & 0xf ) {
			os << ' ';
		} else {
			os << std::endl;
		}
	}

	if( i & 0xf )
		os << std::endl;

	os.flags( flags );
	return os;
}

/*EoF*/
