#include "globalInclude.hpp"
#include "backend.hpp"
#include <iostream>
Backend backend;
int main()
{
    bool run = true;
    while(run){
        int userInput = 1;
        std::cout << "This is the Backend-Deamon-Process, "
                     "to kill the Process type \"0\" and press Enter" << std::endl;

        std::cin >> userInput;
        if(userInput == 0){
            run = 0;
        }
        run = 0;
    }
    return 0;
}
