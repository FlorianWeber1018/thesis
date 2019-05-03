#include "globalInclude.hpp"
#include "functional"
#include "mariadb/mysql.h"
#ifndef rwfile__hpp
#define rwfile__hpp
namespace util{

class RwFile
{
public:
    static bool readFile(
            const std::string& fileName,
            std::function<bool(const std::string& line )> lineHandler);
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
class Table; //forward declaration
class Json{
public:
    static bool toDom(const std::string& jsonString, rj::Document& dom_o);
    static std::string toJson(const rj::Document& dom_o);
    static std::string toJson(const Table& table);
};
class Table{
    //Table(const Table& table);
    Table(MYSQL_RES* mysqlResult);
    friend class Json;
private:
    std::string name_m = "";
    std::vector<unsigned int> keyColumns;
    std::vector<enum_field_types> fieldTypes_m;
    std::vector<std::string> headers_m;
    std::vector<std::vector<std::string>> records_m;
};
}
#endif
