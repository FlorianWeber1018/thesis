#include "globalInclude.hpp"
#include "functional"
#ifndef rwfile__hpp
#define rwfile__hpp
namespace util{

class RwFile
{
public:
    static bool readFile(
            const std::string& fileName,
            std::function<bool(const std::string& line ) noexcept> lineHandler);
    static bool readFile(const std::string& fileName, std::list<std::string>& linesOut);
};



class ConsoleOut{
public:

    template <typename T>
    void operator << (const std::list<T>& dataList){
        mutex_s.lock();
        std::lock_guard<std::mutex> lg(mutex_s, std::adopt_lock);
        for(auto&& element : dataList){
            std::cout << element << std::endl;
        }
    }
    template <typename T>
    void operator << (const T& data){
        mutex_s.lock();
        std::lock_guard<std::mutex> lg(mutex_s, std::adopt_lock);
        std::cout << data << std::endl;
    }
private:
    static std::mutex mutex_s;

};

}
#endif
