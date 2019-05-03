#ifndef opcuaServer__hpp
#define opcuaServer__hpp
#define UA_NO_AMALGAMATION
#include <open62541/ua_server.h>
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
    void addChangeRequest(const ChangeRequest& changeRequest);
    bool start();
    bool stop();
    bool getState();
protected:
    std::

    std::list<ChangeRequest> changeRequests_m;
    std::mutex mutex_m;
    volatile bool running_m = false;
    UA_StatusCode ServerFkt();
private:

};
#endif
