#include "globalInclude.hpp"
#include "backend.hpp"
#include "util.hpp"
Backend::Backend(){
    if(initDB()){
        util::ConsoleOut() << "jo" ;
    }
}
