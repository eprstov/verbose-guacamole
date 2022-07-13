#pragma once

#include <iostream>
#include <string_view>
#include <vector>

struct Tick
{
	std::string_view symbol, bid, ask;
};

void Serialize( std::ostream&, std::string_view );
void Serialize( std::ostream&, const Tick& );

template< typename T, typename Iterator >
requires std::is_same_v< T, std::string_view >
inline std::string_view Deserialize( Iterator&& it, std::remove_cvref_t<Iterator> end )
{
	if( it != end )
	{
		auto sz = std::size_t( *it++ );
		if( sz <= std::size_t( end - it ) )
		{
			auto data = &*it;
			std::advance( it, sz );
			return { data, sz };
		}
	}

	throw std::runtime_error( "format mismatch" );
}

template< typename T, typename Iterator >
requires std::is_same_v< T, Tick >
inline Tick Deserialize( Iterator&& it, std::remove_cvref_t<Iterator> end )
{
	Tick result;
	result.symbol = Deserialize< std::string_view >( it, end );
	result.bid = Deserialize< std::string_view >( it, end );
	result.ask = Deserialize< std::string_view >( it, end );

	return result;
}
