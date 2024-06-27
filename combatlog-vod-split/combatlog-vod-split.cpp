#include <iostream>
#include <string>
#include "FileHandling.h"
#include "CombatLog.h"

class cli_handling
{
public:
    std::string userInput;

    bool AcceptExitRequest()
    {
        std::cout << "type R to restart or any other key to exit: ";
        std::cin >> userInput;
        if (userInput == "r")
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    bool ProcessFiles()
    {

    }
};

int main()
{
    std::cout << "Hello World!\n" << std::endl;

    //CombatEvents combatEvents("date1", "time1", "event1");
    //std::cout << combatEvents.GetDate() << std::endl;

}