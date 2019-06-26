#include "globalInclude.hpp"
#include "backend.hpp"
#include "util.hpp"
#include "rapidjson/document.h"
Backend::Backend(){
    if(initDB()){
        util::ConsoleOut() << "jo Datenbank wurde initialisiert" ;
    }else{
        return;
    }

    rj::Document myDom;
    if(getAllRowsOfTable("GuiElementsDataNodesTemplate",myDom)){
        std::string teststring = util::Json().toJson(myDom);
        rj::Document newDom;
        if(util::Json::toDom(teststring, newDom)){
            std::string teststring2 = util::Json().toJson(newDom);
            util::ConsoleOut() << teststring2;
        }
        util::ConsoleOut() << teststring;
    }
    createPageNode("landingPage","root page where the user is landing",0,1);
    createPageNode("Page1","page1 description",1,2);
    createPageNode("Page2","page2 description",1,3);

    createGuiElementNode("enable", "button", "a simple button that can be pressed", 3, 1);
    createDataNode("Int32","42","funky description","buttonState",1,1,0);
    createDataNode("Int32","42","funky description","buttonState2",1,2,0);

}
void Backend::createOpcuaGuiElementsFromSql(int64_t guiElementID)
{

}
void Backend::traverseOpcuaPagesFromSql(int64_t startPageID)
{
    std::string query = "select PageID, ParentID from";
}
