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
