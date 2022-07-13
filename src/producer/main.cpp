#include "producer.h"
#include "../cmd.h"

#include <string>

int main( int argc, char* argv[] )
{
	struct {} usage;

	try
	{
		if( argc != 3 )
		{
			throw usage;
		}

		std::string symbol{ argv[1] };
		auto [host, port] = split( argv[2], ':' );

		if( symbol.empty() || host.empty() || port.empty() )
		{
			throw usage;
		}

		boost::asio::io_context ioctx;

		Producer::Config config{ "ws-feed.exchange.coinbase.com", "443", "/", symbol, host, port };
		Producer producer( config, ioctx );
		producer.Start();

		boost::asio::signal_set signals( ioctx, SIGINT );
		signals.async_wait( [&] (auto, auto) { producer.Stop(); } );

		ioctx.run();
	}
	catch( const std::exception& ex )
	{
		std::cerr << "error: " << ex.what() << std::endl;
	}
	catch( decltype(usage) )
	{
		std::cerr << "usage: coinbase_ticker_to_udp <symbol> <host>:<port>" << std::endl;
	}

	return 0;
}
