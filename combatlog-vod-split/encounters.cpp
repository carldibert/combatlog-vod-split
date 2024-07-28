#include "encounters.h"
#include "file_handling.h"
#include "video_file.h"
#include "ffmpeg.h"
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <fstream> 
#include <iostream> 
#include <string> 

//splits a string based off of a delimiter
std::vector<std::string> SplitStringCombatLog(std::string str, char splitter)
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
bool CheckIfNumber(std::string str)
{
    return isdigit(str[0]);
};

//gets the file creation date
SYSTEMTIME GetFileCreationDate(std::string fileName)
{
    LPCSTR getString = fileName.c_str();
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    GetFileAttributesExA(getString, GetFileExInfoStandard, &attrs);
    SYSTEMTIME FileTime = { 0 };
    SYSTEMTIME OutFileTimeLocal = { 0 };
    FileTimeToSystemTime(&attrs.ftCreationTime, &FileTime);
    SystemTimeToTzSpecificLocalTimeEx(NULL, &FileTime, &OutFileTimeLocal);
    return OutFileTimeLocal;
};

//gets an unformatted line with the system date being resolved
SYSTIME GetUnformattedEncounter(std::vector<std::string> actionEvent, std::string fileName)
{
    std::vector<std::string> day = SplitStringCombatLog(actionEvent[0], '/');
    std::vector<std::string> time = SplitStringCombatLog(actionEvent[1], ':');

    SYSTEMTIME fileTime = GetFileCreationDate(fileName);

    SYSTIME eventTime
    {
        fileTime.wYear,
        std::stoi(day[0]),
        std::stoi(day[1]),
        std::stoi(time[0]),
        std::stoi(time[1]),
        std::stoi(time[2]),
        0
    };
    return eventTime;
};

//splits the combat log into a usable format because what is wow's formatting
std::vector<std::string> SplitCombatLogEvent(std::string line)
{
    std::vector<std::string> result;
    
    //counts everything after the first 3 spaces in the combatlog because blizz has some weird formatting
    std::string temp = "";
    int spaceCounter = 0;
    for (int i = 0; i < line.size(); i++)
    {
        if (spaceCounter > 2)
        {
            temp += line[i];
        }
        else
        {
            if (line[i] == ' ')
            {
                spaceCounter++;
            }
        }
    }
    std::vector<std::string> working = SplitStringCombatLog(temp, ',');

    //filters based on the quote being in the first character of a CSV entry and removes it as well as the following quote in the following entry
    for (int i = 0; i < working.size(); i++)
    {
        if (working[i][0] != 34)
        {
            result.push_back(working[i]);
        }
        else
        {
            working[i].erase(0, 1);
            //because the compiler doesn't like if I dont check for this
            int e = working[i].size() - 1;
            if (working[i][e] == 34)
            {
                result.push_back(working[i].erase(e, e));
            }
            else
            {
                std::string temp = working[i] + ", " + working[i + 1].erase(working[i + 1].size() - 1, working[i + 1].size());
                result.push_back(temp);
                i++;
            }            
        }
        
    }

    return result;
};

//gets the difficulty type from a string for the sake of filtering later on
//have an idea that it would only split mythic or something later on down the line
DifficultyType GetDifficultyTypeFromString(std::string line)
{
    switch (std::stoi(line))
    {
        case 14:
            return Normal;
        case 15:
            return Heroic;
        case 16:
            return Mythic;
        case 17:
            return LFR;
        default:
            return World;
    }
};

//reads through the combatlog and has an unfiltered raw data before getting the specifics that are formatted well
void ReadCombatLog(std::vector<encounters_unfiltered>* unfiltered, std::string line, std::string fileName)
{
    //reserves memory and filters the combatlog in a few different ways because wow doesnt like to zero out the lines
    encounters_unfiltered tmp;
    std::vector<std::string> actionEvent = SplitStringCombatLog(line, ' ');
    std::vector<std::string> combatLogEvent = SplitCombatLogEvent(line);

    //iterates through the combatlog and populates the fields in the object with trimmed down info
    //don't need to have all of the specific information just the start stop and following along the zones
    if (combatLogEvent[0] == "ENCOUNTER_START" ||
        combatLogEvent[0] == "ENCOUNTER_END" ||
        combatLogEvent[0] == "ZONE_CHANGE" ||
        combatLogEvent[0] == "CHALLENGE_MODE_START" ||
        combatLogEvent[0] == "CHALLENGE_MODE_END")
    {
        tmp.time = GetUnformattedEncounter(actionEvent, fileName);
        if (combatLogEvent[0] == "ENCOUNTER_START")
        {
            tmp.eventType = ENCOUNTER_START;
            tmp.name = combatLogEvent[2];
            tmp.keyLevel = -1;
            tmp.difficulty = GetDifficultyTypeFromString(combatLogEvent[3]);
            unfiltered->push_back(tmp);
        }
        else if (combatLogEvent[0] == "ENCOUNTER_END")
        {
            tmp.eventType = ENCOUNTER_END;
            tmp.name = combatLogEvent[2];
            tmp.keyLevel = -1;
            tmp.difficulty = GetDifficultyTypeFromString(combatLogEvent[3]);
            unfiltered->push_back(tmp);
        }
        else if (combatLogEvent[0] == "ZONE_CHANGE")
        {
            tmp.eventType = ZONE_CHANGE;
            tmp.name = combatLogEvent[2];
            tmp.difficulty = GetDifficultyTypeFromString(combatLogEvent[3]);
            unfiltered->push_back(tmp);
        }
        else if (combatLogEvent[0] == "CHALLENGE_MODE_START")
        {
            tmp.eventType = CHALLENGE_MODE_START;
            tmp.name = combatLogEvent[1];
            tmp.keyLevel = std::stoi(combatLogEvent[4]);
            tmp.difficulty = Keystone;
            unfiltered->push_back(tmp);
        }
        else if (combatLogEvent[0] == "CHALLENGE_MODE_END")
        {
            if (std::stoi(combatLogEvent[3]) > 0)
            {
                tmp.eventType = CHALLENGE_MODE_END;
                tmp.name = "";
                tmp.keyLevel = std::stoi(combatLogEvent[3]);
                tmp.difficulty = Keystone;
                unfiltered->push_back(tmp);
            }
        }
    }
};

