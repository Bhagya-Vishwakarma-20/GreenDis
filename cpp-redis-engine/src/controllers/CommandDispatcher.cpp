#include "CommandDispatcher.h"
#include "../utils/Logger.h"

namespace redis_engine::controllers {

CommandDispatcher::CommandDispatcher(std::shared_ptr<services::KeyValueService> kv_service,
                                     std::shared_ptr<services::ExpirationService> exp_service,
                                     std::shared_ptr<persistence::AOFWriter> aof_writer)
    : kv_service_(std::move(kv_service)),
      exp_service_(std::move(exp_service)),
      aof_writer_(std::move(aof_writer)) {}

void CommandDispatcher::Dispatch(std::shared_ptr<server::Connection> conn, const protocol::Request& req) {
    if (req.IsEmpty()) return;

    if (req.command == "SET") {
        HandleSet(conn, req);
    } else if (req.command == "GET") {
        HandleGet(conn, req);
    } else if (req.command == "DEL") {
        HandleDel(conn, req);
    } else if (req.command == "EXPIRE") {
        HandleExpire(conn, req);
    } else if (req.command == "PING") {
        HandlePing(conn, req);
    } else {
        SendError(conn, "unknown_command");
    }
}

void CommandDispatcher::HandleSet(std::shared_ptr<server::Connection> conn, const protocol::Request& req) {
    if (req.args.size() < 2) {
        SendError(conn, "wrong_number_of_arguments");
        return;
    }
    const std::string& key = req.args[0];
    const std::string& val = req.args[1];
    
    kv_service_->Set(key, val);
    if (aof_writer_) {
        aof_writer_->AppendSet(key, val);
    }
    SendOk(conn);
}

void CommandDispatcher::HandleGet(std::shared_ptr<server::Connection> conn, const protocol::Request& req) {
    if (req.args.size() < 1) {
        SendError(conn, "wrong_number_of_arguments");
        return;
    }
    const std::string& key = req.args[0];
    auto val = kv_service_->Get(key);
    
    protocol::Response res;
    if (val) {
        res.data = val.value();
    } else {
        res.is_error = true;
        res.data = "key_not_found";
    }
    conn->Send(res.Serialize());
}

void CommandDispatcher::HandleDel(std::shared_ptr<server::Connection> conn, const protocol::Request& req) {
    if (req.args.size() < 1) {
        SendError(conn, "wrong_number_of_arguments");
        return;
    }
    const std::string& key = req.args[0];
    bool deleted = kv_service_->Delete(key);
    
    if (deleted && aof_writer_) {
        aof_writer_->AppendDel(key);
    }
    
    protocol::Response res;
    res.data = deleted ? "1" : "0";
    conn->Send(res.Serialize());
}

void CommandDispatcher::HandleExpire(std::shared_ptr<server::Connection> conn, const protocol::Request& req) {
    if (req.args.size() < 2) {
        SendError(conn, "wrong_number_of_arguments");
        return;
    }
    const std::string& key = req.args[0];
    int ttl = 0;
    try {
        ttl = std::stoi(req.args[1]);
    } catch (...) {
        SendError(conn, "invalid_ttl");
        return;
    }

    bool success = exp_service_->Expire(key, ttl);
    if (success && aof_writer_) {
        aof_writer_->AppendExpire(key, ttl);
    }

    protocol::Response res;
    res.data = success ? "1" : "0";
    conn->Send(res.Serialize());
}

void CommandDispatcher::HandlePing(std::shared_ptr<server::Connection> conn, const protocol::Request& req) {
    protocol::Response res;
    res.data = req.args.empty() ? "PONG" : req.args[0];
    conn->Send(res.Serialize());
}

void CommandDispatcher::SendError(std::shared_ptr<server::Connection> conn, const std::string& msg) {
    protocol::Response res;
    res.is_error = true;
    res.data = msg;
    conn->Send(res.Serialize());
}

void CommandDispatcher::SendOk(std::shared_ptr<server::Connection> conn) {
    protocol::Response res;
    res.data = "OK";
    conn->Send(res.Serialize());
}

} // namespace redis_engine::controllers
