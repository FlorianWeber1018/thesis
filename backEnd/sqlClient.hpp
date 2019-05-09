#include "globalInclude.hpp"
#include <mariadb/mysql.h>
#ifndef sqlClient__hpp
#define sqlClient__hpp_
struct SqlCredentials{
    SqlCredentials(
                  std::string userName,
                  std::string password,
                  std::string host,
                  unsigned int port,
                  std::string database);
    std::string userName;
    std::string password;
    std::string host;
    unsigned int port;
    std::string database;
};


class SqlClient
{
public:
    SqlClient(const SqlCredentials& credentials);
    SqlClient();
    ~SqlClient();
    void connect() noexcept;
    void disconnect() noexcept;
    bool connected();
    bool initDB();

    //to Test
    bool insertDatabinding(uint64_t srcID, uint64_t destID, const DataBindingsType& databindingType);
    bool deleteDatabinding(uint64_t ID);
    bool deleteDatabinding(uint64_t destID, const DataBindingsType& databindingType);
    bool getAllRowsOfTable(const std::string& tableName, rj::Document& dom_o);
protected:
    MYSQL_RES* sendCommand(std::string& sendstring);
    bool sendCUD(const std::string& sendstring);
    bool executeScript(const std::string& scriptName);
    bool mysqlResToDom(MYSQL_RES* resultset, unsigned int keyColNumber, rj::Document& dom_o);
private:
    MYSQL* mysqlhandle_m;
    SqlCredentials credentials_m;
    std::mutex mutex_m;
    bool connected_m = false;
    void prepareScript(const std::list<std::string>& src, std::list<std::string>& dest);
    void printSqlError(int ErrCode, const std::string &query);
};
#endif