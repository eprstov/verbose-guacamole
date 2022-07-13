#pragma once

#include "connection.h"
#include "../../tick.h"

#include <string_view>

namespace coinbase
{
namespace feeder
{
namespace trait
{

template< typename T >
concept observer = std::is_invocable_v< T, Tick&& >;

} // namespace trait

struct Config : connection::Config
{
	std::string symbol;
};

namespace detail
{
namespace asio = boost::asio;

template< typename T > struct ConnectionAdaptor
{
	void Connected();
	void Received( boost::json::object&& );
};

template< trait::observer Observer >
class Feeder : ConnectionAdaptor<Observer>
{
public:
	Feeder( Config, asio::io_context&, Observer );

	void Start();
	void Stop();

private:
	friend class ConnectionAdaptor<Observer>;
	void Connected();
	void Received( boost::json::object&& );

	using Connection = coinbase::Connection< ConnectionAdaptor<Observer> >;
	std::shared_ptr<Connection> connection;

	Config config;

	asio::io_context& ioctx;

	Observer observer;
};

template< trait::observer Observer >
inline Feeder<Observer>::Feeder( Config cfg, asio::io_context& ioctx, Observer obsvr ) : config( std::move(cfg) ), ioctx(ioctx), observer( std::forward<Observer>(obsvr) )
{
}

template< trait::observer Observer >
inline void Feeder<Observer>::Start()
{
	auto& observer = static_cast< ConnectionAdaptor<Observer>& >( *this );
	connection = std::make_shared<Connection>( ioctx, config, observer );
}

template< trait::observer Observer >
inline void Feeder<Observer>::Stop()
{
	connection->Detach();
	connection.reset();
}

template< trait::observer Observer >
inline void Feeder<Observer>::Connected()
{
	boost::json::object request;
	request[ "type" ] = "subscribe";
	request[ "product_ids" ] = { config.symbol };
	request[ "channels" ] = { std::string_view{ "ticker" } };

	connection->Send(request);
}

template< trait::observer Observer >
inline void Feeder<Observer>::Received( boost::json::object&& message )
{
	if( auto jtype = message.if_contains( "type" ) )
	{
		auto type = jtype->as_string();
		if( type == "ticker" )
		{
			auto& jbid = message[ "best_bid" ];
			auto& jask = message[ "best_ask" ];

			if( jbid.is_string() && jask.is_string() )
			{
				auto& bid = jbid.get_string();
				auto& ask = jask.get_string();

				Tick tick;
				tick.symbol = config.symbol;
				tick.bid = { bid.data(), bid.size() };
				tick.ask = { ask.data(), ask.size() };

				observer( std::move(tick) );
			}
		}
		else if( type == "error" )
		{
			boost::json::string error;

			auto& msg = message[ "message" ];
			if( msg.is_string() )
			{
				error = msg.get_string();
			}

			if( auto& reason = message[ "reason" ]; reason.is_string() )
			{
				if( !error.empty() )
				{
					error += ": ";
				}

				error += reason.get_string();
			}

			throw std::runtime_error( error.c_str() );
		}
	}
}

template< typename T >
inline void ConnectionAdaptor<T>::Connected()
{
	auto* feeder = static_cast< Feeder<T>* >(this);
	feeder->Connected();
}

template< typename T >
inline void ConnectionAdaptor<T>::Received( boost::json::object&& message )
{
	auto* feeder = static_cast< Feeder<T>* >(this);
	feeder->Received( std::move(message) );
}

} // namespace detail
} // namespace feeder

template< feeder::trait::observer Observer >
using Feeder = feeder::detail::Feeder<Observer>;

} // namespace coinbase
