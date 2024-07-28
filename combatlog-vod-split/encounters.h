#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include "file_handling.h"
#include "video_file.h"

enum InstanceType
{
    Raid = 0,
    Dungeon = 1,
    OpenWorld = 2
};

enum LogEventType
{
    ENCOUNTER_START = 0,
    ENCOUNTER_END = 1,
    ZONE_CHANGE = 2,
    CHALLENGE_MODE_START = 3,
    CHALLENGE_MODE_END = 4,
    OTHER = 99
};

enum DifficultyType
{
    Normal = 14,
    Heroic = 15,
    Mythic = 16,
    LFR = 17,
    World = 100,
    Keystone = 101
};

class encounters_unfiltered
{
    public:
        SYSTIME time;
        LogEventType eventType;
        std::string name;
        DifficultyType difficulty;
        int keyLevel;
};

class encounters
{
    public:
        std::string encounterName;
        SYSTIME startTime;
        SYSTIME endTime;
        std::string zone;
        std::string difficulty;
        int duration;
        int fightNumber;
};

class encounter_list
{
    public:
        int currentLine;
        std::vector<encounters_unfiltered> unfilteredEncounters;
        std::vector<encounters> fights;

        encounter_list(bool liveMode);
        void ReadFromLog(std::string fileName);
        void FormatFights();
};


/*
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
*/
