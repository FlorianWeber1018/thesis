#ifndef backend__hpp
#define backend__hpp
#include "opcuaServer.hpp"
#include "websocketServer.hpp"
#include "sqlClient.hpp"
#include "httpServer.hpp"
class Backend : protected OpcuaServer, protected WebsocketServer, protected SqlClient, protected HttpServer{
public:
    Backend();
protected:
    void traverseOpcuaPagesFromSql(uint64_t startPageID = 0);
private:

};
#endif
