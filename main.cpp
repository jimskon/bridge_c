
#include <cstdlib>
#include <csignal>

#include "ifbridge.h"
#include "iface.h"
#include "logger.h"

#define MESSPORT 9000

#ifdef PROFILE
volatile bool ctrlc = false;
void sigint_handler( int ) { ctrlc = true; }
#endif

int
main( int argc, char *argv[] )
{
	logger  log;
	iface   trunk;
	iface  *ifp;
	brmap   a_brmap( log );

	if( argc < 2 ) {
		std::cerr << "usage: " << argv[0] << " trunk [ iface ... ]" << std::endl
		          << "DEBUG=1 or 2 for debug output"                << std::endl
		;
		exit( 1 );
	}

	const char *debug = getenv( "DEBUG" );
	if( debug != NULL )
		log.level( std::atoi( debug ));

	if( trunk.bind( argv[1] ) != 0 ) {
		std::cerr << "can't bind the trunk" << std::endl;
		exit( 1 );
	}

#ifdef PROFILE
	std::signal( SIGINT, sigint_handler );
#endif

	/*
	 * Now that we have a logger and the trunk, we can
	 * create the bridge and register our interfaces.
	 */

	ifbridge bridge( log, trunk );

	if( bridge.init() != 0 ) {
		std::cerr << "can't initialize bridge" << std::endl;
		exit( 1 );
	}

	for( int i = 2; i < argc; ++i ) {
		ifp = bridge.add_iface( argv[i] );
		if( ifp == (iface*)0 )
			continue;

		log(1) << "PORT: " << *ifp << std::endl;
	}

	/*  Start bridge listener (for vlan assignments) */
	int port = MESSPORT;
	a_brmap.start_listener(port);

	bridge.dump();
	bridge.run();

	return 0;
}

/*EoF*/
