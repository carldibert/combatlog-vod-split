#define PROJECT_NAME "combatlog-vod-split"
#include <filesystem>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <stdlib.h>
#include "file_handling.h"

file_handling::file_handling()
{
    this->exePath = GetExeDirectory();
};

bool file_handling::CheckForLogFiles()
{
    for(const auto& entry : std::filesystem::directory_iterator(exePath.generic_string()))
    {
        if(entry.path().extension() == ".txt")
        {
            logFiles.push_back(entry.path().generic_string());
        }
    }

    return true;
};

std::filesystem::path file_handling::GetExeDirectory()
{
    wchar_t szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    return std::filesystem::path { szPath }.parent_path();
}

combat_log_events::combat_log_events(std::string initDate, std::string initTime, std::string initLogAction, std::string initTarget)
{
    this->date = initDate;
    this->time = initTime;
    this->logAction = initLogAction;
    this->target = initTarget;
};

combat_log::combat_log(std::filesystem::path exePath)
{
    this->exePath = exePath;
};

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

bool combat_log::CheckIfNumber(std::string str)
{
    return isdigit(str[0]);
};

bool combat_log::ReadFile(std::string fileName)
{
    //gathers filename and creation date of text file
    this->fileName = fileName;
    std::wstring stemp = std::wstring(fileName.begin(), fileName.end());
    WIN32_FILE_ATTRIBUTE_DATA fInfo;
    GetFileAttributesEx(stemp.c_str(), GetFileExInfoStandard, &fInfo);
    SYSTEMTIME FileTime = { 0 };
    SYSTEMTIME OutFileTimeLocal = { 0 };
    FileTimeToSystemTime(&fInfo.ftCreationTime, &FileTime);
    SystemTimeToTzSpecificLocalTimeEx(NULL, &FileTime, &OutFileTimeLocal);
    this->createDate = OutFileTimeLocal;

    int line = 1;
    std::string combatLogActiveLine;
    std::cout << "Beginning to Read File: " + fileName << std::endl;
    std::ifstream file(fileName);
    while(std::getline(file, combatLogActiveLine))
    {
        try
        {
            if(CheckIfNumber(combatLogActiveLine))
            {
                std::vector<std::string> actionEvent = SplitString(combatLogActiveLine, ' ');
                std::vector<std::string> combatEvents = SplitString(actionEvent[2], ',');
                std::string targetEvent;
                for (int i = 2; i < actionEvent.size(); i++)
                {
                    if (i == 2)
                    {
                        targetEvent += actionEvent[i];
                    }
                    else
                    {
                        targetEvent += " " + actionEvent[i];
                    }
                }
                
                std::vector<std::string> eventsClean;
                if (targetEvent.find("\"") != std::string::npos)
                {
                    std::vector<std::string> target = SplitString(targetEvent, '\"');
                    for (int i = 0; i < target.size(); i++)
                    {
                        if (i % 2 != 0)
                        {
                            eventsClean.push_back(target[i]);
                        }
                        else if (i == target.size() - 1)
                        {
                            std::vector<std::string> tmp = SplitString(target[i].substr(1, target[i].length()), ',');
                            for (int j = 0; j < tmp.size(); j++)
                            {
                                eventsClean.push_back(tmp[j]);
                            }
                        }
                        else if (i == 0)
                        {
                            std::vector<std::string> tmp = SplitString(target[i].substr(0, target[i].size() - 1), ',');
                            for (int j = 0; j < tmp.size(); j++)
                            {
                                eventsClean.push_back(tmp[j]);
                            }
                        }
                        else
                        {
                            std::vector<std::string> tmp = SplitString(target[i].substr(1, target[i].size() - 2), ',');
                            for (int j = 0; j < tmp.size(); j++)
                            {
                                eventsClean.push_back(tmp[j]);
                            }
                        }
                    }
                }
                else
                {
                    eventsClean = SplitString(targetEvent, ',');
                }
                    
                
                //whitelisted events - encounter for raids and zone entered for m+
                if (
                    combatEvents[0] == "ENCOUNTER_START" || 
                    combatEvents[0] == "ENCOUNTER_END" ||
                    combatEvents[0] == "ZONE_CHANGE" ||
                    combatEvents[0] == "CHALLENGE_MODE_START")
                {
                combatLogEvents.push_back(combat_log_events(
                    actionEvent[0],
                    actionEvent[1],
                    eventsClean[0],
                    eventsClean[2]
                    ));
                }
                line++;
            }
            else
            {
                throw std::runtime_error("\nError Parsing " + fileName + "\nLine: " + std::to_string(line) + " - Missing Date");
            }
        }
        catch (const std::runtime_error& error)
        {
            std::cout << error.what() << '\n';
        }
    }
    std::cout << "Ending Read on File: " << fileName << std::endl;
    return true;
};

instance_definitions::instance_definitions()
{

}

void instance_definitions::something_output()
{
    //char* appData = System.getenv("AppData");
    //std::cout << path << std::endl;
}