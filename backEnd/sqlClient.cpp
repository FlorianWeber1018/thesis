#include "globalInclude.hpp"
#include "sqlClient.hpp"
#include "rapidjson/document.h"
SqlCredentials::SqlCredentials(std::string userName = "WebVisuSU",
        std::string password = "637013",
        std::string host = "localhost",
        unsigned int port = 3306)
{
    this->userName = userName;
    this->password = password;
    this->host = host;
    this->port = port;
}
SqlClient::SqlClient(const SqlCredentials& credentials)
{
    this->credentials_m = credentials;

    mysqlhandle_m = mysql_init(nullptr);
    if(mysqlhandle_m == nullptr){
        throw 100;
    }
    connect();
}
SqlClient::SqlClient()
{
    mysqlhandle_m = mysql_init(nullptr);
    if(mysqlhandle_m == nullptr){
        throw "100";
    }
    connect();
}
SqlClient::~SqlClient()
{
    disconnect();
}
void SqlClient::connect() noexcept
{
    mutex_m.lock();
    std::lock_guard<std::mutex> lg(mutex_m, std::adopt_lock);

    if (mysqlhandle_m == nullptr)
    {
        return;
    }

    //Connect to the database
    if (mysql_real_connect(mysqlhandle_m,
                           credentials_m.host.c_str(),
                           credentials_m.userName.c_str(),
                           credentials_m.password.c_str(),
                           nullptr,
                           credentials_m.port, nullptr, 0)
        == nullptr)
    {
        this->connected_m = true;//wird in disconnect wieder false
        this->disconnect();
    }else{
        this->connected_m = true;
    }
}
void SqlClient::disconnect() noexcept
{
    if(this->connected_m){
        mysql_close(mysqlhandle_m);
        this->connected_m = false;
    }
}
bool SqlClient::connected()
{
    return connected_m;
}
MYSQL_RES* SqlClient::sendCommand(std::string& sendstring)
{
    mutex_m.lock();
    std::lock_guard<std::mutex> lg(mutex_m, std::adopt_lock);
    if(connected_m){
        while(mysql_next_result(mysqlhandle_m) == 0){

        }
        int ErrCode = mysql_query(mysqlhandle_m, sendstring.c_str());
        if (ErrCode == 0)
        {
            return mysql_store_result(mysqlhandle_m);
        }
        printSqlError(ErrCode,sendstring);
        std::cout << mysql_error(this->mysqlhandle_m);
        return nullptr;
    }
    return nullptr;
}
bool SqlClient::sendCUD(const std::string& sendstring)
{
    mutex_m.lock();
    std::lock_guard<std::mutex> lg(mutex_m, std::adopt_lock);
    if(connected_m){
        int ErrCode = mysql_query(mysqlhandle_m, sendstring.c_str());
        if(ErrCode){
            printSqlError(ErrCode, sendstring);

            disconnect();//reconnect
            connect();//reconnect
            ErrCode = mysql_query(mysqlhandle_m, sendstring.c_str());//try again
            if(ErrCode){
                return false;
            }else{
                return true;
            }
        }else{
            return true;
        }
    }else{
        return false;
    }
}
bool SqlClient::executeScript(const std::string& scriptName)
{
    std::string filePath = "../sqlScripts/";
    filePath.append(scriptName);
    std::list<std::string> linesOfFile;
    std::list<std::string> queryList;

    if(util::RwFile::readFile(filePath,linesOfFile)){
        prepareScript(linesOfFile, queryList);
        for(auto&& query : queryList){
            if(!sendCUD(query)){
                return false;
            }
        }
    }else{
        return false;
    }

    return true;
}
bool SqlClient::validateCredentials(std::string userName, std::string pw)
{
    escapeString(userName);
    escapeString(pw);

    std::string query = "Select count(*) from Credentials where userName = '";
    query.append( userName);
    query.append("' and pwHash = MD5('");
    query.append(pw);
    query.append("')");

    MYSQL_RES* result = sendCommand(query);
    if(result != nullptr){
        bool res = false;
        if(MYSQL_ROW row = mysql_fetch_row(result)){
            if(row[0]!=nullptr){
                std::string resultStr(row[0]);
                if(resultStr == "1"){
                    res = true;
                }
            }
        }
        mysql_free_result(result);
        return res;
    }else{
        return false;
    }
}
bool SqlClient::mysqlResToDom(MYSQL_RES* resultset, unsigned int keyColNumber, rj::Document& dom_o)
{
    dom_o.SetObject();

    if(resultset != nullptr){
        unsigned int fieldCount = mysql_num_fields(resultset);
        std::vector<MYSQL_FIELD*> fields;
        MYSQL_FIELD* field;

        while((field = mysql_fetch_field(resultset)) != nullptr){
            fields.push_back(field);
        }
        while (MYSQL_ROW row = mysql_fetch_row(resultset)) {
            rj::Value jsonRow(rj::kObjectType);
            rj::Value rowKey;
            rowKey.SetString(row[keyColNumber], dom_o.GetAllocator());
            for(uint i=0; i<fieldCount; i++){
                rj::Value val;
                rj::Value colKey;
                colKey.SetString(fields[i]->name, dom_o.GetAllocator());
                if(row[i]!=nullptr){
                    if(fields[i]->type == MYSQL_TYPE_LONGLONG){
                        val.SetUint64(std::stoull(row[i]));
                    }else{
                        val.SetString(row[i], dom_o.GetAllocator());
                    }
                }
                jsonRow.AddMember(colKey, val, dom_o.GetAllocator());
            }
            dom_o.AddMember(rowKey, jsonRow, dom_o.GetAllocator());
        }
        return true;
    }else{
        return false;
    }

}
void SqlClient::prepareScript(const std::list<std::string>& src, std::list<std::string>& dest)
{
    std::string tempQuery = "";
    std::string delimiter = ";";
    for(auto&& srcLine : src){
        std::string tempSrcLine = srcLine.substr(0, srcLine.find("-- ", 0));//create substring without comments
        tempSrcLine = tempSrcLine.substr(0, tempSrcLine.find("\r",0)); //remove \r and everything after(normaly there isnt anything after \r)
        tempSrcLine = tempSrcLine.substr(
                    tempSrcLine.find_first_not_of(" ") > tempSrcLine.length() ?
                        tempSrcLine.length() : tempSrcLine.find_first_not_of(" "),
                    tempSrcLine.length()); //remove " " in front of the line
        tempSrcLine = tempSrcLine.substr(0, tempSrcLine.find_last_not_of(" ")+1); //remove " " in the end of the line
        if(tempSrcLine != "" && tempQuery != ""){
            tempQuery.append(" ");
        }
        tempQuery.append(tempSrcLine);
        if(tempQuery.find("DELIMITER") != std::string::npos){
            delimiter = tempQuery.substr(tempQuery.find("DELIMITER") + std::string("DELIMITER").length(), tempQuery.find_last_not_of(' '));
            delimiter = delimiter.substr(delimiter.find_first_not_of(' '));
            tempQuery = "";
        }
        if(tempQuery.rfind(delimiter) != std::string::npos && !(tempQuery.empty())){
            tempQuery = tempQuery.substr(0, tempQuery.rfind(delimiter));
            dest.push_back(tempQuery);
            tempQuery = "";
        }
    }
}
void SqlClient::printSqlError(int ErrCode, const std::string& query)
{
    switch(ErrCode){
    case 0:{
        return;
    }break;
    case 2014:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "Commands were executed in an improper order";
    }break;
    case 2006:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "MySQL server has gone away";
    }break;
    case 2013:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "The connection to the server was lost during the query";
    }break;
    case 2000:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "An unknown error occurred";
    }break;
    default:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "IDontKnowErr?!?";
    }
    }
    std::cout << "code was:" << ErrCode << "   " << "query was:" << std::endl << query << std::endl;
}
bool SqlClient::initDB()
{
    bool rtn = true;
    rtn &= executeScript("dropDatabase.sql");
    rtn &= executeScript("createDatabase.sql");
    rtn &= executeScript("createTables.sql");
    rtn &= executeScript("createProcedures.sql");
    rtn &= executeScript("createSampleData.sql");
    return rtn;
}

