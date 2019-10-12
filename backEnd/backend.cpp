#include "globalInclude.hpp"
#include "backend.hpp"
#include "util.hpp"
#include "rapidjson/document.h"
#include "httpServer.hpp"
#include <functional>
#include <chrono>
Backend::Backend(){
    if(initDB()){
        util::ConsoleOut() << "jo Datenbank wurde initialisiert" ;
    }else{
        util::ConsoleOut() << "da lief wohl was schief beim datenbank initialisieren" ;
        return;
    }


    traverseOpcuaPagesFromSql(0);



    //teststuff
    /*rj::Document myDom;
    getStructureOfPage("1", myDom);
    util::ConsoleOut() << util::Json().toJson(myDom);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    flushChangeRequest("1", 1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    flushChangeRequest("42", 2);
    //removeNode(IdType_DataNode, 4);



    rj::Document myDom;
    if(getAllRowsOfTable("GuiElementDataNodeTemplates",myDom)){
        std::string teststring = util::Json().toJson(myDom);
        rj::Document newDom;
        if(util::Json::toDom(teststring, newDom)){
            std::string teststring2 = util::Json().toJson(newDom);
            util::ConsoleOut() << teststring2;
        }
        util::ConsoleOut() << teststring;
    }*/
}
void Backend::traverseOpcuaPagesFromSql(uint64_t startPageID)
{
    MYSQL_RES* recordSet;

    recordSet = getPages(startPageID);  //create all underlying pages
    std::list<uint64_t> pageList;
    itterateThroughMYSQL_RES(recordSet, [this, &pageList](const MYSQL_ROW& row){
        this->createPageNode(row);
        pageList.push_back(std::stoull(row[3]));
    });
    for(auto&& createdPageNodeID : pageList){
        traverseOpcuaPagesFromSql(createdPageNodeID);
    }
    std::list<uint64_t> guiElementList;
    recordSet = getGuiElements(startPageID);
    itterateThroughMYSQL_RES(recordSet, [this, &guiElementList](const MYSQL_ROW& row){
        this->createGuiElementNode(row);
        guiElementList.push_back(std::stoull(row[4]));
    });
    for(auto&& createdGuiElementID : guiElementList){
        MYSQL_RES* dataNodesRecordSet = getDataNodes(createdGuiElementID);
        itterateThroughMYSQL_RES(dataNodesRecordSet, [this](const MYSQL_ROW& row){
            this->createDataNode(row);
        });
    }

}
void Backend::dataChangeDispatcher(const ChangeRequest& changeRequest)
{
    ws_message msg(wsEvent_dataNodeChange);
    msg.payload.push_back(std::string(reinterpret_cast<const char*>(changeRequest.nodeID.identifier.string.data)));
    NodeIdToSqlId(msg.payload.front());//in OPCUA server (das es DN_ ist ist hier redundante info)
    msg.payload.push_back(plotValue(changeRequest.newValue, changeRequest.newValue.type->typeIndex));
    publishtoAllSessions(msg);
}
//dispatcher to dispatch messages emitted by the websocketServer Class
void Backend::ws_dispatch(const ws_message& msg, std::shared_ptr<ws_session> ws_session_)
{
    switch(msg.event){
    case wsEvent_dataNodeChange:{
        if(msg.payload.size() == 2){
            flushChangeRequest(msg.payload[1], msg.payload[0]);
        }
    }break;
    case wsEvent_paramNodeChange:{
        if(msg.payload.size() == 2 && util::isUnsignedLL(msg.payload[0])){
            updateParamNode(msg.payload[0], msg.payload[1]);
        }
    }break;
    case wsEvent_pageChange:{
        if(msg.payload.size() == 1 && util::isUnsignedLL(msg.payload[0]) && pageExists(msg.payload[0])){
            ws_session_->setPage(msg.payload[0]);
            std::shared_ptr<ws_message> answer = std::make_shared<ws_message>(msg);
            ws_session_->send(answer);
        }
    }break;
    case wsEvent_authentification:{
        if(msg.payload.size() == 2){
            if(validateCredentials(msg.payload[0], msg.payload[1])){
                ws_session_->setAuthenticated();
                std::shared_ptr<ws_message> answer = std::make_shared<ws_message>(wsEvent_authentification);
                ws_session_->send(answer);
            }
        }
    }break;
    case wsEvent_structure:{
        if(msg.payload.size() == 1 && util::isUnsignedLL(msg.payload[0])){
            rj::Document structureDOM;
            getStructureOfPage(msg.payload[0], structureDOM);
            std::shared_ptr<ws_message> answer = std::make_shared<ws_message>(wsEvent_structure, util::Json().toJson(structureDOM));
            ws_session_->send(answer);
            //hier wird das bdo aufgebaut und als JSON zurückgeschickt
        }
    }break;
    case wsEvent_reqSendDataNodes:{
        for(auto& element : msg.payload){
            std::shared_ptr<ws_message> answer = std::make_shared<ws_message>(wsEvent_dataNodeChange, element, readDataNode(element));
            ws_session_->send(answer);
        }
    }break;
    case wsEvent_reqSendParams:{
        auto start = std::chrono::steady_clock::now();
        for(auto& element : msg.payload){

            std::shared_ptr<ws_message> answer = std::make_shared<ws_message>(wsEvent_paramNodeChange, element, getParamNodeValue(element));
            ws_session_->send(answer);
            auto end = std::chrono::steady_clock::now();
            std::cout << "Elapsed time in nanoseconds : "
                    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
                    << " ns" << std::endl;
        }
    }break;
    default:{
        return;
    }
    }
}
//dispatcher to dispatch messages emitted by the SqlClient Class
void Backend::sql_dispatch(const sql_message& msg)
{
    switch(msg.event_m){
    case sqlEvent_pageChange:{

    }break;
    case sqlEvent_paramNodeChange:{
        if(msg.payload_m.size() == 2){
            publishtoAllSessions( ws_message (wsEvent_paramNodeChange, msg.payload_m) );
        }
    }break;
    default:{
        return;
    }
    }
}
void Backend::dispatchGetDataNodeIDs(std::shared_ptr<std::set<std::string> >  outDnIds, std::string pageID)
{
    getDataNodeIDs(outDnIds, pageID);
}
void Backend::dispatchGetParamNodeIDs(std::shared_ptr<std::set<std::string> > outPnIds, std::string pageID)
{
    getParamNodeIDs(outPnIds, pageID);
}
