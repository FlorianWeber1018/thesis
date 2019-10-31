#ifndef opcuaServer__hpp
#define opcuaServer__hpp
#define UA_NO_AMALGAMATION
#include "open62541.h"
#include "globalInclude.hpp"
class OpcuaServer; //fwd declaration
class NodeContext : public std::enable_shared_from_this<NodeContext>
{
public:
    NodeContext(const std::string& initOldValue, OpcuaServer* opcuaServer);
    std::string oldValue;
    OpcuaServer* opcuaServer_m;
};

struct opcua_changeRequest{
    UA_Variant newValue;
    UA_NodeId nodeID;
};
enum IdType
{
        IdType_DataNode,
        IdType_GuiElement,
        IdType_Page,
        IdType_GuiELementTypedef
};
class OpcuaServer
{
public:

    OpcuaServer();
    ~OpcuaServer();



protected:

    virtual void opcua_dispatch(const opcua_changeRequest& changeRequest); //dispatcher to be overloaded in Backend to transmit events to the websocket class

    void flushChangeRequest(const std::string& newValue, const std::string& type, uint64_t dataNodeSqlID);//not blocking      //interface for websocket
    void flushChangeRequest(const std::string& newValue, uint64_t dataNodeSqlID);
    void flushChangeRequest(const std::string& newValue, const std::string& dataNodeSqlID);
    void createDataNode(const std::string& typeStr, const std::string& initValue, const std::string& description, const std::string& name, uint64_t parentguiElementSqlID, uint64_t newDataNodeSqlID, bool writePermission);
    void createGuiElementNode(const std::string& name, const std::string &type, const std::string& description, uint64_t parentPageSqlID, uint64_t newGuiELementSqlID);
    void createPageNode(const std::string& title, const std::string& description, uint64_t parentPageSqlID, uint64_t newPageSqlID);

    void createDataNode(const MYSQL_ROW& dataNodeRow);
    void createGuiElementNode(const MYSQL_ROW& guiElementNodeRow);
    void createPageNode(const MYSQL_ROW& pageNodeRow);

    void removeNode(const IdType& type, uint64_t sqlID);
    std::string readDataNode(uint64_t sqlID);
    std::string readDataNode(const std::string& sqlID);
    bool start();
    bool stop();
    bool getState();
    std::string plotValue(const UA_Variant& variant, int8_t type);
    void NodeIdToSqlId(std::string& str);


private:
    UA_Server* server_m = nullptr;
    util::Clock* changeRequestWorkerClock_m = nullptr;
    std::list<opcua_changeRequest> changeRequests_m;
    std::mutex changeRequestMutex_m;
    std::mutex serverFktMutex_m;
    volatile bool running_m = false;
    UA_StatusCode ServerFkt();
    void ChangeRequestWorker();
    void performChangeRequest(const opcua_changeRequest& changeRequest);
    bool parseValue(UA_Variant& outVariant, const std::string& valueString, int8_t type);
    bool parseValue(UA_Variant& outVariant, const std::string& valueString, const std::string& typeString);

    bool parseType(int8_t& outType, const std::string& typeString);
    std::string plotType (int8_t type);
    std::string to_string(const UA_String& uaString);

    void generateNodeID(std::string& prefix, uint64_t sqlID);
    UA_NodeId generateNodeID(const IdType& type, uint64_t sqlID);
    void generateNodeID(UA_NodeId& outNodeID, const IdType& type, uint64_t sqlID);
    void createVariable(const UA_VariableAttributes& attributes, const UA_NodeId& newNodeID, const UA_NodeId &parentNodeID = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    void createObject(const UA_ObjectAttributes& attributes, const UA_NodeId& newNodeID, const UA_NodeId &parentNodeID = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    void flushChangeRequest(const opcua_changeRequest& changeRequest);
    void flushChangeRequest(const std::string& newValue, int8_t type, uint64_t dataNodeSqlID);

    void dataChangeDispatcher(const UA_NodeId *nodeId, const UA_DataValue *data); //generic dispatcher for dataChange
    static void staticDataChangeDispatcher(UA_Server *server,
                           const UA_NodeId *sessionId, void *sessionContext,
                           const UA_NodeId *nodeId, void *nodeContext,
                           const UA_NumericRange *range, const UA_DataValue *data);
    const size_t changeRequestsMaxCnt = 100;        // if this Count is reached, the coresponding flush funktion
                                                    // will not add new change Requests and will not block (maby this isnt a good idea because the coresponding changerequest is lost if this occurs)
    const int64_t changeRequestWorkerTimebase = 20; // Time in  Milliseconds only used in Constructor

    std::map<std::string,int8_t> basicTypeMapping
    {
        {"Bool" , UA_TYPES_BOOLEAN},
        {"Int8" , UA_TYPES_SBYTE},
        {"UInt8" , UA_TYPES_BYTE},
        {"Int16" , UA_TYPES_INT16},
        {"UInt16" , UA_TYPES_UINT16},
        {"Int32" , UA_TYPES_INT32},
        {"UInt32" , UA_TYPES_UINT32},
        {"Int64" , UA_TYPES_INT64},
        {"UInt64" , UA_TYPES_UINT64},
        {"Float" , UA_TYPES_FLOAT},
        {"Double" , UA_TYPES_DOUBLE},
        {"String" , UA_TYPES_STRING}
    };
    std::map<int8_t,std::string> basicTypeMappingReverse; // will be set in runtime by the constructor
    static void customNodeDestructor(UA_Server *server,
                                   const UA_NodeId *sessionId, void *sessionContext,
                                   const UA_NodeId *nodeId, void *nodeContext);
};

#endif
