#pragma once

#include <tick.h>

#include <boost/asio.hpp>

#include <vector>

class Consumer
{
public:
	Consumer( boost::asio::io_context&, boost::asio::ip::udp::endpoint );

	void Start();
	void Stop();

private:
	void Read();

	std::vector<char> buffer;
	boost::asio::ip::udp::socket socket;
};
