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

struct Databinding{
    uint64_t blaaa;
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

    //bool insertDatabinding()
    /*
    To implement!!!


    */
    //to Test
    bool initDB();
protected:

    MYSQL_RES* sendCommand(std::string sendstring);
    bool sendCUD(const std::string& sendstring);
    bool executeScript(const std::string& scriptName);
    static pt::ptree MYSQL_RES_to_ptree(MYSQL_RES* resultset, unsigned int keyColNumber);
private:
    MYSQL* mysqlhandle_m;
    SqlCredentials credentials_m;
    std::mutex mutex_m;
    bool connected_m = false;
    void prepareScript(const std::list<std::string>& src, std::list<std::string>& dest);
    void printSqlError(int ErrCode, const std::string &query);
};
#endif
