#pragma once

#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;
namespace posix_time = boost::posix_time;

class pinger
{
public:
	pinger(boost::asio::io_service& io_service, const std::string& destination);
	bool isAvailableDestination() const noexcept;

private:
	void start_send();

	void handle_timeout();

	void start_receive();

	void handle_receive(std::size_t length);

	static unsigned short get_identifier();

	icmp::resolver resolver_;
	icmp::endpoint destination_;
	icmp::socket socket_;
	deadline_timer timer_;
	unsigned short sequence_number_;
	posix_time::ptime time_sent_;
	boost::asio::streambuf reply_buffer_;
	std::size_t num_replies_;
};