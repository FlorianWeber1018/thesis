#include "globalInclude.hpp"
#include "backend.hpp"
#include "util.hpp"
#include "rapidjson/document.h"
#include "httpServer.hpp"
#include <functional>
Backend::Backend(){
    if(initDB()){
        util::ConsoleOut() << "jo Datenbank wurde initialisiert" ;
    }else{
        util::ConsoleOut() << "da lief wohl was schief beim datenbank initialisieren" ;
        return;
    }


    traverseOpcuaPagesFromSql(0);

    //teststuff

    std::this_thread::sleep_for(std::chrono::seconds(3));
    flushChangeRequest("1", 1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    flushChangeRequest("42", 2);
    removeNode(IdType_DataNode, 4);

/*
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
    ws_message msg;
    msg.event = wsEvent_dataNodeChange;
    msg.payload.push_back(std::string(reinterpret_cast<const char*>(changeRequest.nodeID.identifier.string.data)));
    NodeIdToSqlId(msg.payload.front());
    msg.payload.push_back(plotValue(changeRequest.newValue, changeRequest.newValue.type->typeIndex));
    publishtoAllSessions(msg);
}
void Backend::ws_dispatch(const ws_message& msg, std::shared_ptr<ws_session> ws_session_)
{
    switch(msg.event){
    case wsEvent_dataNodeChange:{
        if(msg.payload.size() == 2){
            flushChangeRequest(msg.payload[1], msg.payload[0]);
        }
    }break;
    case wsEvent_paramNodeChange:{
        if(msg.payload.size() == 2){
            //
        }
        util::ConsoleOut() << "wsEvent_paramNodeChange";
    }break;
    case wsEvent_pageChange:{
        util::ConsoleOut() << "wsEvent_pageChange";
    }break;
    case wsEvent_authentification:{
        if(msg.payload.size() == 2){
            if(validateCredentials(msg.payload[0], msg.payload[1])){
                ws_session_->setAuthenticated();
                std::shared_ptr<ws_message> answer = std::make_shared<ws_message>();
                answer->event=wsEvent_authentification;
                ws_session_->send(answer);
            }
        }
        util::ConsoleOut() << "wsEvent_authentification";
    }break;
    case wsEvent_structure:{
        util::ConsoleOut() << "wsEvent_structure";
    }break;
    default:{
        return;
    }
    }
}

