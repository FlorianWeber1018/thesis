#include "globalInclude.hpp"
#include "backend.hpp"
#include <iostream>
#define serverRun 0
Backend backend;
int main()
{
    bool run = true;
    while(run){
        int userInput = 1;
        std::cout << "This is the Backend-Deamon-Process, "
                     "to kill the Process type \"0\" and press Enter" << std::endl;
        if(serverRun){
            std::cin >> userInput;
        }else{
            run = false;
        }
        if(userInput == 0){
            run = false;
        }
    }
    return 0;
}
