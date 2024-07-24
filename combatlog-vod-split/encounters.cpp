#include "encounters.h"
#include "file_handling.h"
#include "video_file.h"
#include "ffmpeg.h"
#include <vector>
#include <string>
#include <thread>
#include <future>

Encounters_Ordered::Encounters_Ordered()
{
    processed = false;
    failed = false;
};

void Encounters_Ordered::GetStartTime(SYSTIME vid)
{
    int extraHours = 0;

    if (start.wHour < vid.wHour)
    {
        extraHours = 24;
    }

    int fightSeconds = ((start.wHour + extraHours) * 3600) + (start.wMinute * 60) + start.wSecond;
    int vidSeconds = (vid.wHour * 3600) + (vid.wMinute * 60) + vid.wSecond;

    this->startSeconds = fightSeconds - vidSeconds;
};

bool ProcessVid(std::string inFile, std::string outFile, double start, double duration)
{
    ffmpeg proc;
    if (proc.ProcessFile(inFile.c_str(), outFile.c_str(), start - 20, start + duration + 20))
    {
        std::cout << "Processing completed" << std::endl;
        return true;
    }
    else
    {
        return false;
    }
}

void Encounters_Total::RunThroughLog()
{
    while (running)
    {
        
        files.log.ReadFileLive(files.log.fileName);
        PopulateEncounters(files.log);
        for (int i = 0; i < orderedEncounters.size(); i++)
        {
            if (!orderedEncounters[i].processed)
            {
                if (orderedEncounters[i].keyLevel <= 0)
                {
                    orderedEncounters[i].GetStartTime(vid.startTime);
                    orderedEncounters[i].inFilename = vid.fileName;
                    orderedEncounters[i].outFilename = logDirectory +
                        std::to_string(orderedEncounters[i].start.wYear) + "_" +
                        std::to_string(orderedEncounters[i].start.wMonth) + "-" +
                        std::to_string(orderedEncounters[i].start.wDay) + "-" +
                        orderedEncounters[i].name + "_" + orderedEncounters[i].difficulty + "_" + std::to_string(orderedEncounters[i].fightNumber) + ".mkv";
                }
                else
                {
                    orderedEncounters[i].GetStartTime(vid.startTime);
                    orderedEncounters[i].inFilename = vid.fileName;
                    orderedEncounters[i].outFilename = logDirectory +
                        std::to_string(orderedEncounters[i].start.wYear) + "_" +
                        std::to_string(orderedEncounters[i].start.wMonth) + "-" +
                        std::to_string(orderedEncounters[i].start.wDay) + "-" +
                        orderedEncounters[i].zone + "_" + std::to_string(orderedEncounters[i].keyLevel) + "_" + std::to_string(orderedEncounters[i].fightNumber) + ".mkv";
                }
                std::future<bool> asyncVideoProcess = std::async(&ProcessVid, orderedEncounters[i].inFilename.c_str(), orderedEncounters[i].outFilename.c_str(), orderedEncounters[i].startSeconds - 20, orderedEncounters[i].duration + 20);
                bool result = asyncVideoProcess.get();
                if (result)
                {
                    orderedEncounters[i].processed = true;
                }
            }
        }
        int j = 0;
    }
};

bool Encounters_Total::processEncounters()
{
    if (files.logFiles.size() == 0)
    {
        files.CheckForLogFiles(logDirectory);
    }
    if (files.vodFiles.size() == 0)
    {
        files.CheckForVodFiles(vodDirectory);
    }
    if (files.log.fileName == "")
    {
        files.log.fileName = files.currentLog;
    }
    if (vid.fileName == "")
    {
        video_file vid;
        vid.InitFile(files.vodFiles[0]);
        this->vid = vid;
        this->vodStartTime = vid.startTime;
    }
    
    running = true;
    std::thread logRunner(&Encounters_Total::RunThroughLog, this);

    while (running)
    {
        int j = 0;
    }
    
    

    
    return true;
}

//get difficulty name
std::string GetDifficultyName(DifficultyType diff)
{
    switch(diff)
    {
        case 14:
            return "Normal";
            break;
        case 15:
            return "Heroic";
            break;
        case 16:
            return "Mythic";
            break;
        case 17:
            return "LFR";
            break;
    }
}