//used to get the value of the enum for the purpose of the filename
std::string GetDifficultyValue(DifficultyType difficulty)
{
    switch (difficulty)
    {
        case (Normal):
            return "Normal";
        case (Heroic):
            return "Heroic";
        case (Mythic):
            return "Mythic";
        case (LFR):
            return "LFR";
    }
};

//gets fight duration in seconds
int GetDuration(SYSTIME startTime, SYSTIME endTime)
{
    //used in the case a fight starts before midnight and ends after the clock rolls over to the next day
    int tmpEndHour;
    if (endTime.wHour == 0 && startTime.wHour == 23)
    {
        tmpEndHour = 24;
    }
    else
    {
        tmpEndHour = endTime.wHour;
    }
    return ((tmpEndHour - startTime.wHour) * 3600) + ((endTime.wMinute - startTime.wMinute) * 60) + (endTime.wSecond - startTime.wSecond);
};

//formats fights into a usable format for populating the video file information
void encounter_list::FormatFights()
{
    encounters tmp;
    std::string currentZone;
    for (auto& var : unfilteredEncounters)
    {
        //filters based off of the event type in log
        //uses zone filtering to work with classic andys and their speed run logs
        switch (var.eventType)
        {
            case ENCOUNTER_START:
                if (var.difficulty != 100)
                {
                    tmp.encounterName = var.name;
                    tmp.startTime = var.time;
                    tmp.difficulty = GetDifficultyValue(var.difficulty);
                    tmp.zone = currentZone;
                }
                break;
            case ENCOUNTER_END:
                if (var.difficulty != 100)
                {
                    tmp.endTime = var.time;
                    tmp.duration = GetDuration(tmp.startTime, tmp.endTime);
                    if (tmp.duration > 20)
                    {
                        tmp.fightNumber = -1;
                        this->fights.push_back(tmp);
                    }
                    tmp = encounters();
                }
                break;
            case ZONE_CHANGE:
                currentZone = var.name;
                break;
            case CHALLENGE_MODE_START:
                tmp.encounterName = var.name;
                tmp.startTime = var.time;
                tmp.difficulty = std::to_string(var.keyLevel);
                tmp.zone = currentZone;
                break;
            case CHALLENGE_MODE_END:
                tmp.endTime = var.time;
                tmp.duration = GetDuration(tmp.startTime, tmp.endTime);
                if (tmp.duration > 20)
                {
                    tmp.fightNumber = -1;
                    this->fights.push_back(tmp);
                }
                tmp = encounters();
                break;
        }
    };
    
    //used to calculate the amount of fights that have occured so it lines up with warcraftlogs
    //I think 20 seconds is the duration I should use im not sure
    int fight = 1;
    for (auto& var : fights)
    {
        for (int i = 0; i < fights.size(); i++)
        {
            if (fights[i].fightNumber != -1)
            {
                if (fights[i].encounterName == var.encounterName)
                {
                    fight++;
                }
            }
        }
        var.fightNumber = fight;
        fight = 1;
    }
    
    //empties the unfiltered encounters for the live processing purposes
    while (!unfilteredEncounters.empty())
    {
        unfilteredEncounters.pop_back();
    }
};

//reads from the log
void encounter_list::ReadFromLog(std::string fileName)
{
    std::vector<std::string> linesInFile;
    
    int line = 1;
    std::string combatLogActiveLine;
    std::ifstream file(fileName);
    std::vector<encounters_unfiltered> unfiltered;

    while (std::getline(file, combatLogActiveLine))
    {
        if (currentLine == -1)
        {
            try
            {
                //checks to see if the first digit is a number to check in case of a crash
                if (CheckIfNumber(combatLogActiveLine))
                {
                    //runs through the combatlog and adds events to vector
                    ReadCombatLog(&unfiltered, combatLogActiveLine, fileName);
                }
                else
                {
                    throw std::runtime_error("\nError Parsing " + fileName + "\nLine: " + std::to_string(line) + " - Missing Date");
                }
            }
            //error handling which is mostly just to catch if the log file wasnt updated after a crash
            catch (const std::runtime_error& error)
            {
                std::cout << error.what() << '\n';
            }
            line++;
        }
        //this is for the live processing ill handle this later
        else
        {
            if (line >= currentLine)
            {
                try
                {
                    if (CheckIfNumber(combatLogActiveLine))
                    {
                        ReadCombatLog(&unfiltered, combatLogActiveLine, fileName);
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
            line++;
        } 
    }

    //settings this as a variable for the object to use
    this->unfilteredEncounters = unfiltered;
};

//init for modes live mode follows the most recent line so it doesnt have to iterate through the whole file every single time
encounter_list::encounter_list(bool liveMode)
{
    if (liveMode)
    {
        this->currentLine = 1;
    }
    else
    {
        this->currentLine = -1;
    }
};