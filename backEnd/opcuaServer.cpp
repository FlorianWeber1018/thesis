#include "globalInclude.hpp"
#include "opcuaServer.hpp"
OpcuaServer::OpcuaServer()
{
    start();
    changeRequestWorkerClock_m = new util::Clock(std::chrono::milliseconds(changeRequestWorkerTimebase),
                                               std::bind(&OpcuaServer::ChangeRequestWorker, this));
    changeRequestWorkerClock_m->start();
}
OpcuaServer::~OpcuaServer()
{
    if(changeRequestWorkerClock_m != nullptr){
        changeRequestWorkerClock_m->stop();
        delete changeRequestWorkerClock_m;
    }
}
void OpcuaServer::flushChangeRequest(const ChangeRequest& changeRequest)
{
    changeRequestMutex_m.lock();
    std::lock_guard<std::mutex> lg(changeRequestMutex_m, std::adopt_lock);

    if(changeRequests_m.size() >= changeRequestsMaxCnt){
        util::ConsoleOut() << "changeRequestsMaxCnt reached, "
                              "colsult to increase Constant changeRequestsMaxCnt, "
                              "or use a more performant backend Server."
                              "The Change Request with the Node ID: "
                                << changeRequest.nodeID.identifier.string.data
                           << "to a new Value was not performed";
        return;
    }else{
        changeRequests_m.push_back(changeRequest);
    }
}
bool OpcuaServer::start()
{
    if(!running_m && server_m == nullptr){
        running_m = true;
        std::thread newThread(&OpcuaServer::ServerFkt, this);
        newThread.detach();
        return true;
    }else{
        return false;
    }
}
bool OpcuaServer::stop()
{
    if(running_m){
        running_m = false;
        while(server_m != nullptr){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        util::ConsoleOut() << "OpcuaServer::stop()";
        return true;
    }else{
        return false;
    }
}
bool OpcuaServer::getState()
{
    return running_m;
}
UA_StatusCode OpcuaServer::ServerFkt()
{
    if(serverFktMutex_m.try_lock()){
        std::lock_guard<std::mutex> lg(serverFktMutex_m, std::adopt_lock);

        UA_ServerConfig* config = UA_ServerConfig_new_default();
        server_m = UA_Server_new(config);
        UA_StatusCode retVal = UA_Server_run(server_m, &running_m);

        UA_Server_delete(server_m);
        UA_ServerConfig_delete(config);
        config = nullptr;
        server_m = nullptr;

        return retVal == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
    }else{
        return EXIT_FAILURE;
    }
}
void OpcuaServer::ChangeRequestWorker()
{
    if(server_m != nullptr && running_m){
        changeRequestMutex_m.lock();
        std::lock_guard<std::mutex> lg(changeRequestMutex_m, std::adopt_lock);
        while(changeRequests_m.size() > 0){
            ChangeRequest tempChangeRequest = changeRequests_m.front();
            performChangeRequest(tempChangeRequest);
            changeRequests_m.pop_front();
        }
    }
}
void OpcuaServer::performChangeRequest(const ChangeRequest& changeRequest)
{
//TO Implement
}



void OpcuaServer::createVariable(const UA_VariableAttributes& attributes,
                                 const UA_NodeId& newNodeID,
                                 const UA_NodeId& parentNodeID)
{
    UA_QualifiedName qualifiedName;
    qualifiedName.namespaceIndex = 1;
    qualifiedName.name = attributes.displayName.text;
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server_m, newNodeID, parentNodeID, parentReferenceNodeId, qualifiedName, UA_NODEID_NUMERIC(0,UA_NS0ID_BASEDATAVARIABLETYPE), attributes, nullptr, nullptr);
}
void OpcuaServer::createObject(const UA_ObjectAttributes& attributes,
                  const UA_NodeId& newNodeID,
                  const UA_NodeId &parentNodeID)
{
    UA_QualifiedName qualifiedName;
    qualifiedName.namespaceIndex = 1;
    qualifiedName.name = attributes.displayName.text;
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addObjectNode(server_m, newNodeID, parentNodeID, parentReferenceNodeId, qualifiedName, UA_NODEID_NUMERIC(0,UA_NS0ID_BASEOBJECTTYPE), attributes, nullptr, nullptr);
}
void OpcuaServer::generateNodeID(std::string& prefix, uint64_t sqlID)
{
    prefix.append(std::to_string(sqlID));
}
UA_NodeId OpcuaServer::generateNodeID(const IdType& type, uint64_t sqlID)
{
    std::string prefix;
    switch(type){
        case IdType_DataNode:{
            prefix = "DN_";
        }break;
        case IdType_GuiElement:{
            prefix = "GE_";
        }break;
        case IdType_GuiELementTypedef:{
            prefix = "GEType_";
        }break;
        case IdType_Page:{
            prefix = "P_";
        }break;
        default:{
            prefix = "";
        }
    }
    generateNodeID(prefix, sqlID);
    return UA_NODEID_STRING_ALLOC(1, prefix.c_str());
}
void OpcuaServer::createDataNode(const std::string& type,
                                 const std::string& initValue,
                                 const std::string& description,
                                 const std::string& name,
                                 uint64_t parentguiElementSqlID,
                                 uint64_t newDataNodeSqlID,
                                 bool writePermission)
{
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    uint8_t typeFromMapping;
    try{
        typeFromMapping = this->basicTypeMapping.at(type);
    }catch(std::out_of_range e){
        typeFromMapping = -1;
    }
    switch(typeFromMapping){
        case UA_TYPES_BOOLEAN:{
            UA_Boolean convertedInitValue = (initValue == "1") ? true : false;
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_SBYTE:{
            int64_t intVal =  std::stoi(initValue);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_SBYTE_MIN), static_cast<int64_t>(UA_SBYTE_MAX));
            UA_SByte convertedInitValue = static_cast<UA_SByte>(intVal);
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_BYTE:{
            int64_t intVal =  std::stoi(initValue);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_BYTE_MIN), static_cast<int64_t>(UA_BYTE_MAX));
            UA_Byte convertedInitValue = static_cast<UA_Byte>(intVal);
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_INT16:{
            int64_t intVal =  std::stoll(initValue);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_INT16_MIN), static_cast<int64_t>(UA_INT16_MAX));
            UA_Int16 convertedInitValue = static_cast<UA_Int16>(intVal);
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_UINT16:{
            int64_t intVal =  std::stoll(initValue);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_UINT16_MIN), static_cast<int64_t>(UA_UINT16_MAX));
            UA_UInt16 convertedInitValue = static_cast<UA_UInt16>(intVal);
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_INT32:{
            int64_t intVal =  std::stoll(initValue);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_INT32_MIN), static_cast<int64_t>(UA_INT32_MAX));
            UA_Int32 convertedInitValue = static_cast<UA_Int32>(intVal);
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_UINT32:{
            int64_t intVal =  std::stoll(initValue);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_UINT32_MIN), static_cast<int64_t>(UA_UINT32_MAX));
            UA_UInt32 convertedInitValue = static_cast<UA_UInt32>(intVal);
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_INT64:{
            int64_t intVal =  std::stoll(initValue);
            util::moveToBorders(intVal, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
            UA_Int64 convertedInitValue = static_cast<UA_Int64>(intVal);
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_UINT64:{
            uint64_t intVal =  std::stoull(initValue);
            util::moveToBorders(intVal, std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max());
            UA_UInt64 convertedInitValue = static_cast<UA_UInt64>(intVal);
            UA_Variant_setScalarCopy(&attr.value, &convertedInitValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_FLOAT:{
            UA_Float convertedValue = std::stof(initValue);
            UA_Variant_setScalarCopy(&attr.value, &convertedValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_DOUBLE:{
            UA_Double convertedValue = std::stod(initValue);
            UA_Variant_setScalarCopy(&attr.value, &convertedValue, &UA_TYPES[typeFromMapping]);
        }break;
        case UA_TYPES_STRING:{
            UA_String convertedValue = UA_STRING_ALLOC(initValue.c_str());
            UA_Variant_setScalarCopy(&attr.value, &convertedValue, &UA_TYPES[typeFromMapping]);
        }break;
        default:{
            util::ConsoleOut() << "Exception by creating the Datanode with the ID: " << newDataNodeSqlID << "GuiElementID was: " << parentguiElementSqlID << "and type was: " << type << "This type cant be resolved of the internal Type Mapping.";
            return;
        }
    }

    attr.dataType = UA_TYPES[typeFromMapping].typeId;
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", description.c_str());
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", name.c_str());
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    if(writePermission){
        attr.accessLevel |= UA_ACCESSLEVELMASK_WRITE;
    }
    if(parentguiElementSqlID != 0){
        createVariable(attr, generateNodeID(IdType_DataNode, newDataNodeSqlID), generateNodeID(IdType_GuiElement, parentguiElementSqlID));
    }else{
        createVariable(attr, generateNodeID(IdType_DataNode, newDataNodeSqlID));
    }
}
void OpcuaServer::createGuiElementNode(const std::string& name,
                                       const std::string& type,
                                       const std::string& description,
                                       uint64_t parentPageSqlID,
                                       uint64_t newGuiELementSqlID)
{
    UA_ObjectAttributes attr = UA_ObjectAttributes_default;
    std::string displayName = name;
    displayName.append("_");
    displayName.append(type);
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", displayName.c_str());
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", description.c_str());
    if(parentPageSqlID != 0){
        createObject(attr, generateNodeID(IdType_GuiElement, newGuiELementSqlID),generateNodeID(IdType_Page, parentPageSqlID));
    }else{
        createObject(attr, generateNodeID(IdType_GuiElement, newGuiELementSqlID));
    }

}
void OpcuaServer::createPageNode(const std::string& title,
                    const std::string& description,
                    uint64_t parentPageSqlID,
                    uint64_t newPageSqlID)
{
    UA_ObjectAttributes attr = UA_ObjectAttributes_default;
    std::string displayName = title;
    displayName.append("_Page_");
    displayName.append(std::to_string(newPageSqlID));
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", displayName.c_str());
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", description.c_str());
    if(parentPageSqlID != 0){
        createObject(attr, generateNodeID(IdType_Page, newPageSqlID),generateNodeID(IdType_Page, parentPageSqlID));
    }else{
        createObject(attr, generateNodeID(IdType_Page, newPageSqlID));
    }
}
