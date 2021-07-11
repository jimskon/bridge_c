
#ifndef _UBNG_PACKET_H
#define _UBNG_PACKET_H

#include <cstdlib>
#include <cstdint>
#include <ostream>

#include <linux/if_packet.h>

struct packet
{
	uint8_t *_x;   // buffer
	size_t   _cap; // capacity
	size_t   _len; // length

	struct sockaddr_ll _sll; // link-layer info

	packet( size_t cap );
	packet( uint8_t *x, size_t cap );
	virtual ~packet();

	virtual std::ostream& dump( std::ostream& os ) const;

	friend std::ostream& operator<< ( std::ostream& os, const packet& pkt );
};

// Ethernet

struct packet_eth final : public packet
{
	packet_eth( uint8_t *x, size_t cap ) : packet( x, cap )
	{ ; }

	packet_eth( const packet_eth& other )
		: packet( other._x, other._cap )
	{ ; }

	virtual ~packet_eth()
	{ ; }

	std::ostream& dump( std::ostream& os ) const;
};

std::ostream& operator<< ( std::ostream& os, const struct ethhdr* eth );

// IP

struct packet_ip final : public packet
{
	packet_ip( uint8_t *x, size_t cap ) : packet( x, cap )
	{ ; }

	virtual ~packet_ip()
	{ ; }

	std::ostream& dump( std::ostream& os ) const;
};

// UDP

struct packet_udp final : public packet
{
	packet_udp( uint8_t *x, size_t cap ) : packet( x, cap )
	{ ; }

	virtual ~packet_udp()
	{ ; }

	std::ostream& dump( std::ostream& os ) const;
};

// TCP

struct packet_tcp final : public packet
{
	packet_tcp( uint8_t *x, size_t cap ) : packet( x, cap )
	{ ; }

	virtual ~packet_tcp()
	{ ; }

	std::ostream& dump( std::ostream& os ) const;
};

#endif /*_UBNG_PACKET_H*/

/*EoF*/
