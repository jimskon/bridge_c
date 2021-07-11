
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
		std::ostream* _os;
		int        _level;

	public:

		logger() : _os( &std::clog ), _level(0)
		{ ; }

		virtual ~logger()
		{ ; }

		void level( int level )
		{ _level = level; }

		logger& operator() ( int user ) {
			_os = ( user <= _level ) ? &std::clog : &_null;
			return *this;
		}

		template<class T>
		std::ostream& operator<< ( const T &obj ) {
			*_os << obj;
			return *_os;
		}
};

#endif /*_UBNG_LOGGER_H*/

/*EoF*/
