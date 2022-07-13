#include "consumer.h"
#include "../cmd.h"

#include <string>

int main( int argc, char* argv[] )
{
	struct {} usage;

	try
	{
		if (argc != 2)
		{
			throw usage;
		}

		auto [ip, port] = split( argv[1], ':' );
		if( ip.empty() || port.empty() )
		{
			throw usage;
		}

		boost::asio::ip::udp::endpoint endpoint( boost::asio::ip::address::from_string(ip), std::stoi(port) );

		boost::asio::io_context ioctx;

		Consumer consumer( ioctx, endpoint );
		consumer.Start();

		boost::asio::signal_set signals( ioctx, SIGINT );
		signals.async_wait( [&] (auto, auto) { consumer.Stop(); } );

		ioctx.run();
	}
	catch( std::exception& e )
	{
		std::cerr << "error: " << e.what() << "\n";
	}
	catch( decltype(usage) )
	{
		std::cerr << "Usage: consume_coinbase_udp_stream <interface>:<port>" << std::endl;
	}

	return 0;
}
