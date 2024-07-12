#pragma once
#include <string>
#include "file_handling.h"

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
        encounters();       
};

class Encounters_Ordered
{
    public:
        std::string name;
        std::string date;
        std::string startTime;
        std::string endTime;
        std::string zone;
        Encounters_Ordered();
};