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
enum sqlEvent
{
    sqlEvent_invalid = 0,
    sqlEvent_paramNodeChange = 1,
    sqlEvent_pageChange = 2
};
struct sql_message{
    sql_message(sqlEvent event, const std::string& singlePayload);
    sql_message(sqlEvent event, const std::string& firstPayload, const std::string& secondPayload);
    sql_message(sqlEvent event, const std::vector<std::string>& payloadVec);
    sql_message(const sql_message& otherMsg);
    sqlEvent event_m = sqlEvent_invalid;
    std::vector<std::string> payload_m;
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



    // INTERFACE FOR BACKEND
    //  all strings must beeing passed by value to make it possible to escape the strings in the individual methods
    bool updateParamNode(std::string paramNodeSqlID, std::string newParamValue);
    bool validateCredentials(std::string userName, std::string pw);
    bool pageExists(std::string pageID);
    void getDataNodeIDs(std::shared_ptr<std::set<std::string> >  outDnIds, std::string pageID);
    void getParamNodeIDs(std::shared_ptr<std::set<std::string> > outPnIds, std::string pageID);
    virtual void sql_dispatch(const sql_message& msg) = 0; //overloaded in backend
    void getStructureOfPage(std::string pageID, rj::Document& outDom);
    //_____________________________
    bool entryExists(const std::string& tableName, const std::string& keyColName, const std::string& keyVal);

    bool getAllRowsOfTable(const std::string& tableName, rj::Document& dom_o);

    bool createInstanceOfGuiElement(const std::string& type, uint64_t pageSqlID, const std::string& name);
    MYSQL_RES* getParams(uint64_t guiElementID);
    MYSQL_RES* getDataNodes(uint64_t guiElementID);
    MYSQL_RES* getGuiElements(uint64_t pageID);
    MYSQL_RES* getPages(uint64_t pageID);

    MYSQL_RES* getParams(const std::string& guiElementID);
    MYSQL_RES* getDataNodes(const std::string& guiElementID);
    MYSQL_RES* getGuiElements(const std::string& pageID);
    MYSQL_RES* getPages(const std::string& pageID);

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
