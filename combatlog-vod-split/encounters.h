#pragma once
#include "file_handling.h"
#include <vector>
#include <string>

enum InstanceType
{
    Raid = 0,
    Dungeon = 1,
    OpenWorld = 2
};

class encounters
{
    public:
        std::string zone;
        InstanceType instanceType;
        std::string time;
        std::string date;
        LogEventType eventType;
        std::string dungeonName;
        std::string difficulty;
        int keyLevel;     
};

class Encounters_Ordered
{
    public:
        std::string name;
        std::string date;
        std::string startTime;
        std::string endTime;
        std::string zone;
        std::string difficulty;
        int fightNumber;
        int keyLevel;
        bool processed;
        Encounters_Ordered();
};

class Encounters_Total
{
    public:
        Encounters_Total(std::vector<combat_log> contents);
    private:
        std::vector<encounters> encounterList;
        std::vector<Encounters_Ordered> orderedEncounters;
};