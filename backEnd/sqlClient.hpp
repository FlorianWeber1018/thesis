#include "globalInclude.hpp"
#include <mariadb/mysql.h>
#ifndef sqlClient__hpp
#define sqlClient__hpp_
struct SqlCredentials{
    SqlCredentials(
                  std::string userName,
                  std::string password,
                  std::string host,
                  unsigned int port);
    std::string userName;
    std::string password;
    std::string host;
    unsigned int port;
};


class SqlClient
{
public:
    SqlClient(const SqlCredentials& credentials);
    SqlClient();
    ~SqlClient();

protected:
    void connect() noexcept;
    void disconnect() noexcept;
    bool connected();
    bool initDB();

    void updateParamNode;

    //to Test
    bool getAllRowsOfTable(const std::string& tableName, rj::Document& dom_o);
    bool createInstanceOfGuiElement(const std::string& type, uint64_t pageSqlID, const std::string& name);
    bool validateCredentials(std::string userName, std::string pw);
    MYSQL_RES* getDataNodes(uint64_t guiElementID);
    MYSQL_RES* getGuiElements(uint64_t pageID);
    MYSQL_RES* getPages(uint64_t pageID);

    bool itterateThroughMYSQL_RES(MYSQL_RES* resultSet, std::function<void(const MYSQL_ROW&)> callback, bool freeResultSetAfter = true);

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
    void escapeString( std::string& str );
};
#endif
