#ifndef backend__hpp
#define backend__hpp
#include "opcuaServer.hpp"
#include "websocketServer.hpp"
#include "sqlClient.hpp"

class Backend : protected OpcuaServer, protected WebsocketServer, protected SqlClient{
public:
    Backend();
protected:
    void traverseOpcuaPagesFromSql(uint64_t startPageID = 0);
    virtual void opcua_dispatch(const opcua_changeRequest& changeRequest) override;
    virtual void ws_dispatch(const ws_message& msg, std::shared_ptr<ws_session> ws_session_) override;
    virtual void sql_dispatch(const sql_message& msg) override;
private:

};
#endif
