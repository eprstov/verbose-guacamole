#pragma once

#include "coinbase/feeder.h"

#include <boost/asio.hpp>

#include <optional>

namespace detail::producer
{

struct FeederAdaptor
{
	void operator()( Tick&& );
};

namespace asio = ::boost::asio;

class Producer : detail::producer::FeederAdaptor
{
	friend struct detail::producer::FeederAdaptor;

public:
	struct Config : coinbase::feeder::Config
	{
		std::string host, port;
	};

	Producer( Config, boost::asio::io_context& );

	void Start();
	void Stop();

private:
	using Source = coinbase::Feeder< detail::producer::FeederAdaptor& >;

	void Resolved( boost::system::error_code, const asio::ip::udp::resolver::results_type& );
	void Received( Tick&& );

	Config config;
	boost::asio::io_context& ioctx;

	std::optional<Source> source;
	std::optional< boost::asio::ip::udp::resolver > resolver;

	boost::asio::streambuf buffer;
	std::optional< boost::asio::ip::udp::socket > socket;
};

} // namespace detail::producer

using Producer = detail::producer::Producer;
