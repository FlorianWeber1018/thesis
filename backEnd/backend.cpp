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
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInt = 42;
    UA_Variant_setScalar(&attr.value, &myInt, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT("en-US", "the super funky description");
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "the super funky displayName");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_VariableAttributes attr2 = UA_VariableAttributes_default;
    UA_Int32 myInt2 = 42;
    UA_Variant_setScalar(&attr2.value, &myInt2, &UA_TYPES[UA_TYPES_INT32]);
    attr2.description = UA_LOCALIZEDTEXT("en-US", "the super funky description42");
    attr2.displayName = UA_LOCALIZEDTEXT("en-US", "the super funky displayName2");
    attr2.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr2.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    createVariable(attr, generateNodeID(IdType_DataNode, 42));
    createVariable(attr2, generateNodeID(IdType_DataNode, 50));
}
