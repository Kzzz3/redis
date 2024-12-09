#include "server.h"
#include "command.h"

void CmdSet(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
		db->kvstore[Sds::create(cmd[1])] = StringObjectCreate(cmd[2]);
	else
		db->kvstore[cmd[1]] = StringObjectUpdate(db->kvstore[cmd[1]], cmd[2]);
}

void CmdGet(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	auto reply = db->kvstore.contains(cmd[1]) ?
		GenerateReply(StringObjectGet(db->kvstore[cmd[1]])) : GenerateErrorReply("nil");

	conn->AsyncSend(std::move(reply));
}

void CmdIncr(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
			obj->data.num++;
		else
			conn->AsyncSend(GenerateErrorReply("value is not an integer"));
	}
	else
	{
		conn->AsyncSend(GenerateErrorReply("nil"));
	}
}

void CmdDecr(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
			obj->data.num--;
		else
			conn->AsyncSend(GenerateErrorReply("value is not an integer"));
	}
	else
	{
		conn->AsyncSend(GenerateErrorReply("nil"));
	}
}

void CmdAppend(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->encoding == ObjEncoding::REDIS_ENCODING_RAW)
		{
			auto value = reinterpret_cast<Sds*>(obj->data.ptr);
			obj->data.ptr = value->append(cmd[2]);
		}
		else
		{
			conn->AsyncSend(GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value"));
		}
	}
	else
	{
		conn->AsyncSend(GenerateErrorReply("nil"));
	}
}