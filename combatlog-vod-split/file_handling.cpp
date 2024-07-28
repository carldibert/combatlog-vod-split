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

//gathers attributes for when file was created for the most recent files
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

//compares two files and return the one that was created the nearest
//used for selecting the most recent log file, but I should really do this for the video files too
//for live log splitting
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

//gets the most recent file like the function lists and has a default when only one file in directory
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

//gets the executable path but I changed my mind and made this portable to go off of the documents folder
//now the config is stored in documents
//should have the default location for everything be based on the directory in the users documents
//why do I still have a running bool you can just close out of the window and leave everything always running
file_handling::file_handling()
{
    this->exePath = GetExeDirectory();
    running = false;
};

//old and unused for testing
//I should really remove this at some point
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
            //default naming scheme and if wow decides to change this later on this should be updated
            //or if I get other people to coax me to try and adopt this for some other purpose
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

//gets executable directory
//this is basically unused now and when refactoring I should be able to delete this
std::filesystem::path file_handling::GetExeDirectory()
{
    wchar_t szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    return std::filesystem::path { szPath }.parent_path();
};

//converts difficulty types to enum for some case statements
//I could just compare the strings but for some reason this seems like it would be the cooler option
//why do I do the things that I do?
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

//partial initializer and I am not initializing everything and should really do it properly when I refactor
combat_log_events::combat_log_events(std::string initDate, std::string initTime, std::string initTarget)
{
    this->date = initDate;
    this->time = initTime;
    this->target = initTarget;
};

//no params init used for live log detection
combat_log::combat_log()
{
    this->currentLine = 0;
    this->running = false;
};

//unused now it was when I had the different idea
//will remove this when I refactor
combat_log::combat_log(std::filesystem::path exePath)
{
    this->exePath = exePath;
    this->currentLine = 0;
    this->running = false;
};

//splits string based on a delimiter
//I should really fix this so that way its much less janky
//looking through logs there is one space between the date and time
//two spaces between time and the actual combat event
//should make a space counter and after the 3rd instance it outputs the final part so it should cut down on a second iterator
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

//checks to see if the first digit is a number
//used to see if a combatlog event is a real event or recovering after a game crash/switching toons
bool combat_log::CheckIfNumber(std::string str)
{
    return isdigit(str[0]);
};

//converts a string to an enum because I thought it would be easier to sort by but its just kinda wasting iterations when I can just compare strings
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

//reads a file for splitting I should change the name of this method during refactoring
//or I could use some other kinda thing like to have some kinda enum for the splitting type when initially starting to write
//so I dont really need to have two different methods that do effectively the same thing just one has a counter to only go over new ones
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
                            //activeLine.difficulty = convertToDifficultyTypeEnum(eventsClean[3]);
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

//basically the same as the function above but with an active line tracker for only using the current line to prevent duplicate entries during live splitting
//the same as above where I should really cut down on the duplicate code and add in some way to not have as much duplicate code
bool combat_log::ReadFileLive(std::string fileName)
{
    //gathers filename and creation date of text file
    this->fileName = fileName;

    if (createDate.wYear == 0)
    {
        LPCSTR getString = fileName.c_str();
        WIN32_FILE_ATTRIBUTE_DATA attrs;
        GetFileAttributesExA(getString, GetFileExInfoStandard, &attrs);
        SYSTEMTIME FileTime = { 0 };
        SYSTEMTIME OutFileTimeLocal = { 0 };
        FileTimeToSystemTime(&attrs.ftCreationTime, &FileTime);
        SystemTimeToTzSpecificLocalTimeEx(NULL, &FileTime, &OutFileTimeLocal);
        this->createDate = OutFileTimeLocal;
    }

    //begins searching based off of the last read line in combatlog
    int line = 1;
    std::string combatLogActiveLine;
    std::ifstream file(fileName);
    while (std::getline(file, combatLogActiveLine))
    {
        //if line is newer than previously read line to avoid searching the full file
        if (line >= currentLine)
        {
            try
            {
                //checks to see if the line starts with a number - indicates that there was a game crash
                if (CheckIfNumber(combatLogActiveLine))
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
                    //this is what I should change to just store the raw value cause why did I seem to really want to use enums
                    //I really question the decions that I make 4am honestly
                    LogEventType logType = convertToEnum(eventsClean[0]);
                    if (logType != OTHER)
                    {
                        combat_log_events activeLine
                        (
                            actionEvent[0],
                            actionEvent[1],
                            eventsClean[2]
                        );
                        switch (logType)
                        {
                        case ENCOUNTER_START:
                            combatLogEvents.push_back(activeLine);
                            break;
                        case ENCOUNTER_END:
                            combatLogEvents.push_back(activeLine);
                            std::cout << "Encounter detected" << std::endl;
                            break;
                        case ZONE_CHANGE:
                            //activeLine.difficulty = convertToDifficultyTypeEnum(eventsClean[3]);
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
                            std::cout << "Keystone detected" << std::endl;
                            break;
                        }
                    }
                }
                else
                {
                    //should prolly throw an error like crash detected in log and include a timestamp
                    throw std::runtime_error("\nError Parsing " + fileName + "\nLine: " + std::to_string(line) + " - Missing Date");
                }
            }
            catch (const std::runtime_error& error)
            {
                std::cout << error.what() << '\n';
            }
            currentLine++;
        }
        line++;
    }
    this->currentLine = line;
    return true;
};