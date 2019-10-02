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

    rj::Document myDom;
    if(getAllRowsOfTable("GuiElementDataNodeTemplates",myDom)){
        std::string teststring = util::Json().toJson(myDom);
        rj::Document newDom;
        if(util::Json::toDom(teststring, newDom)){
            std::string teststring2 = util::Json().toJson(newDom);
            util::ConsoleOut() << teststring2;
        }
        util::ConsoleOut() << teststring;
    }
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
