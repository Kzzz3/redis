#include "server.h"

Server server;
uint32_t DATABASE_NUM = 16;

Server::Server() :
	connection_id(0),
	databases(DATABASE_NUM),
	exec_threadpool(1)
{
}

void Server::start()
{
    co_spawn(io_context, listener(), detached);
	asio::signal_set signals(io_context, SIGINT, SIGTERM);
	signals.async_wait([&](const asio::error_code&, int) { io_context.stop(); });
	
	io_context.run();
}


awaitable<void> Server::listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { tcp::v4(), 10087 });
    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
		shared_ptr<Connection> conn = std::make_shared<Connection>(connection_id++, std::move(socket));

        co_spawn(executor, handleConnection(conn), detached);
    }
}

awaitable<void> Server::handleConnection(shared_ptr<Connection> conn) {
	conn->state = ConnectionState::CONN_STATE_CONNECTED;

	for (;;) {
		//read mutibulk len
		Command cmd = co_await readCommandFromClient(conn);

		//command process
		std::function<void(shared_ptr<Connection> conn, Command&)> handler = CommandProcess(cmd);

		//execute command
		if (handler) {
			asio::post(exec_threadpool, [conn, handler, cmd]() mutable {
				handler(conn, cmd);
				for (auto& sds : cmd)
					Sds::destroy(sds);
			});
		}
		else {
			//err
		}

	}
}

awaitable<Command> Server::readCommandFromClient(shared_ptr<Connection> conn)
{
	size_t n = co_await async_read_until(conn->socket, *conn->read_buffer, "\r\n", use_awaitable);
	if (n == 0) {
        // close command
	}

	const char* data = asio::buffer_cast<const char*>(conn->read_buffer->data());
	if (data[0] != '*') {
		//err
	}
	
	size_t bulkSize = std::stoi(std::string(data + 1, n - 3), nullptr, 10);
	if (bulkSize <= 0) {
		//err
	}
	conn->read_buffer->consume(n);

	Command cmd;
	cmd.reserve(bulkSize);

	int64_t bulkLen = 0;
	for (size_t i = 0; i < bulkSize; i++) {
		n = co_await async_read_until(conn->socket, *conn->read_buffer, "\r\n", use_awaitable);
		if (n == 0) {
			// close command
		}

		optional<int64_t> num;
		data = asio::buffer_cast<const char*>(conn->read_buffer->data());
		switch (data[0])
		{
		case '+':
			cmd.emplace_back(Sds::create(data + 1, n - 3, n - 3));
			conn->read_buffer->consume(n);
			break;
		case ':':
			cmd.emplace_back(Sds::create(data + 1, n - 3, n - 3));
			conn->read_buffer->consume(n);
			break;
		case '$':
			num = str2num<int64_t>(data + 1, n - 3);
			if (!num.has_value() || num.value() <= 0) {
				//err
			}

			bulkLen = num.value();
			conn->read_buffer->consume(n);
			if (conn->read_buffer->in_avail() < bulkLen + 2) {
				n = co_await async_read(conn->socket, *conn->read_buffer, asio::transfer_exactly(bulkLen + 2 - conn->read_buffer->in_avail()), use_awaitable);
				if (n == 0) {
					// close command
				}
			}

			data = asio::buffer_cast<const char*>(conn->read_buffer->data());
			cmd.emplace_back(Sds::create(data, bulkLen, bulkLen));
			conn->read_buffer->consume(bulkLen + 2);
			break;
		default:
			break;
		}
	}
	co_return cmd;
}

RedisDb* Server::selectDb(Sds* key)
{
	return &databases[std::hash<Sds*>{}(key) % DATABASE_NUM];
}

std::function<void(shared_ptr<Connection>, Command&)> Server::CommandProcess(Command& cmd)
{
	return GetCommandHandler(cmd[0]);
}
