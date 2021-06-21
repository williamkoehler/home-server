#pragma once
#include <iostream>
#include <string>
#include <ostream>

//Dependencies

// boost rapidjson xxHash etc.
#include <common.h>

//IP
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

typedef boost::beast::tcp_stream tcp_socket_t;
typedef boost::beast::ssl_stream<tcp_socket_t> ssl_socket_t;
typedef boost::beast::websocket::stream<ssl_socket_t> websocket_t;

typedef boost::asio::ip::udp::socket udp_socket_t;

#include <openssl/ssl.h>

#ifdef _WIN32
#include <Windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#endif