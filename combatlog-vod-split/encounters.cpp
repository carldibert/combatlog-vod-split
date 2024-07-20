#include "encounters.h"
#include "file_handling.h"
#include <vector>
#include <string>

Encounters_Ordered::Encounters_Ordered()
{
    processed = false;
};

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
            encounterList->push_back(tmp);
        }
    }
};

//orders encounters with their appropriate start and end times + dates
void OrderEncounters(std::vector<Encounters_Ordered>* orderedEncounters, std::vector<encounters>encounterList)
{
    Encounters_Ordered tmp;
    std::string currentZone;
    for (auto& var : encounterList)
    {
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
            
            orderedEncounters->push_back(tmp);
            tmp = Encounters_Ordered();
        }
    }
};

Encounters_Total::Encounters_Total(std::vector<combat_log> contents)
{
    PopulateEncounterList(&encounterList, contents);
    OrderEncounters(&orderedEncounters, encounterList);
}