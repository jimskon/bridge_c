
#ifndef _UBNG_LOGGER_H
#define _UBNG_LOGGER_H

#include <iostream>
#include <cstdint>

class logger
{
	private:

		int _level;
		int _user;

	public:

		logger();
		virtual ~logger();

		void hexdump( const uint8_t *x, size_t len );

		void level( int level )
		{ _level = level; }

		logger& operator() ( int user ) {
			_user = user;
			return *this;
		}

		template<class T>
		logger& operator<< ( const T &obj ) {
			if( _user >= _level )
				std::cerr << obj << std::endl;
			return *this;
		}
};

#endif /*_UBNG_LOGGER_H*/

/*EoF*/
