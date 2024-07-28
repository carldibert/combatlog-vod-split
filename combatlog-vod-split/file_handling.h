#pragma once

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
#include <stdio.h>

#include "file_handling.h"
#include "encounters.h"

class combat_log_events
{
public:
    //LogEventType logAction;
    std::string target;
    std::string date;
    std::string time;
    int keyLevel;
    //DifficultyType difficulty;
    std::string dungeonName;
    bool keyChested;
    bool isOpenWorld;
    combat_log_events(std::string initDate, std::string initTime, std::string initTarget);
private:
    
};

class combat_log
{
public:
    std::string fileName;
    std::vector<combat_log_events> combatLogEvents;
    std::vector<std::string> SplitString(std::string str, char splitter);
    std::filesystem::path exePath;
    SYSTEMTIME createDate;
    int currentLine;
    bool running;
    combat_log();
    combat_log(std::filesystem::path exePath);
    bool ReadFile(std::string fileName);
    bool ReadFileLive(std::string fileName);
    bool CheckIfNumber(std::string str);
};

class file_handling
{
public:
	file_handling();
    //unused just for testing
    bool CheckForLogFiles();
    bool CheckForLogFiles(std::string directory);
    bool CheckForVodFiles(std::string directory);
    void GetMostRecentFile();
    std::filesystem::path exePath;
    std::vector<std::string> logFiles;
    std::vector<std::string> vodFiles;
    std::vector<combat_log> contents;
    std::string currentLog;
    combat_log log;
    bool running;
private:
    std::filesystem::path GetExeDirectory();
};