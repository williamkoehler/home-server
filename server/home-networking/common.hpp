#pragma once
#include <home-common/common.hpp>

typedef boost::beast::tcp_stream tcp_socket_t;
typedef boost::beast::ssl_stream<tcp_socket_t> ssl_socket_t;
typedef boost::beast::websocket::stream<tcp_socket_t> websocket_t;

typedef boost::asio::ip::udp::socket udp_socket_t;

#include <openssl/ssl.h>