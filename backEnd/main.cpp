#include "globalInclude.hpp"
#include "backend.hpp"
#include <iostream>
#define serverRun 1
Backend backend;
int main()
{
    bool run = true;
    while(run){

        std::cout << "This is the Backend-Deamon-Process, "
                     "to kill the Process hit CTRL + C" << std::endl;
        if(serverRun){
            std::this_thread::sleep_for(std::chrono::seconds(240));
        }else{
            run = false;
        }
    }
    return 0;
}
//
