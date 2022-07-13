#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <iostream>
#include <memory>

namespace coinbase
{
namespace connection
{
namespace trait
{

template< typename T >
concept observer = std::is_same_v< T, std::decay_t<T> > && requires( T* observer, boost::json::object message )
{
	observer->Connected();
	observer->Received( std::move(message) );
};

} // namespace trait

struct Config
{
	std::string host, port, target;
};

namespace detail
{
namespace beast = ::boost::beast;
namespace asio = ::boost::asio;

using Endpoints = asio::ip::tcp::resolver::results_type;

template< trait::observer Observer >
class Connection : public std::enable_shared_from_this< Connection<Observer> >
{
public:
	Connection( boost::asio::io_context&, Config, Observer& );
	void Detach();

	void Send( const boost::json::object& );

private:
	void NegotiateTls();
	void NegotiateWebsocket();

	void Read();

	void OnResolve( boost::system::error_code, const Endpoints& );

	using std::enable_shared_from_this<Connection>::shared_from_this;

private:
	boost::asio::ssl::context sctx;
	Config config;

	Observer* observer;

	std::optional< std::string > output;
	beast::flat_buffer input;

	boost::asio::ip::tcp::resolver resolver;
	beast::websocket::stream< beast::ssl_stream< beast::tcp_stream > > websocket;
};

template< trait::observer Observer >
inline Connection<Observer>::Connection( asio::io_context& ioctx, Config cfg, Observer& obsvr ) : sctx{ asio::ssl::context::tlsv12_client }, config( std::move(cfg) ), observer( &obsvr ), resolver(ioctx), websocket( ioctx, sctx )
{
	sctx.set_verify_mode( boost::asio::ssl::context::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert );

	auto handler = [this] (auto ec, const auto& endpoints)
	{
		OnResolve( ec, endpoints );
	};

	resolver.async_resolve( config.host, config.port, std::move(handler) );
}

template< trait::observer Observer >
inline void Connection<Observer>::Detach()
{
	assert(observer);
	observer = nullptr;
}

template< trait::observer Observer >
inline void Connection<Observer>::Send( const boost::json::object& message )
{
	auto handler = [this, ths = shared_from_this()] (boost::system::error_code error, std::size_t sz)
	{
		output.reset();
	};

	assert( !output );
	output.emplace( boost::json::serialize(message) );

	websocket.async_write( boost::asio::const_buffers_1{ output->data(), output->size() }, std::move(handler) );
}

template< trait::observer Observer >
inline void Connection<Observer>::OnResolve( boost::system::error_code error, const Endpoints& endpoints )
{
	if( !error )
	{
		assert( !endpoints.empty() );
		auto& endpoint = *endpoints.begin();

		auto& ssl = websocket.next_layer();
		auto& tcp = ssl.next_layer();

		auto handler = [this, ths = shared_from_this()] ( const boost::system::error_code& error )
		{
			if( !error )
			{
				NegotiateTls();
			}
			else if( error != boost::asio::error::operation_aborted )
			{
				throw std::runtime_error( error.message() );
			}
		};

		tcp.async_connect( endpoint, std::move(handler) );
	}
	else if( error != boost::asio::error::operation_aborted )
	{
		throw std::runtime_error( error.message() );
	}
}

template< trait::observer Observer >
inline void Connection<Observer>::NegotiateTls()
{
	auto& ssl = websocket.next_layer();

	if( !SSL_set_tlsext_host_name( ssl.native_handle(), config.host.data() ) )
	 {
		 boost::system::error_code error{ static_cast<int>( ::ERR_get_error() ), asio::error::get_ssl_category() };
		 throw std::runtime_error( error.message() );
	 }

	auto handler = [this, ths = shared_from_this()] (const boost::system::error_code& error)
	{
		if( !error )
		{
			NegotiateWebsocket();
		}
		else if( error != boost::asio::error::operation_aborted )
		{
			throw std::runtime_error( error.message() );
		}
	};

	ssl.async_handshake( boost::asio::ssl::stream_base::client, std::move(handler) );
}

template< trait::observer Observer >
inline void Connection<Observer>::NegotiateWebsocket()
{
	auto handler = [this, ths = shared_from_this()] (const boost::system::error_code& error)
	{
		if( !error )
		{
			if(observer)
			{
				observer->Connected();
				Read();
			}
		}
		else if( error != boost::asio::error::operation_aborted )
		{
			throw std::runtime_error( error.message() );
		}
	};

	websocket.async_handshake( config.host, config.target, handler );
}

template< trait::observer Observer >
inline void Connection<Observer>::Read()
{
	auto handler = [this, ths = shared_from_this()] ( boost::system::error_code error, std::size_t sz )
	{
		if(error)
		{
			if( error != boost::asio::error::operation_aborted )
			{
				throw std::runtime_error( error.message() );
			}
		}
		else if(observer)
		{
			auto buffer = input.cdata();
			static_assert( std::is_same_v< std::remove_cvref_t< decltype(buffer) >, boost::asio::const_buffer > );

			auto* text = static_cast< const char* >( buffer.data() );
			auto message = boost::json::parse( { text, buffer.size() } ).as_object();
			observer->Received( std::move(message) );

			input.consume(sz);
			Read();
		}
	};

	websocket.async_read( input, std::move(handler) );
}

} // namespace detail
} // namespace connection

template< connection::trait::observer Observer >
using Connection = connection::detail::Connection<Observer>;

} // namespace coinbase
