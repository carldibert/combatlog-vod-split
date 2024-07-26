#pragma once
#include <string>
#include <vector>
#include <filesystem>

class driver
{
    public:
        std::string mode;
        std::vector<std::string> logFiles;
        driver();
        void SetCombatLogLocation(std::string dirLocation);
};

