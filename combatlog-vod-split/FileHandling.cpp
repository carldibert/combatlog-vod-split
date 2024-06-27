#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#elif
#include <unistd.h>
#endif
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <thread>

#include "FileHandling.h"


combat_log_events::combat_log_events(std::string initDate, std::string initTime, std::string initCombatInfo)
{
    this->date = initDate;
    this->time = initTime;
    this->combatInfo = initCombatInfo;
};

bool combat_log::CheckForLogFiles()
{
    for(const auto& entry : std::filesystem::directory_iterator(GetExeDirectory().generic_string()))
    {
        if(entry.path().extension() == ".txt")
        {
            logFiles.push_back(entry.path().generic_string());
        }
    }
    return true;
};

bool combat_log::CheckIfFileExists(std::string fileName)
{
    //testing file cause im too lazy to add flags
    combatLogFile.open(GetExeDirectory().generic_string() + "\\Split-2024-06-08T235016.570Z-DatheaAscended Mythic.txt");
    //combatLogFile.open(GetExeDirectory().generic_string() + fileName);

    if(combatLogFile.fail())
    {
        return false;
    }
    else
    {
        return true;
    }
};

std::filesystem::path GetExeDirectory()
{
    wchar_t szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    return std::filesystem::path { szPath }.parent_path();
};

std::vector<std::string> SplitString(std::string str, char splitter)
{
    std::vector<std::string> result;
        std::string current = "";
        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == splitter)
            {
                if (current != "")
                {
                    result.push_back(current);
                    current = "";
                }
                continue;
            }
            current += str[i];
        }

    if (current.size() != 0)
    {
        result.push_back(current);
    }

    return result;
};

FileHandling::FileHandling()
{

};

std::filesystem::path combat_log::GetExeDirectory()
{
    wchar_t szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);

    return std::filesystem::path{ szPath }.parent_path();
}



bool combat_log::ReadFile(std::string fileName)
{

    std::string combatLogActiveLine;
    std::cout << "Reading from File: " + fileName << std::endl;
    if(CheckIfFileExists(fileName))
    {
        std::ifstream file(GetExeDirectory().generic_string() + fileName);

        while(std::getline(file, combatLogActiveLine))
        {
            std::vector<std::string> actionEvent = SplitString(combatLogActiveLine, ' ');
            std::string combatEvents;
            for(int i = 2; i < actionEvent.size(); i++)
            {
                combatEvents += actionEvent[i];
            }
            combatLogEvents.push_back(combat_log_events(
                actionEvent[0],
                actionEvent[1],
                combatEvents
            ));
        }
    }

    return true;
}

std::vector<std::string> combat_log::SplitString(std::string str, char splitter)
{
    std::vector<std::string> result;
        std::string current = "";
        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == splitter)
            {
                if (current != "")
                {
                    result.push_back(current);
                    current = "";
                }
                continue;
            }
            current += str[i];
        }

        if (current.size() != 0)
        {
            result.push_back(current);
        }

        return result;
};