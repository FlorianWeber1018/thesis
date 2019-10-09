#include "globalInclude.hpp"
#include "mariadb/mysql.h"
#ifndef rwfile__hpp
#define rwfile__hpp
namespace util{

template <typename Container, typename Key>
bool includes(Container container, Key key)
{
    return container.count(key) > 0 ? true : false;
}

template <class Container>
void split(const std::string& str, Container& cont, char delim = ';')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}

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
    ConsoleOut& operator << (const std::list<T>& dataList){
        mutex_s.lock();
        std::lock_guard<std::mutex> lg(mutex_s, std::adopt_lock);
        for(auto&& element : dataList){
            std::cout << element << std::endl;
        }
        return *this;
    }
    template <typename T>
    ConsoleOut& operator << (const T& data){
        mutex_s.lock();
        std::lock_guard<std::mutex> lg(mutex_s, std::adopt_lock);
        std::cout << data << std::endl;
        return *this;
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


enum ClockTrigger{noTrigger, A_trigger, B_trigger};

class Clock
{
public:
    Clock(std::chrono::milliseconds _T, std::function<void(void)> callback);
    void start();
    void stop();
protected:
    void threadloopA();
    void threadloopB();

    std::chrono::milliseconds T;

    std::mutex m_triggerMutex;
    volatile ClockTrigger m_trigger = noTrigger;
    ClockTrigger getTrigger();
    void setTrigger(const ClockTrigger& newTrigger);
    void setTriggerConditionary(const ClockTrigger& newTrigger, const ClockTrigger& waitTrigger);

    std::mutex m_CallbackMutex;
    std::function<void(void)> m_callback;
private:

};

template <typename T>
void moveToBorders(T &value, T const& min, T const& max)
{
  if(value < min){
    value = min;
  }else if(value > max){
    value = max;
  }
}

template <typename T>
void moveToBorders(
  T &value, T const& min, T const& max, T const& min_value, T const& max_value)
{
  if(value < min){
    value = min_value;
  }else if(value > max){
    value = max_value;
  }
}
}
#endif
