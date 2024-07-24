#pragma once
#include <vector>
#include <string>
#include "video_file.h"

#include "file_handling.h"
#include "video_file.h"

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
        int year;
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
        std::string inFilename;
        std::string outFilename;
        SYSTIME start;
        SYSTIME end;
        int duration;
        double startSeconds;
        double endSeconds;
        int fightNumber;
        int keyLevel;
        bool processed;
        bool failed;
        Encounters_Ordered();
        void GetStartTime(SYSTIME vid);
};

class Encounters_Total
{
    public:
        std::string logDirectory;
        std::string vodDirectory;
        std::vector<Encounters_Ordered> orderedEncounters;
        bool running;
        file_handling files;
        video_file vid;
        SYSTIME vodStartTime;
        Encounters_Total();
        Encounters_Total(std::vector<combat_log> contents);
        bool processEncounters();
        void PopulateEncounters(std::vector<combat_log> contents);
        void PopulateEncounters(combat_log contents);
        void RunThroughLog();
        void OrderEncounters();
    private:
        std::vector<encounters> encounterList;      
};