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

SYSTEMTIME GetFileAttr(std::string file)
{
    LPCSTR getString = file.c_str();
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    GetFileAttributesExA(getString, GetFileExInfoStandard, &attrs);
    SYSTEMTIME FileTime = { 0 };
    SYSTEMTIME FileTimeLocal = { 0 };
    FileTimeToSystemTime(&attrs.ftCreationTime, &FileTime);
    SystemTimeToTzSpecificLocalTimeEx(NULL, &FileTime, &FileTimeLocal);

    return FileTimeLocal;
};

bool GetNewerFile(std::string oldFile, std::string newFiles)
{
    SYSTEMTIME oldTime = GetFileAttr(oldFile);
    SYSTEMTIME newTime = GetFileAttr(newFiles);

    if (!newTime.wYear > oldTime.wYear)
    {
        return false;
    }
    if (!newTime.wMonth > oldTime.wMonth)
    {
        return false;
    }
    if (!newTime.wDay > oldTime.wDay)
    {
        return false;
    }
    if (!newTime.wMinute > oldTime.wMinute)
    {
        return false;
    }
    if (!newTime.wSecond > oldTime.wSecond)
    {
        return false;
    }

    return true;
};

void file_handling::GetMostRecentFile()
{
    if (logFiles.size() <= 1)
    {
        currentLog = logFiles[0];
    }

    std::string newestFile = logFiles[0];

    for (auto& file : logFiles)
    {
        if (GetNewerFile(newestFile, file))
        {
            newestFile = file;
        }
    }

    currentLog = newestFile;
};

file_handling::file_handling()
{
    this->exePath = GetExeDirectory();
};

//old and unused for testing
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

//checks directory for combatlog files
bool file_handling::CheckForLogFiles(std::string directory)
{
    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.path().extension() == ".txt")
        {
            if (entry.path().generic_string().find("WoWCombatLog") != std::string::npos)
            {
                logFiles.push_back(entry.path().generic_string());
            }
        }
    }

    GetMostRecentFile();
    return true;
};

//checks directory for vod files
bool file_handling::CheckForVodFiles(std::string directory)
{
    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.path().extension() == ".mkv")
        {
            vodFiles.push_back(entry.path().generic_string());
        }
    }
    return true;
};

std::filesystem::path file_handling::GetExeDirectory()
{
    wchar_t szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    return std::filesystem::path { szPath }.parent_path();
};

DifficultyType convertToDifficultyTypeEnum (std::string& diff)
{
    if (diff == "14")
    {
        return Normal;
    }
    else if (diff == "15")
    {
        return Heroic;
    }
    else if (diff == "16")
    {
        return Mythic;
    }
    else if (diff == "17")
    {
        return LFR;
    }
    else
    {
        return World;
    }
};

combat_log::combat_log()
{

};

combat_log_events::combat_log_events(std::string initDate, std::string initTime, LogEventType initLogAction, std::string initTarget)
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

LogEventType convertToEnum (const std::string& str)
{
    if (str == "ENCOUNTER_START")
    {
        return ENCOUNTER_START;
    }
    else if (str == "ENCOUNTER_END")
    {
        return ENCOUNTER_END;
    }
    else if (str == "ZONE_CHANGE")
    {
        return ZONE_CHANGE;
    }
    else if (str == "CHALLENGE_MODE_START")
    {
        return CHALLENGE_MODE_START;
    }
    else if (str == "CHALLENGE_MODE_END")
    {
        return CHALLENGE_MODE_END;
    }
    else
    {
        return OTHER;
    }
};

bool combat_log::ReadFile(std::string fileName)
{
    //gathers filename and creation date of text file
    this->fileName = fileName;

    LPCSTR getString = fileName.c_str();
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    GetFileAttributesExA(getString, GetFileExInfoStandard, &attrs);
    SYSTEMTIME FileTime = { 0 };
    SYSTEMTIME OutFileTimeLocal = { 0 };
    FileTimeToSystemTime(&attrs.ftCreationTime, &FileTime);
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
            //checks to see if the line starts with a number - indicates that there was a game crash
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
                
                //cleaning output with uneven output in combatlog file
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
                  

                //builds log type with actions
                LogEventType logType = convertToEnum(eventsClean[0]);
                if (logType != OTHER)
                {
                    combat_log_events activeLine
                    (
                        actionEvent[0],
                        actionEvent[1],
                        logType,
                        eventsClean[2]
                    );
                    switch (logType)
                    {
                        case ENCOUNTER_START:
                            combatLogEvents.push_back(activeLine);
                            break;
                        case ENCOUNTER_END:
                            combatLogEvents.push_back(activeLine);
                            break;
                        case ZONE_CHANGE:
                            activeLine.difficulty = convertToDifficultyTypeEnum(eventsClean[3]);
                            combatLogEvents.push_back(activeLine);
                            break;
                        case CHALLENGE_MODE_START:
                            activeLine.keyLevel = std::stoi(eventsClean[4]);
                            activeLine.isOpenWorld = false;
                            activeLine.dungeonName = eventsClean[1];
                            combatLogEvents.push_back(activeLine);
                            break;
                        case CHALLENGE_MODE_END:
                            activeLine.keyChested = std::stoi(eventsClean[2]);
                            activeLine.isOpenWorld = false;
                            combatLogEvents.push_back(activeLine);
                            break;
                    }
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