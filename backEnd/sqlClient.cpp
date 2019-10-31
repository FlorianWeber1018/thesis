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


sql_message::sql_message(sqlEvent event, const std::string& singlePayload)
{
    event_m = event;
    payload_m.emplace_back(singlePayload);
}
sql_message::sql_message(sqlEvent event, const std::string& firstPayload, const std::string& secondPayload)
{
    event_m = event;
    payload_m.push_back(firstPayload);
    payload_m.push_back(secondPayload);
}
sql_message::sql_message(sqlEvent event, const std::vector<std::string>& payloadVec) : payload_m(payloadVec)
{
    event_m = event;
}
sql_message::sql_message(const sql_message& otherMsg) : payload_m(otherMsg.payload_m)
{
    event_m = otherMsg.event_m;
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
    connectAlreadyLocked();
}
void SqlClient::connectAlreadyLocked() noexcept
{

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
        util::ConsoleOut() << "SqlClient::connect() failed";
    }else{
        this->connected_m = true;
        if(sendCUDAlreadyLocked("USE WebVisu")){
            util::ConsoleOut() << "SqlClient::connect() succeed";
        }
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
MYSQL_RES* SqlClient::sendCommand(std::string& sendstring, int maxReconnectCnt)
{
    mutex_m.lock();
    std::lock_guard<std::mutex> lg(mutex_m, std::adopt_lock);
    return sendCommandAlreadyLocked(sendstring, maxReconnectCnt);
}
MYSQL_RES* SqlClient::sendCommandAlreadyLocked(std::string& sendstring, int maxReconnectCnt)
{
    if(connected_m){
        while(mysql_next_result(mysqlhandle_m) == 0){

        }
        int ErrCode = mysql_query(mysqlhandle_m, sendstring.c_str());
        if (ErrCode == 0)
        {
            return mysql_store_result(mysqlhandle_m);
        }else{
            printSqlError(ErrCode,sendstring);
            std::cout << mysql_error(this->mysqlhandle_m) << std::endl;
            if(maxReconnectCnt > 0){
                maxReconnectCnt--;
                util::ConsoleOut() << "SqlClient::sendCommand:: trying to reconnect ";
                //std::this_thread::sleep_for(std::chrono::seconds(1));
                connectAlreadyLocked();
                return sendCommandAlreadyLocked(sendstring, maxReconnectCnt);
            }else{
                util::ConsoleOut() << "SqlClient::sendCommand:: trying to reconnect max reconnectCnt reached -> giving up";
                return nullptr;
            }

        }

    }else{
        if(maxReconnectCnt > 0){
            maxReconnectCnt--;
            util::ConsoleOut() << "SqlClient::sendCommand:: trying to reconnect ";
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            connectAlreadyLocked();
            return sendCommandAlreadyLocked(sendstring, maxReconnectCnt);
        }
    }
    return nullptr;
}
bool SqlClient::sendCUD(const std::string& sendstring)
{
    mutex_m.lock();
    std::lock_guard<std::mutex> lg(mutex_m, std::adopt_lock);
    return sendCUDAlreadyLocked(sendstring);
}
bool SqlClient::sendCUDAlreadyLocked(const std::string& sendstring)
{
    if(connected_m){
        int ErrCode = mysql_query(mysqlhandle_m, sendstring.c_str());
        if(ErrCode){
            return false;
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
bool SqlClient::pageExists(std::string pageID)
{
    escapeString(pageID);
    return entryExists("Pages", "ID", pageID);
}
void SqlClient::getDataNodeIDs(std::set<std::string>& outDnIds, std::string pageID)
{
    escapeString(pageID);
    std::string query = "Call getDataNodesIDsByPageID(";
    query.append(pageID);
    query.append(")");
    MYSQL_RES* result = sendCommand(query);
    itterateThroughMYSQL_RES(result,[&](const MYSQL_ROW& row){ if(row[0]!=nullptr){ outDnIds.insert(std::string(row[0])); } });
}
void SqlClient::getParamNodeIDs(std::set<std::string>& outPnIds, std::string pageID)
{
    escapeString(pageID);
    std::string query = "Call getParamsIDsByPageID(";
    query.append(pageID);
    query.append(")");
    MYSQL_RES* result = sendCommand(query);
    itterateThroughMYSQL_RES(result,[&](const MYSQL_ROW& row){ if(row[0]!=nullptr){ outPnIds.insert(std::string(row[0])); } });
}
std::string SqlClient::getParamNodeValue(std::string paramNodeSqlID)
{
    std::string query = "select value from GuiElementParams where ID = ";
    query.append(paramNodeSqlID);
    MYSQL_RES* result = sendCommand(query);
    if(result != nullptr){
        if(MYSQL_ROW row = mysql_fetch_row(result)){
            if(row[0]!=nullptr){
                return std::string(row[0]);
            }
        }
    }
    return std::string();
}
bool SqlClient::updateParamNode(std::string paramNodeSqlID, std::string newParamValue)
{
    escapeString(paramNodeSqlID);
    escapeString(newParamValue);

    bool success = false;

    std::string query = "select value from GuiElementParams where ID = ";
    query.append(paramNodeSqlID);
    MYSQL_RES* result = sendCommand(query);
    if(result != nullptr){
        if(MYSQL_ROW row = mysql_fetch_row(result)){
            if(row[0]!=nullptr){
                std::string oldParamValue(row[0]);
                if(oldParamValue != newParamValue){
                    std::string updateQuery = "Update GuiElementParams set value = '";
                    updateQuery.append(newParamValue);
                    updateQuery.append("' where ID = ");
                    updateQuery.append(paramNodeSqlID);
                    success = sendCUD(updateQuery);
                    if(success){
                        sql_message msg(sqlEvent_paramNodeChange, paramNodeSqlID, newParamValue);
                        sql_dispatch(msg);
                    }
                }else{
                    success = true; //there was nothing to change
                }
            }
        }
        mysql_free_result(result);
    }
    return success;
}
void SqlClient::getStructureOfPage(std::string pageID, rj::Document& outDom)
{
    rj::Document::AllocatorType& allocator = outDom.GetAllocator();
    escapeString(pageID);
    outDom.SetObject();

    outDom.AddMember(rj::Value("pageID"), rj::Value(std::stoul(pageID)), allocator);

    std::string query = "select parentID, title, description from Pages where ID = ";
    query.append(pageID);
    bool success = itterateThroughMYSQL_RES(sendCommand(query),[&](const MYSQL_ROW& row){
        outDom.AddMember(rj::Value("parentId"), row[0] == nullptr ? rj::Value(rj::kNullType) : rj::Value(std::stoul(row[0])), allocator );
        outDom.AddMember(rj::Value("title"), row[1]==nullptr ? rj::Value(rj::kNullType) : rj::Value(row[1], allocator), allocator);
        outDom.AddMember(rj::Value("description"), row[2]==nullptr ? rj::Value(rj::kNullType) : rj::Value(row[2], allocator), allocator);
    });

    rj::Value subPages(rj::kArrayType);
    success = itterateThroughMYSQL_RES(getPages(pageID),[&](const MYSQL_ROW& row){
        if(row[3]!=nullptr){
            rj::Value page(rj::kObjectType);
            page.AddMember(rj::Value("id"), rj::Value(std::stoul(row[3])), allocator);
            page.AddMember(rj::Value("title"), row[0]==nullptr ? rj::Value(rj::kNullType) : rj::Value(row[0], allocator), allocator);
            page.AddMember(rj::Value("description"), row[1]==nullptr ? rj::Value(rj::kNullType) : rj::Value(row[1], allocator), allocator);
            subPages.PushBack(page, allocator);
        }
    });
    if(success){
        outDom.AddMember(rj::Value("subPages"), subPages, allocator);
    }

    rj::Value guiElements(rj::kObjectType);

    success = itterateThroughMYSQL_RES(getGuiElements(pageID), [&](const MYSQL_ROW& row){
        if(row[1] != nullptr && row[4] != nullptr){
            std::string guiElementIdStr(row[4]);
            rj::Value guiElement(rj::kObjectType);
            guiElement.AddMember(rj::Value("id"), rj::Value(std::stoul(row[4])), allocator);
            guiElement.AddMember(rj::Value("type"), rj::Value(row[1], allocator), allocator);
            guiElement.AddMember(rj::Value("name"), row[0]==nullptr ? rj::Value(rj::kNullType) : rj::Value(row[0], allocator), allocator);
            guiElement.AddMember(rj::Value("description"), row[2]==nullptr ? rj::Value(rj::kNullType) : rj::Value(row[2], allocator), allocator);

            rj::Value dataNodes(rj::kObjectType);
            itterateThroughMYSQL_RES(getDataNodes(guiElementIdStr), [&](const MYSQL_ROW& row){
                if(row[3] != nullptr && row[5] != nullptr && row[0] != nullptr && row[6] != nullptr){
                    rj::Value dataNode(rj::kObjectType);
                    dataNode.AddMember(rj::Value("id"), rj::Value(std::stoul(row[5])), allocator);
                    dataNode.AddMember(rj::Value("name"), rj::Value(row[3], allocator), allocator);
                    dataNode.AddMember(rj::Value("type"), rj::Value(row[0], allocator), allocator);
                    dataNode.AddMember(rj::Value("value"), rj::Value(rj::kNullType), allocator);
                    //dataNode.AddMember(rj::Value("writePermission"), rj::Value(std::string(row[6]) == "1" ? true : false), allocator );
                    dataNode.AddMember(rj::Value("description"), row[2]==nullptr ? rj::Value(rj::kNullType) : rj::Value(row[2], allocator), allocator);
                    dataNodes.AddMember(rj::Value(row[3], allocator), dataNode, allocator);
                }
            });
            guiElement.AddMember(rj::Value("dataNodes"), dataNodes, allocator);

            rj::Value paramNodes(rj::kObjectType);
            itterateThroughMYSQL_RES(getParams(guiElementIdStr), [&](const MYSQL_ROW& row){
                if(row[0] != nullptr && row[1] != nullptr && row[2] != nullptr){
                    rj::Value param(rj::kObjectType);
                    param.AddMember(rj::Value("id"), rj::Value(std::stoul(row[0])), allocator);
                    param.AddMember(rj::Value("name"), rj::Value(row[1], allocator), allocator);
                    param.AddMember(rj::Value("type"), rj::Value(row[2], allocator), allocator);
                    param.AddMember(rj::Value("value"), rj::Value(rj::kNullType), allocator);
                    param.AddMember(rj::Value("description"), row[4]==nullptr ? rj::Value(rj::kNullType) : rj::Value(row[4], allocator), allocator);
                    paramNodes.AddMember(rj::Value(row[1], allocator), param, allocator);
                }
            });
            guiElement.AddMember(rj::Value("paramNodes"), paramNodes, allocator);

            guiElements.AddMember(rj::Value(row[4], allocator), guiElement, allocator);
        }

    });
    if(success){
        outDom.AddMember(rj::Value("guiElements"), guiElements, allocator);
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
                        val.SetUint64(std::stoul(row[i]));
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
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "Commands were executed in an improper order ";
    }break;
    case 2006:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "MySQL server has gone away ";
    }break;
    case 2013:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "The connection to the server was lost during the query ";
    }break;
    case 2000:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "An unknown error occurred ";
    }break;
    default:{
        std::cout << "mysqlcon::sendCommand::printErr(ErrCode): " << "IDontKnowErr?!? ";
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
bool SqlClient::entryExists(const std::string& tableName, const std::string& keyColName, const std::string& keyVal)
{
    std::string query = "Select count(*) from ";
    query.append(tableName);
    query.append(" where ");
    query.append(keyColName);
    query.append(" = ");
    query.append(keyVal);
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
MYSQL_RES* SqlClient::getParams(uint64_t guiElementID)
{
    return getParams(std::to_string(guiElementID));
}
MYSQL_RES* SqlClient::getDataNodes(uint64_t guiElementID)
{
    return getDataNodes(std::to_string(guiElementID));
}
MYSQL_RES* SqlClient::getGuiElements(uint64_t pageID)
{
    return getGuiElements(std::to_string(pageID));
}
MYSQL_RES* SqlClient::getPages(uint64_t pageID)
{
    return getPages(std::to_string(pageID));
}
MYSQL_RES* SqlClient::getParams(const std::string& guiElementID)
{
    std::string query = "CALL getParamsByGuiElementID(";
    query.append(guiElementID);
    query.append(")");
    return sendCommand(query);
}
MYSQL_RES* SqlClient::getDataNodes(const std::string& guiElementID)
{
    std::string query = "CALL getDataNodesByGuiElementID(";
    query.append(guiElementID);
    query.append(")");
    return sendCommand(query);
}
MYSQL_RES* SqlClient::getGuiElements(const std::string& pageID)
{
    std::string query = "CALL getGuiElementsByPageID(";
    query.append(pageID);
    query.append(")");
    return sendCommand(query);
}
MYSQL_RES* SqlClient::getPages(const std::string& pageID)
{
    std::string query = "CALL getPagesByParentPageID(";
    query.append(pageID == "0" ? "NULL" : pageID);
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
    if(str.length() < 100) //remember: nullterminated ( result only can hold 99 chars)
    {
        mutex_m.lock();
        std::lock_guard<std::mutex> lg(mutex_m, std::adopt_lock);
        mysql_real_escape_string(mysqlhandle_m, result, str.c_str(), str.size());
    } else{
        str.clear();
        return;
    }
    str.clear();
    str.append(result);
}
