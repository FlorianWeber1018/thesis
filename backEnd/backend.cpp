#include "globalInclude.hpp"
#include "backend.hpp"
#include "util.hpp"
#include "rapidjson/document.h"
Backend::Backend(){
    if(initDB()){
        util::ConsoleOut() << "jo" ;
    }
    rj::Document myDom;
    if(getAllRowsOfTable("Pages",myDom)){
        std::string teststring = util::Json().toJson(myDom);
        rj::Document newDom;
        if(util::Json::toDom(teststring, newDom)){
            std::string teststring2 = util::Json().toJson(newDom);
            util::ConsoleOut() << teststring2;
        }
        util::ConsoleOut() << teststring;
    }
}
