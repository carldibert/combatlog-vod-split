#include <iostream>
#include <fstream>

using namespace std;
class combat_log
{
public:
    string combatLog;
    fstream combatLogFile;
    
    bool checkIfFileExists()
    {
        //combatLogFile.open("D:\\Documents\\Split-2024-06-08T235016.570Z-DatheaAscended Mythic.txt");
        combatLogFile.open("Split-2024-06-08T235016.570Z-DatheaAscended Mythic.txt");

        if (combatLogFile.fail())
        {
            cout << "failed" << endl;
        }
        else
        {
            cout << "opened" << endl;
        }

        return true;
    }

    bool getExeFilepath()
    {


        return true;
    }
};

class cli_handling
{
public:
    string userInput;

    bool acceptExitRequest()
    {
        cout << "type r to restart or any other key to exit: ";
        cin >> userInput;
        if (userInput == "r")
        {
            return false;
        }
        else
        {
            return true;
        }
    }
};

int main()
{
    std::cout << "Hello World!\n";

    combat_log activeLog;
    activeLog.checkIfFileExists();
}