//gathers info from list and sanitizes the data
void PopulateEncounterList(std::vector<encounters>* encounterList, std::vector<combat_log> contents)
{
    for (auto& var : contents)
    {
        for (auto& evnt : var.combatLogEvents)
        {
            encounters tmp;
            tmp.zone = evnt.target;
            if (evnt.difficulty == 100 && evnt.keyLevel < 0)
            {
                tmp.instanceType = OpenWorld;
            }
            else if (evnt.logAction == 3)
            {
                tmp.instanceType = Dungeon;
                tmp.dungeonName = evnt.dungeonName;
                tmp.keyLevel = evnt.keyLevel;
            }
            else if (evnt.logAction == 4)
            {
                tmp.instanceType = Dungeon;
            }
            else if (evnt.difficulty == 14 || evnt.difficulty == 15 || evnt.difficulty == 16 || evnt.difficulty == 17)
            {
                tmp.instanceType = Raid;
                tmp.difficulty = GetDifficultyName(evnt.difficulty);
            }
            tmp.time = evnt.time;
            tmp.date = evnt.date;
            tmp.eventType = evnt.logAction;
            tmp.year = var.createDate.wYear;
            encounterList->push_back(tmp);
        }
    }
};

//gathers info from list and sanitizes the data
void PopulateEncounterList(std::vector<encounters>* encounterList, combat_log contents)
{
    for (auto& evnt : contents.combatLogEvents)
    {
        encounters tmp;
        tmp.zone = evnt.target;
        if (evnt.difficulty == 100 && evnt.keyLevel < 0)
        {
            tmp.instanceType = OpenWorld;
        }
        else if (evnt.logAction == 3)
        {
            tmp.instanceType = Dungeon;
            tmp.dungeonName = evnt.dungeonName;
            tmp.keyLevel = evnt.keyLevel;
        }
        else if (evnt.logAction == 4)
        {
            tmp.instanceType = Dungeon;
        }
        else if (evnt.difficulty == 14 || evnt.difficulty == 15 || evnt.difficulty == 16 || evnt.difficulty == 17)
        {
            tmp.instanceType = Raid;
            tmp.difficulty = GetDifficultyName(evnt.difficulty);
        }
        tmp.time = evnt.time;
        tmp.date = evnt.date;
        tmp.eventType = evnt.logAction;
        tmp.year = contents.createDate.wYear;
        encounterList->push_back(tmp);
    }
};

std::vector<int> SplitString(std::string str, char splitter)
{
    std::vector<int> result;
    std::string current = "";
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == splitter)
        {
            if (current != "")
            {
                result.push_back(stoi(current));
                current = "";
            }
            continue;
        }
        current += str[i];
    }

    if (current.size() != 0)
    {
        result.push_back(stoi(current));
    }

    return result;
};

std::vector<int> SplitString(std::string str, char splitter, char splitter2)
{
    std::vector<int> result;
    std::string current = "";
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == splitter || str[i] == splitter2)
        {
            if (current != "")
            {
                result.push_back(stoi(current));
                current = "";
            }
            continue;
        }
        current += str[i];
    }

    if (current.size() != 0)
    {
        result.push_back(stoi(current));
    }

    return result;
};

void GenerateSysTime(Encounters_Ordered* temp, int year)
{
    std::vector<int> startTime = SplitString(temp->startTime, ':', '.');
    std::vector<int> endTime = SplitString(temp->endTime, ':', '.');
    std::vector<int> dates = SplitString(temp->date, '/');
    
    SYSTIME start
    {
        year,
        dates[0],
        dates[1],
        startTime[0],
        startTime[1],
        startTime[2],
        startTime[3]
    };
    SYSTIME end
    {
        year,
        dates[0],
        dates[1],
        endTime[0],
        endTime[1],
        endTime[2],
        endTime[3]
    };
    temp->start = start;
    temp->end = end;
    
    //gets fight duration in seconds
    int tmpEndHour;
    if (end.wHour == 0 && start.wHour == 23)
    {
        tmpEndHour = 24;
    }
    else
    {
        tmpEndHour = end.wHour;
    }
    int dur = ((tmpEndHour - start.wHour) * 60) + ((end.wMinute - start.wMinute) * 60) + (end.wSecond - start.wSecond);
    temp->duration = ((tmpEndHour - start.wHour) * 60) + ((end.wMinute - start.wMinute) * 60) + (end.wSecond - start.wSecond);
};

