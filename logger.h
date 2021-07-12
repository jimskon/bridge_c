
#ifndef _UBNG_LOGGER_H
#define _UBNG_LOGGER_H

#include <iostream>
#include <cstdint>

class logger : public std::ostream
{
	private:

		struct NullBuffer : public std::streambuf
		{ int overflow( int c ) { return c; } };

		class NullStream : public std::ostream
		{
			private:
				NullBuffer _null;

			public:
				NullStream() : std::ostream( &_null )
				{ ; }

				virtual ~NullStream()
				{ ; }
		};

		NullStream _null;
		int        _level;

	public:

		logger() : _level(0)
		{ ; }

		virtual ~logger()
		{ ; }

		void level( int level )
		{ _level = level; }

		std::ostream& operator() ( int user ) {
			if( user <= _level ) {
				return std::clog;
			} else {
				return _null;
			}
		}
};

#endif /*_UBNG_LOGGER_H*/

/*EoF*/
