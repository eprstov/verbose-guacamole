#include "consumer.h"

#include <iostream>

Consumer::Consumer( boost::asio::io_context& ioctx, boost::asio::ip::udp::endpoint endpoint ) : socket( ioctx, endpoint )
{
}

void Consumer::Start()
{
	Read();
}

void Consumer::Stop()
{
	socket.cancel();
}

void Consumer::Read()
{
	auto handler = [this] ( boost::system::error_code error, std::size_t sz )
	{
		if( !error )
		{
			buffer.resize(sz);

			auto tick = Deserialize<Tick>( buffer.begin(), buffer.end() );
			std::cout << tick.symbol << "  " << tick.bid << "  " << tick.ask << std::endl;

			Read();
		}
		else if( error != boost::asio::error::operation_aborted )
		{
			throw std::runtime_error( error.message() );
		}
	};

	buffer.resize(1024);
	socket.async_receive( boost::asio::mutable_buffer( buffer.data(), buffer.size() ), std::move(handler) );
}
