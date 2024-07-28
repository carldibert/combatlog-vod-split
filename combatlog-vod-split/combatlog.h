#pragma once

#include <string>
#include <windows.h>

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
    World = 100
};

struct SYSTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

class combatlog
{
	public:
        LogEventType logAction;
        std::string target;
        SYSTIME date;
        SYSTIME time;
        int keyLevel;
        DifficultyType difficulty;
        std::string dungeonName;
        bool keyChested;
        bool isOpenWorld;
};

