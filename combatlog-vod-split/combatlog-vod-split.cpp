#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <map>
#include <stdlib.h>
#include "file_handling.h"
#include "encounters.h"
#include <windows.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

//for threading processing of logs 
void ProcessLogs(file_handling* files, std::string logFile)
{
    combat_log log(files->exePath);
    log.ReadFile(logFile);
    files->contents.push_back(log);
    //files->contents.insert(std::pair<std::string, combat_log>(logFile, log));
}

//populates the encounter list in a more sanitized format
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
            }
            else if (evnt.logAction == 4)
            {
                tmp.instanceType = Dungeon;
            }
            else if (evnt.difficulty == 14 || evnt.difficulty == 15 || evnt.difficulty == 16 || evnt.difficulty == 17)
            {
                tmp.instanceType = Raid;
            }
            tmp.time = evnt.time;
            tmp.date = evnt.date;
            tmp.eventType = evnt.logAction;
            encounterList->push_back(tmp);
        }
    }
};

//orders encounters by start and end time + gives them names
void OrderEncounters(std::vector<Encounters_Ordered>* orderedEncounters, std::vector<encounters>encounterList)
{
    Encounters_Ordered tmp;
    std::string currentZone;
    for (auto& var : encounterList)
    {
        if (var.instanceType == 0)
        {
            if (var.eventType == 0)
            {
                tmp.startTime = var.time;
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
            orderedEncounters->push_back(tmp);
            tmp = Encounters_Ordered();
        }
    }
}

int main()
{
    file_handling files;

    files.CheckForLogFiles();
    std::vector<std::thread> processingThreads;

    //processes through available files and adds log info
    for (int i = 0; i < files.logFiles.size(); i++)
    {
        processingThreads.push_back(std::thread(&ProcessLogs, &files, files.logFiles[i]));
        Sleep(20);
    }
    for (auto& threads : processingThreads)
    {
        threads.join();
    }

    //gathers info from list and sanitizes the data
    std::vector<encounters> encounterList;
    PopulateEncounterList(&encounterList, files.contents);
    
    //orders encounters with their appropriate start and end times + dates
    std::vector<Encounters_Ordered> orderedEncounters;
    OrderEncounters(&orderedEncounters, encounterList);
    




    return 0;


}