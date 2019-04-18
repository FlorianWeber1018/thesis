#include "globalInclude.hpp"
#include "sqlClient.hpp"
SqlCredentials::SqlCredentials(
        std::string userName = "backEnd",
        std::string password = "backendPW",
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
    this->SqlClient();
}
SqlClient::SqlClient()
{
    mysqlhandle_m = mysql_init(nullptr);
    if(mysqlhandle_m == nullptr){
        throw 100;
    }
    connect();
}
SqlClient::~mysqlcon()
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
        this->connected = true;//wird in disconnect wieder false
        this->disconnect();
    }else{
        this->connected = true;
    }
}
void SqlClient::disconnect() noexcept
{
    if(this->connected){
        mysql_close(mysqlhandle_m);
        this->connected = false;
    }
}
MYSQL_RES* SqlClient::sendCommand(std::string sendstring)
{

}
bool SqlClient::sendCUD(const std::string& sendstring)
{

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
