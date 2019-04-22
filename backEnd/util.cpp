#include "globalInclude.hpp"
#include "util.hpp"
#include <fstream>
namespace util{
std::mutex ConsoleOut::mutex_s;

bool RwFile::readFile(const std::string& fileName, std::function<bool(const std::string& line ) noexcept> lineHandler)
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
}
