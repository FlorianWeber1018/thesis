#include "globalInclude.hpp"
#include "sqlClient.hpp"
#include <boost/property_tree/ptree.hpp>
SqlCredentials::SqlCredentials(
        std::string userName = "WebVisuSU",
        std::string password = "637013",
        std::string host = "localhost",
        unsigned int port = 3306,
        std::string database = "WebVisu")
{
    this->userName = userName;
    this->password = password;
    this->host = host;
    this->port = port;
    this->database = database;

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
        throw 100;
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
                           credentials_m.database.c_str(),
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
MYSQL_RES* SqlClient::sendCommand(std::string sendstring)
{
    mutex_m.lock();
    std::lock_guard<std::mutex> lg(mutex_m, std::adopt_lock);
    if(connected_m){
        if (!mysql_query(mysqlhandle_m, sendstring.c_str()))
        {
            return mysql_store_result(mysqlhandle_m);
        }
        return NULL;
    }

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
pt::ptree SqlClient::MYSQL_RES_to_ptree(MYSQL_RES* resultset, unsigned int keyColNumber)
{
    pt::ptree resTree;
    if(resultset != nullptr){
        while (MYSQL_ROW row = mysql_fetch_row(resultset)) {
            pt::ptree rowTree;
            for(uint i = 0; i < resultset->field_count; i++){
                rowTree.put(resultset->fields[i].name, row[i]);
            }
            resTree.put_child(row[keyColNumber], rowTree);
        }
    }
    return resTree;
}
void SqlClient::prepareScript(const std::list<std::string>& src, std::list<std::string>& dest)
{
    std::string tempQuery = "";
    for(auto&& srcLine : src){
        std::string tempSrcLine = srcLine.substr(0, srcLine.find("-- ", 0));//create substring without comments
        tempSrcLine = tempSrcLine.substr(0, tempSrcLine.find("\r",0)); //remove \r and everything after(normaly there isnt anything after \r)
        tempSrcLine = tempSrcLine.substr(
                    tempSrcLine.find_first_not_of(" ") > tempSrcLine.length() ?
                        tempSrcLine.length() : tempSrcLine.find_first_not_of(" "),
                    tempSrcLine.length()); //remove " " in front of the line
        tempQuery.append(tempSrcLine);
        if(tempQuery.back() == ';'){
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
    return rtn;
}
