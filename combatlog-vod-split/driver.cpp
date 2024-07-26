#include "driver.h"

#include <string>
#include <vector>
#include <filesystem>

//sets the combatlog and iterates through all of the files within the logDirectory location
//adds all of the files that are text files that contain WoWCombatLog which is how the game formats the log files
void driver::SetCombatLogLocation(std::string dirLocation)
{
    for(const auto& file : std::filesystem::directory_iterator(dirLocation))
    {
        if (file.path().extension() == ".txt")
        {
            //default naming scheme and if wow decides to change this later on this should be updated
            //or if I get other people to coax me to try and adopt this for some other purpose
            if (file.path().generic_string().find("WoWCombatLog") != std::string::npos)
            {
                logFiles.push_back(file.path().generic_string());
            }
        }
    }
};

//for init on loading
driver::driver()
{
    std::vector<std::string> logFiles;
    this->mode = "";
    this->logFiles = logFiles;
};