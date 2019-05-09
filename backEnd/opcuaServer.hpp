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

class OpcuaServer
{
public:

    OpcuaServer();
    ~OpcuaServer();
    void flushChangeRequest(const ChangeRequest& changeRequest);
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
    void performChangeRequest(const ChangeRequest& changeRequest);
private:
    const size_t changeRequestsMaxCnt = 100;        // if this Count is reached, the coresponding flush funktion
                                                    // will not add new change Requests and will not block
    const int64_t changeRequestWorkerTimebase = 20; // Time in  Milliseconds only used in Constructor
};
#endif
