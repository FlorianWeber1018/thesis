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
    std::string host;
    unsigned int port;
    std::string userName;
    std::string password;
    std::string database;
};
class SqlClient
{
public:
    SqlClient(const SqlCredentials& credentials);
    SqlClient();
    ~mysqlcon();
protected:

    void connect() noexcept;
    void disconnect() noexcept;
    MYSQL_RES* sendCommand(std::string sendstring);
    bool sendCUD(const std::string& sendstring);
    static pt::ptree MYSQL_RES_to_ptree(MYSQL_RES* resultset, unsigned int keyColNumber);
private:
    MYSQL* mysqlhandle_m;
    SqlCredentials credentials_m;
    std::mutex mutex_m;
    bool connected_m = false;
};
#endif
