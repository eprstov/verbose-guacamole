#pragma once

#include <string>
#include <string_view>
#include <tuple>

inline std::tuple< std::string, std::string > split( std::string_view text, char delim )
{
	if( auto pos = text.find(delim); pos != text.npos )
	{
		std::string_view left{ text.data(), pos };
		std::string_view right{ text.data() + pos + 1, text.size() - pos - 1 };
		return { std::string(left), std::string(right) };
	}

	return { std::string(text), {} };
}
