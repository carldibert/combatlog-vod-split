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


class combat_log_events
{
public:
    combat_log_events(std::string initDate, std::string initTime, std::string initLogAction, std::string target);
private:
    std::string logAction;
    std::string target;
    std::string date;
    std::string time;
    std::string keyLevel;
    std::string raidDifficulty;
};

class combat_log
{
public:
    std::string fileName;
    std::vector<combat_log_events> combatLogEvents;
    std::vector<std::string> SplitString(std::string str, char splitter);
    std::filesystem::path exePath;
    SYSTEMTIME createDate;
    combat_log(std::filesystem::path exePath);
    bool ReadFile(std::string fileName);
    bool CheckIfNumber(std::string str);
private:

};

class file_handling
{
public:
	file_handling();
    bool CheckForLogFiles();
    std::filesystem::path exePath;
    std::vector<std::string> logFiles;
    std::map<std::string, combat_log> contents;
private:
    std::filesystem::path GetExeDirectory();

};

class instance_definitions
{
public:
    std::vector<std::string> raids;
    std::vector<std::string> dungeons;
    std::filesystem::path localappdata;
    char* path;
    instance_definitions();
    void something_output();
};