bool SqlClient::getAllRowsOfTable(const std::string& tableName, rj::Document& dom_o)
{
    std::string query = "Select * from ";
    query.append(tableName);
    MYSQL_RES* result = sendCommand(query);
    if(result != nullptr){
        bool res = mysqlResToDom(result, 0, dom_o);
        mysql_free_result(result);
        return res;
    }else{
        return false;
    }
}
bool SqlClient::createInstanceOfGuiElement(const std::string& type, uint64_t pageSqlID, const std::string& name)
{
    std::string query = "CALL CreateInstanceOfGuiElement('";
    query.append(type);
    query.append("', ");
    query.append(std::to_string(pageSqlID));
    query.append(", '");
    query.append(name);
    query.append("')");
    return sendCUD(query);
}
MYSQL_RES* SqlClient::getDataNodes(uint64_t guiElementID)
{
    std::string query = "CALL getDataNodesByGuiElementID(";
    query.append(std::to_string(guiElementID));
    query.append(")");
    return sendCommand(query);
}
MYSQL_RES* SqlClient::getGuiElements(uint64_t pageID)
{
    std::string query = "CALL getGuiElementsByPageID(";
    query.append(std::to_string(pageID));
    query.append(")");
    return sendCommand(query);
}
MYSQL_RES* SqlClient::getPages(uint64_t pageID)
{
    std::string query = "CALL getPagesByParentPageID(";
    query.append(pageID == 0 ? "NULL" : std::to_string(pageID));
    query.append(")");
    return sendCommand(query);
}
bool SqlClient::itterateThroughMYSQL_RES(MYSQL_RES* resultSet, std::function<void(const MYSQL_ROW&)> callback, bool freeResultSetAfter)
{
    if(resultSet != nullptr){
        while (MYSQL_ROW row = mysql_fetch_row(resultSet)) {
            callback(row);
        }
        if(freeResultSetAfter){
            mysql_free_result(resultSet);
        }
        return true;
    }else{
        return false;
    }
}
void SqlClient::escapeString( std::string& str )
{
    char result[100];
    {
        mutex_m.lock();
        std::lock_guard<std::mutex> lg(mutex_m, std::adopt_lock);
        mysql_real_escape_string(mysqlhandle_m, result, str.c_str(), str.size());
    }
    str.clear();
    str.append(result);
}
