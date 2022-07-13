#include "tick.h"

void Serialize( std::ostream& stream, std::string_view text )
{
	auto sz = std::uint8_t( text.size() );
	stream.put(sz);
	stream.write( text.data(), sz );
}

void Serialize( std::ostream& stream, const Tick& tick )
{
	Serialize( stream, tick.symbol );
	Serialize( stream, tick.bid );
	Serialize( stream, tick.ask );
}

