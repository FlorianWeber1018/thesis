#ifndef opcuaServer__hpp
#define opcuaServer__hpp
#define UA_NO_AMALGAMATION
#include "open62541.h"
#include "globalInclude.hpp"

struct ChangeRequest{
    UA_Variant newValue;
    bool silent = false;
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
    void flushChangeRequest(const ChangeRequest& changeRequest); //not blocking

    void createDataNode(const std::string& type, const std::string& initValue, const std::string& description, const std::string& name, uint64_t parentguiElementSqlID, uint64_t newDataNodeSqlID, bool writePermission);
    void createGuiElementNode(const std::string& name, const std::string &type, const std::string& description, uint64_t parentPageSqlID, uint64_t newGuiELementSqlID);
    void createPageNode(const std::string& title, const std::string& description, uint64_t parentPageSqlID, uint64_t newPageSqlID);
//TO IMPLEMENT
    void createDataNode(const MYSQL_ROW& dataNodeRow);
    void createGuiElementNode(const MYSQL_ROW& guiElementNodeRow);
    void createPageNode(const MYSQL_ROW& pageNodeRow);

    bool start();
    bool stop();
    bool getState();



private:
    UA_Server* server_m = nullptr;
    util::Clock* changeRequestWorkerClock_m = nullptr;
    std::list<ChangeRequest> changeRequests_m;
    std::mutex changeRequestMutex_m;
    std::mutex serverFktMutex_m;
    volatile bool running_m = false;
    UA_StatusCode ServerFkt();
    void ChangeRequestWorker();
    void performChangeRequest(const ChangeRequest& changeRequest);      //TO IMPLEMENT

    void generateNodeID(std::string& prefix, uint64_t sqlID);
    UA_NodeId generateNodeID(const IdType& type, uint64_t sqlID);

    void createVariable(const UA_VariableAttributes& attributes, const UA_NodeId& newNodeID, const UA_NodeId &parentNodeID = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    void createObject(const UA_ObjectAttributes& attributes, const UA_NodeId& newNodeID, const UA_NodeId &parentNodeID = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));

    const size_t changeRequestsMaxCnt = 100;        // if this Count is reached, the coresponding flush funktion
                                                    // will not add new change Requests and will not block (maby this isnt a good idea because the coresponding changerequest is lost if this occurs)
    const int64_t changeRequestWorkerTimebase = 20; // Time in  Milliseconds only used in Constructor

    std::map<std::string,uint8_t> basicTypeMapping
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

};

#endif
