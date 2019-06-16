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
    void flushChangeRequest(const ChangeRequest& changeRequest); //not blocking



    void createVariable(const UA_VariableAttributes& attributes, const UA_NodeId &newNodeID, const UA_NodeId &parentNodeID = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));

    void createDataNode(const std::string& type, const std::string& defaultValue, const std::string& description, const std::string& name, uint64_t ParentguiElementSqlID, uint64_t newDataNodeSqlID);
    void createGuiElementNode(const std::string& type, const std::string& description, const std::string& name, uint64_t ParentPageSqlID, uint64_t newGuiELementSqlID);
    void createPageNode(const std::string& description, const std::string& name, const std::string& title, uint64_t ParentPageSqlID, uint64_t newPageID);


    bool start();
    bool stop();
    bool getState();
protected:
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

private:
    const size_t changeRequestsMaxCnt = 100;        // if this Count is reached, the coresponding flush funktion
                                                    // will not add new change Requests and will not block (maby this isnt a good idea because the coresponding changerequest is lost if this occurs)
    const int64_t changeRequestWorkerTimebase = 20; // Time in  Milliseconds only used in Constructor
};
#endif
