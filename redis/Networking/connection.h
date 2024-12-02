#pragma once 

#include <asio.hpp>

#include "../db.h"

using asio::ip::tcp;
using asio::streambuf;
using std::unique_ptr;

constexpr uint64_t BUFFER_MAX_SIZE = 1024 * 1024;

enum class ConnectionState
{
    CONN_STATE_NONE,
    CONN_STATE_CONNECTING,
    CONN_STATE_ACCEPTING,
    CONN_STATE_CONNECTED,
    CONN_STATE_CLOSED,
    CONN_STATE_ERROR
};

class Connection
{
public:
    uint64_t id;
	tcp::socket socket;
	ConnectionState state;
	unique_ptr<streambuf> read_buffer;
	unique_ptr<streambuf> write_buffer;

	RedisDb* db;

public:
    Connection(uint64_t id, tcp::socket&& socket): 
		id(id),
		socket(std::move(socket)),
        state(ConnectionState::CONN_STATE_NONE), 
        read_buffer(std::make_unique<streambuf>(BUFFER_MAX_SIZE)),
		write_buffer(std::make_unique<streambuf>(BUFFER_MAX_SIZE))
    {
    }
};