//orders encounters with their appropriate start and end times + dates
void OrderEncounter(std::vector<Encounters_Ordered>* orderedEncounters, std::vector<encounters>encounterList)
{
    Encounters_Ordered tmp;
    std::string currentZone;
    for (auto& var : encounterList)
    {
        //when instance type of 0 for retil or 2 for classic sets the start time and difficulty
        //when instance type is a key it sets the key level
        if (var.instanceType == 0 || var.instanceType == 2)
        {
            if (var.eventType == 0)
            {
                tmp.startTime = var.time;
                tmp.difficulty = var.difficulty;
            }
            else if (var.eventType == 1 && tmp.startTime != "")
            {
                tmp.endTime = var.time;
            }
            else if (var.eventType == 2)
            {
                currentZone = var.zone;
            }
            tmp.name = var.zone;
        }
        else if (var.instanceType == 1)
        {
            if (var.eventType == 3)
            {
                tmp.startTime = var.time;
                tmp.name = var.dungeonName;
                tmp.keyLevel = var.keyLevel;
                currentZone = var.dungeonName;
            }
            else if (var.eventType == 4 && tmp.startTime != "")
            {
                tmp.endTime = var.time;
            }
        }
        else if (var.instanceType == 2 && var.zone != "0")
        {
            currentZone = var.zone;
        }
        if (tmp.endTime != "")
        {
            tmp.zone = currentZone;
            tmp.date = var.date;
            if (orderedEncounters->size() > 0)
            {
                int fightNumber = 1;
                for (auto& comp : *orderedEncounters)
                {
                    if (comp.name == tmp.name && comp.difficulty == tmp.difficulty)
                    {
                        fightNumber++;
                    }
                }
                tmp.fightNumber = fightNumber;
            }
            else
            {
                tmp.fightNumber = 1;
            }

            //generates time of encounters and does not add in fights shorter than 5 seconds for boss resets
            GenerateSysTime(&tmp, var.year);
            if (tmp.duration > 15)
            {
                orderedEncounters->push_back(tmp);
            } 
            tmp = Encounters_Ordered();
        }
    }
};

//orders encounters with their appropriate start and end times + dates
void Encounters_Total::OrderEncounters()
{
    Encounters_Ordered tmp;
    std::string currentZone;
    for (auto& var : encounterList)
    {
        //when instance type of 0 for retil or 2 for classic sets the start time and difficulty
        //when instance type is a key it sets the key level
        if (var.instanceType == 0 || var.instanceType == 2)
        {
            if (var.eventType == 0)
            {
                tmp.startTime = var.time;
                tmp.difficulty = var.difficulty;
            }
            else if (var.eventType == 1 && tmp.startTime != "")
            {
                tmp.endTime = var.time;
            }
            else if (var.eventType == 2)
            {
                currentZone = var.zone;
            }
            tmp.name = var.zone;
        }
        else if (var.instanceType == 1)
        {
            if (var.eventType == 3)
            {
                tmp.startTime = var.time;
                tmp.name = var.dungeonName;
                tmp.keyLevel = var.keyLevel;
                currentZone = var.dungeonName;
            }
            else if (var.eventType == 4 && tmp.startTime != "")
            {
                tmp.endTime = var.time;
            }
        }
        else if (var.instanceType == 2 && var.zone != "0")
        {
            currentZone = var.zone;
        }
        if (tmp.endTime != "")
        {
            tmp.zone = currentZone;
            tmp.date = var.date;
            if (orderedEncounters.size() > 0)
            {
                int fightNumber = 1;
                for (auto& comp : orderedEncounters)
                {
                    if (comp.name == tmp.name && comp.difficulty == tmp.difficulty)
                    {
                        fightNumber++;
                    }
                }
                tmp.fightNumber = fightNumber;
            }
            else
            {
                tmp.fightNumber = 1;
            }

            //generates time of encounters and does not add in fights shorter than 5 seconds for boss resets
            GenerateSysTime(&tmp, var.year);
            if (tmp.duration > 15)
            {
                bool contains = false;
                for (auto& comp : orderedEncounters)
                {
                    if (tmp.start.wMinute == comp.start.wMinute && tmp.start.wSecond == comp.start.wSecond)
                    {
                        contains = true;
                    }
                }
                if (!contains)
                {
                    orderedEncounters.push_back(tmp);
                }
            }
            tmp = Encounters_Ordered();
        }
    }
};

Encounters_Total::Encounters_Total()
{

};

Encounters_Total::Encounters_Total(std::vector<combat_log> contents)
{
    PopulateEncounterList(&encounterList, contents);
    OrderEncounter(&orderedEncounters, encounterList);
};

void Encounters_Total::PopulateEncounters(std::vector<combat_log> contents)
{
    PopulateEncounterList(&encounterList, contents);
    OrderEncounter(&orderedEncounters, encounterList);
};

void Encounters_Total::PopulateEncounters(combat_log contents)
{
    PopulateEncounterList(&encounterList, contents);
    OrderEncounters();
    while (!encounterList.empty())
    {
        encounterList.pop_back();
    } 
};