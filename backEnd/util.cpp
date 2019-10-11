#include "globalInclude.hpp"
#include "util.hpp"
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace util{
std::mutex ConsoleOut::mutex_s;
bool isUnsignedLL(const std::string& str) noexcept
{
    try{
        std::stoull(str);
    }catch(...) {
        return false;
    }
    return true;
}

bool RwFile::readFile(const std::string& fileName, std::function<bool(const std::string& line )> lineHandler)
{
    std::string line;
    std::ifstream file (fileName);
    if (file.is_open())
    {
        try{
            while ( std::getline (file, line) )
            {
                if(!lineHandler(line)){
                    throw "error in lineHandler";
                }
            }
            file.close();
            return true;
        }catch(...){
            if(file.is_open()){
                file.close();
            }
            return false;
        }
    }else{
        return false;
    }
}

bool RwFile::readFile(const std::string& fileName, std::list<std::string>& linesOut)
{
    auto lambda = [&linesOut](const std::string& line) noexcept -> bool {linesOut.push_back(line);return true;};
    return readFile(fileName, lambda);
}
bool Json::toDom(const std::string& jsonString, rj::Document& dom_o)
{
    dom_o.Parse(jsonString.c_str());
    return !dom_o.HasParseError();
}
std::string Json::toJson(const rj::Document& dom_o)
{
    rj::StringBuffer buffer;
    rj::Writer<rj::StringBuffer> writer(buffer);
    dom_o.Accept(writer);
    return buffer.GetString();
}
std::string Json::toJson(const Table& table)
{
   rj::Document dom;
   //doing stuff with the dom and the table(to implement)


   return toJson(dom);
}
Table::Table(MYSQL_RES* mysqlResult)
{
/*
    dom_o.SetObject();

    if(resultset != nullptr){
        unsigned int fieldCount = mysql_num_fields(resultset);
        std::vector<MYSQL_FIELD*> fields;
        MYSQL_FIELD* field;

        while((field = mysql_fetch_field(resultset)) != nullptr){
            fields.push_back(field);
        }
        while (MYSQL_ROW row = mysql_fetch_row(resultset)) {
            rj::Value jsonRow(rj::kObjectType);
            rj::Value rowKey;
            rowKey.SetString(row[keyColNumber], dom_o.GetAllocator());
            for(uint i=0; i<fieldCount; i++){
                rj::Value val;
                rj::Value colKey;
                colKey.SetString(fields[i]->name, dom_o.GetAllocator());
                if(row[i]!=nullptr){
                    if(fields[i]->type == MYSQL_TYPE_LONGLONG){
                        val.SetUint64(std::stoull(row[i]));
                    }else{
                        val.SetString(row[i], dom_o.GetAllocator());
                    }
                }
                jsonRow.AddMember(colKey, val, dom_o.GetAllocator());
            }
            dom_o.AddMember(rowKey, jsonRow, dom_o.GetAllocator());
        }
        return true;
    }else{
        return false;
    }*/
}
Clock::Clock(std::chrono::milliseconds T, std::function<void(void)> callback)
{
    this->T = T;
    this->m_callback = callback;
    std::thread threadA(&Clock::threadloopA, this);
    std::thread threadB(&Clock::threadloopB, this);
    threadB.detach();
    threadA.detach();

}

void Clock::start(){

    setTrigger(A_trigger);
}
void Clock::stop(){
    setTrigger(noTrigger);
}
ClockTrigger Clock::getTrigger()
{
    m_triggerMutex.lock();
    std::lock_guard<std::mutex> lg(m_triggerMutex, std::adopt_lock);
    return m_trigger;
}
void Clock::setTrigger(const ClockTrigger &newTrigger)
{
    m_triggerMutex.lock();
    std::lock_guard<std::mutex> lg(m_triggerMutex, std::adopt_lock);
    m_trigger = newTrigger;
}
void Clock::setTriggerConditionary(const ClockTrigger &newTrigger, const ClockTrigger &waitTrigger)
{
    while(getTrigger() == waitTrigger) {
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
    setTrigger(newTrigger);
}
void Clock::threadloopA()
{
    while(1){
        while(getTrigger() != A_trigger){      //wait for trigger from B
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::this_thread::sleep_for(T);      //wait cycle Time

        m_CallbackMutex.lock();
        std::lock_guard<std::mutex> lg(m_CallbackMutex, std::adopt_lock);

        setTriggerConditionary(B_trigger, noTrigger);    //trigger B

        m_callback();
    }
}
void Clock::threadloopB()

{
    while(1){
        while(getTrigger() != B_trigger){      //wait for trigger from A
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::this_thread::sleep_for(T);      //wait cycle Time

        m_CallbackMutex.lock();
        std::lock_guard<std::mutex> lg(m_CallbackMutex, std::adopt_lock);

        setTriggerConditionary(A_trigger, noTrigger);      //trigger A

        m_callback();
    }
}


}
