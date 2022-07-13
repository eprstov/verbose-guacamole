#include "producer.h"

namespace detail::producer
{

Producer::Producer( Config cfg, boost::asio::io_context& ioctx ) : config( std::move(cfg) ), ioctx(ioctx)
{
}

void Producer::Start()
{
	auto handler = [this] (auto ec, const auto& endpoints)
	{
		Resolved( ec, endpoints );
	};

	resolver.emplace(ioctx);
	resolver->async_resolve( config.host, config.port, std::move(handler) );
}

void Producer::Stop()
{
	if(socket)
	{
		socket->cancel();
	}

	if(resolver)
	{
		resolver->cancel();
	}

	if(source)
	{
		source->Stop();
		source.reset();
	}
}

void Producer::Resolved( boost::system::error_code error, const asio::ip::udp::resolver::results_type& endpoints )
{
	resolver.reset();

	if( !error )
	{
		auto& observer = static_cast< detail::producer::FeederAdaptor& >( *this );
		source.emplace( config, ioctx, observer );
		source->Start();

		assert( !endpoints.empty() );
		auto&& endpoint = *endpoints.begin();

		socket.emplace(ioctx);
		socket->connect(endpoint);
	}
	else if( error != boost::asio::error::operation_aborted )
	{
		throw std::runtime_error( error.message() );
	}
}

void Producer::Received( Tick&& tick )
{
	std::ostream stream( &buffer );
	Serialize( stream, tick );

	socket->send( buffer.data() );
	buffer.consume( buffer.size() );
}

void detail::producer::FeederAdaptor::operator()( Tick&& tick )
{
	auto* producer = static_cast< Producer* >(this);
	producer->Received( std::move(tick) );
}

} // namespace detail::producer
