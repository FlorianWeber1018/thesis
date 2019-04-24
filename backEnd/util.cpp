#include "globalInclude.hpp"
#include "util.hpp"
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace util{
std::mutex ConsoleOut::mutex_s;

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
}
