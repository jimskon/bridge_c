
#include "packet.h"

//  - - - - - - -  //
//  M E T H O D S  //
//  - - - - - - -  //

std::ostream&
packet_ip::dump( std::ostream& os ) const
{
	os << "IP packet" << std::endl;
	return os;
}

/*EoF*/
