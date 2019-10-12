#include "globalInclude.hpp"
#include "opcuaServer.hpp"

NodeContext::NodeContext(const std::string& initOldValue, OpcuaServer* opcuaServer) : oldValue (initOldValue)
{
    opcuaServer_m = opcuaServer;
}

OpcuaServer::OpcuaServer()
{
    for(auto&& element : basicTypeMapping){
        basicTypeMappingReverse[element.second] = element.first;
    }

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
void OpcuaServer::flushChangeRequest(const std::string& newValue, const std::string& type, uint64_t dataNodeSqlID)
{
    int8_t parsedType = -1;
    if(parseType(parsedType,type)){
        flushChangeRequest(newValue, parsedType, dataNodeSqlID);
    }
}
void OpcuaServer::flushChangeRequest(const std::string& newValue, uint64_t dataNodeSqlID)
{
    int8_t type = -1;
    //determine Type
    UA_NodeId nodeID;
    UA_Variant oldValue;
    generateNodeID(nodeID, IdType_DataNode, dataNodeSqlID);
    UA_Server_readValue(server_m, nodeID, &oldValue);
    if(oldValue.type != nullptr && oldValue.type->typeIndex <= std::numeric_limits<int8_t>::max()){
        type = static_cast<int8_t>(oldValue.type->typeIndex);
        flushChangeRequest(newValue, type, dataNodeSqlID);
    }

}
void OpcuaServer::flushChangeRequest(const std::string& newValue, const std::string& dataNodeSqlID)
{
    flushChangeRequest(newValue, std::stoull(dataNodeSqlID));
}
void OpcuaServer::flushChangeRequest(const std::string& newValue, int8_t type, uint64_t dataNodeSqlID)
{
    ChangeRequest newChangeRequest;
    if(parseValue(newChangeRequest.newValue, newValue, type)){
        generateNodeID(newChangeRequest.nodeID, IdType_DataNode, dataNodeSqlID);
        flushChangeRequest(newChangeRequest);
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
        config->nodeLifecycle.destructor = OpcuaServer::customNodeDestructor;
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
    UA_Server_writeValue(server_m, changeRequest.nodeID, changeRequest.newValue);
}
void OpcuaServer::NodeIdToSqlId(std::string& str)
{
    size_t pos = str.find_last_of("_") + 1;
    if( !(pos > str.npos) ){
       str = str.substr(str.find_last_of("_")+1);
    }

}
bool OpcuaServer::parseValue(UA_Variant& outVariant, const std::string& valueString, int8_t type)
{
    switch(type){
        case UA_TYPES_BOOLEAN:{
            UA_Boolean convertedInitValue = (valueString == "1" || valueString == "true") ? true : false;
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_SBYTE:{
            int64_t intVal =  std::stoi(valueString);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_SBYTE_MIN), static_cast<int64_t>(UA_SBYTE_MAX));
            UA_SByte convertedInitValue = static_cast<UA_SByte>(intVal);
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_BYTE:{
            int64_t intVal =  std::stoi(valueString);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_BYTE_MIN), static_cast<int64_t>(UA_BYTE_MAX));
            UA_Byte convertedInitValue = static_cast<UA_Byte>(intVal);
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_INT16:{
            int64_t intVal =  std::stoll(valueString);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_INT16_MIN), static_cast<int64_t>(UA_INT16_MAX));
            UA_Int16 convertedInitValue = static_cast<UA_Int16>(intVal);
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_UINT16:{
            int64_t intVal =  std::stoll(valueString);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_UINT16_MIN), static_cast<int64_t>(UA_UINT16_MAX));
            UA_UInt16 convertedInitValue = static_cast<UA_UInt16>(intVal);
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_INT32:{
            int64_t intVal =  std::stoll(valueString);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_INT32_MIN), static_cast<int64_t>(UA_INT32_MAX));
            UA_Int32 convertedInitValue = static_cast<UA_Int32>(intVal);
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_UINT32:{
            int64_t intVal =  std::stoll(valueString);
            util::moveToBorders(intVal, static_cast<int64_t>(UA_UINT32_MIN), static_cast<int64_t>(UA_UINT32_MAX));
            UA_UInt32 convertedInitValue = static_cast<UA_UInt32>(intVal);
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_INT64:{
            int64_t intVal =  std::stoll(valueString);
            util::moveToBorders(intVal, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
            UA_Int64 convertedInitValue = static_cast<UA_Int64>(intVal);
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_UINT64:{
            uint64_t intVal =  std::stoull(valueString);
            util::moveToBorders(intVal, std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max());
            UA_UInt64 convertedInitValue = static_cast<UA_UInt64>(intVal);
            UA_Variant_setScalarCopy(&outVariant, &convertedInitValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_FLOAT:{
            UA_Float convertedValue = std::stof(valueString);
            UA_Variant_setScalarCopy(&outVariant, &convertedValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_DOUBLE:{
            UA_Double convertedValue = std::stod(valueString);
            UA_Variant_setScalarCopy(&outVariant, &convertedValue, &UA_TYPES[type]);
        }break;
        case UA_TYPES_STRING:{
            UA_String convertedValue = UA_STRING_ALLOC(valueString.c_str());
            UA_Variant_setScalarCopy(&outVariant, &convertedValue, &UA_TYPES[type]);
        }break;
        default:{
            util::ConsoleOut() << "Exception by parsing a Value with the UA_Type: " << type << " ValueString was:" << valueString;
            return false;
        }
    }
    return true;
}
bool OpcuaServer::parseValue(UA_Variant& outVariant, const std::string& valueString, const std::string& typeString)
{
    int8_t type;
    return parseType(type, typeString) == true ? parseValue(outVariant, valueString, type) : false;
}
std::string OpcuaServer::plotValue(const UA_Variant& variant, int8_t type){
    switch(type){
        case UA_TYPES_BOOLEAN:{
            UA_Boolean* castedValue = static_cast<UA_Boolean*>(variant.data);
            if(castedValue != nullptr){
                return *castedValue ? "1" : "0";
            }else{
                return "";
            }
        }break;

        case UA_TYPES_SBYTE:{
            UA_SByte* castedValue = static_cast<UA_SByte*>(variant.data);

            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_BYTE:{
            UA_Byte* castedValue = static_cast<UA_Byte*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_INT16:{
            UA_Int16* castedValue = static_cast<UA_Int16*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_UINT16:{
            UA_UInt16* castedValue = static_cast<UA_UInt16*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_INT32:{
            UA_Int32* castedValue = static_cast<UA_Int32*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_UINT32:{
            UA_UInt32* castedValue = static_cast<UA_UInt32*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_INT64:{
            UA_Int64* castedValue = static_cast<UA_Int64*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_UINT64:{
            UA_UInt64* castedValue = static_cast<UA_UInt64*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_FLOAT:{
            UA_Float* castedValue = static_cast<UA_Float*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_DOUBLE:{
            UA_Double* castedValue = static_cast<UA_Double*>(variant.data);
            return castedValue != nullptr ? std::to_string(*castedValue) : "";
        }break;
        case UA_TYPES_STRING:{
            UA_String* castedValue = static_cast<UA_String*>(variant.data);
            return castedValue != nullptr ? to_string(*castedValue) : "";
        }break;
        default:{
            util::ConsoleOut() << "Exception by ploting a Value of a Variant with the UA_TypeID: " << type << "the type ID cant be resolved";
            return "";
        }
    }
    return "";
}
bool OpcuaServer::parseType(int8_t& outType, const std::string& typeString)
{
    try{
        outType = this->basicTypeMapping.at(typeString);
    }catch(std::out_of_range e){
        outType = -1;
        return false;
    }
    return true;
}
std::string OpcuaServer::plotType (int8_t type)
{
    std::string rtn;
    try{
        rtn = this->basicTypeMappingReverse.at(type);
    }catch(std::out_of_range e){
        rtn = "";
    }
    return rtn;
}
std::string OpcuaServer::to_string(const UA_String& uaString)
{

    return std::string(reinterpret_cast<const char*>(uaString.data), uaString.length);
}
void OpcuaServer::createVariable(const UA_VariableAttributes& attributes,
                                 const UA_NodeId& newNodeID,
                                 const UA_NodeId& parentNodeID)
{
    NodeContext* nodeContext = new NodeContext(plotValue( attributes.value, attributes.value.type->typeIndex), this);
    UA_QualifiedName qualifiedName;
    qualifiedName.namespaceIndex = 1;
    qualifiedName.name = attributes.displayName.text;
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server_m, newNodeID, parentNodeID, parentReferenceNodeId, qualifiedName, UA_NODEID_NUMERIC(0,UA_NS0ID_BASEDATAVARIABLETYPE), attributes, nodeContext , nullptr);
    UA_ValueCallback callback;
    callback.onRead = nullptr;
    callback.onWrite = OpcuaServer::staticDataChangeDispatcher;
    UA_Server_setVariableNode_valueCallback(server_m, newNodeID, callback);
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
    UA_NodeId newNodeID;
    generateNodeID(newNodeID, type, sqlID);
    return newNodeID;
}
void OpcuaServer::generateNodeID(UA_NodeId& outNodeID, const IdType& type, uint64_t sqlID){
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
    outNodeID = UA_NODEID_STRING_ALLOC(1, prefix.c_str());
}
void OpcuaServer::createDataNode(const std::string& typeStr,
                                 const std::string& initValue,
                                 const std::string& description,
                                 const std::string& name,
                                 uint64_t parentguiElementSqlID,
                                 uint64_t newDataNodeSqlID,
                                 bool writePermission)
{
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    int8_t type;
    if(parseType(type, typeStr) && parseValue(attr.value, initValue, type)){
        attr.dataType = UA_TYPES[type].typeId;
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
void OpcuaServer::createDataNode(const MYSQL_ROW& dataNodeRow)
{
    std::string type = dataNodeRow[0];
    std::string initValue = dataNodeRow[1];
    std::string description = dataNodeRow[2];
    std::string name = dataNodeRow[3];
    uint64_t parentguiElementSqlID = std::stoull(dataNodeRow[4]);
    uint64_t newDataNodeSqlID = std::stoull(dataNodeRow[5]);
    std::string writePermissionString = dataNodeRow[6];
    bool writePermission = writePermissionString == "1" ? true : false;
    createDataNode(type,initValue,description, name, parentguiElementSqlID, newDataNodeSqlID, writePermission);
}
void OpcuaServer::createGuiElementNode(const MYSQL_ROW& guiElementNodeRow)
{
    std::string name = guiElementNodeRow[0];
    std::string type = guiElementNodeRow[1];
    std::string description = guiElementNodeRow[2];
    uint64_t parentPageSqlID = std::stoull(guiElementNodeRow[3]);
    uint64_t newGuiELementSqlID = std::stoull(guiElementNodeRow[4]);
    createGuiElementNode(name, type, description, parentPageSqlID, newGuiELementSqlID);
}
void OpcuaServer::createPageNode(const MYSQL_ROW& pageNodeRow)
{
    std::string title = pageNodeRow[0];
    std::string description = pageNodeRow[1];
    uint64_t parentPageSqlID = pageNodeRow[2] == nullptr ? 0 : std::stoull(pageNodeRow[2]);
    uint64_t newPageSqlID = std::stoull(pageNodeRow[3]);
    createPageNode(title, description, parentPageSqlID, newPageSqlID);
}
void OpcuaServer::removeNode(const IdType& type, uint64_t sqlID)
{
    UA_Server_deleteNode(server_m, generateNodeID(type, sqlID), true);
}
std::string OpcuaServer::readDataNode(const std::string& sqlID)
{
    return readDataNode(std::stoul(sqlID));
}

std::string OpcuaServer::readDataNode(uint64_t sqlID)
{
    UA_ReadValueId rvi;
    UA_ReadValueId_init(&rvi);
    generateNodeID(rvi.nodeId, IdType_DataNode, sqlID);
    rvi.attributeId = UA_ATTRIBUTEID_VALUE;
    UA_DataValue resp = UA_Server_read(server_m, &rvi, UA_TIMESTAMPSTORETURN_NEITHER);
    if(resp.hasValue){
        return plotValue(resp.value, resp.value.type->typeIndex);
    }
    return std::string();
}
void OpcuaServer::dataChangeDispatcher(const ChangeRequest& changeRequest)
{
    util::ConsoleOut() << "______________________________________________________________________________"
                       << "this is the virtual OpcuaServer::dataChangeDispatcher which is the Interface for the Backend class and should be reimplemented by the Backend Class to dispatch the datachange events on the OPCUA Server and transmit them to the Websocket Server."
                       << "ID Of DataNode with changed Data:" << changeRequest.nodeID.identifier.string.data << "type is: " << this->plotType(changeRequest.newValue.type->typeIndex) << "new Value is: " << plotValue(changeRequest.newValue, changeRequest.newValue.type->typeIndex)
                       << "______________________________________________________________________________";
}
void OpcuaServer::staticDataChangeDispatcher(UA_Server *server,
                       const UA_NodeId *sessionId, void *sessionContext,
                       const UA_NodeId *nodeId, void *nodeContext,
                       const UA_NumericRange *range, const UA_DataValue *data)
{
    if(data != nullptr && data->hasValue && data->status == UA_STATUSCODE_GOOD && nodeId != nullptr && nodeContext != nullptr){
        NodeContext* nodeContext_ = static_cast<NodeContext*>(nodeContext);
        if(nodeContext_->opcuaServer_m != nullptr){
            std::string newValueStr(nodeContext_->opcuaServer_m->plotValue(data->value, data->value.type->typeIndex));
            if(nodeContext_->oldValue != newValueStr){                      //just pass diffs not all writes!
                nodeContext_->oldValue = newValueStr;
                ChangeRequest newChangeRequest;
                UA_Variant_copy(&data->value, &newChangeRequest.newValue);
                UA_NodeId_copy(nodeId, &newChangeRequest.nodeID);
                nodeContext_->opcuaServer_m->dataChangeDispatcher(newChangeRequest);
                UA_NodeId_deleteMembers(&newChangeRequest.nodeID);
                UA_Variant_deleteMembers(&newChangeRequest.newValue);
            }

            return;
        }
    }
    util::ConsoleOut() << "native OpcuaServer::staticDataChangeDispatcher(UA_Server *, const UA_NodeId*, ...) failed";
}
void OpcuaServer::customNodeDestructor(UA_Server *server,
                               const UA_NodeId *sessionId, void *sessionContext,
                               const UA_NodeId *nodeId, void *nodeContext)
{
    if(nodeContext!=nullptr){
        NodeContext* nodeContext_ = static_cast<NodeContext*>(nodeContext);
        delete nodeContext_;
    }
}
