#pragma once
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#elif
#include <unistd.h>
#endif
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <thread>


class combat_log_events
{
    std::string date;
    std::string time;
    std::string combatInfo;
public:
    combat_log_events(std::string initDate, std::string initTime, std::string initCombatInfo);
};

class FileHandling
{
public:
    std::string fileLocation;
    std::string fileDirectory;
    FileHandling();
};

class combat_log
{
public:
    std::fstream combatLogFile;
    std::vector<combat_log_events> combatLogEvents;
    std::vector<std::string> logFiles;
    std::string fileName;

    bool CheckForLogFiles();
    bool CheckIfFileExists(std::string fileName);
    std::vector<std::string> SplitString(std::string str, char splitter);

private:
    std::filesystem::path GetExeDirectory();
    bool ReadFile(std::string fileName);
};
