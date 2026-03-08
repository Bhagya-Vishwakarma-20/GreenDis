#pragma once

#include "../server/Connection.h"
#include "../protocol/CommandParser.h"
#include "../services/KeyValueService.h"

#include "../persistence/AOFWriter.h"
#include <memory>

namespace redis_engine::controllers {

class CommandDispatcher {
public:
    CommandDispatcher(std::shared_ptr<services::KeyValueService> kv_service,
                      std::shared_ptr<services::ExpirationService> exp_service,
                      std::shared_ptr<persistence::AOFWriter> aof_writer);

    void Dispatch(std::shared_ptr<server::Connection> conn, const protocol::Request& req);

private:
    void HandleSet(std::shared_ptr<server::Connection> conn, const protocol::Request& req);
    void HandleGet(std::shared_ptr<server::Connection> conn, const protocol::Request& req);
    void HandleDel(std::shared_ptr<server::Connection> conn, const protocol::Request& req);
    void HandleExpire(std::shared_ptr<server::Connection> conn, const protocol::Request& req);
    void HandlePing(std::shared_ptr<server::Connection> conn, const protocol::Request& req);
    
    void SendError(std::shared_ptr<server::Connection> conn, const std::string& msg);
    void SendOk(std::shared_ptr<server::Connection> conn);

    std::shared_ptr<services::KeyValueService> kv_service_;
    std::shared_ptr<services::ExpirationService> exp_service_;
    std::shared_ptr<persistence::AOFWriter> aof_writer_;
};

} // namespace redis_engine::controllers
