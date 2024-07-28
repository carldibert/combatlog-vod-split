#pragma once
#include <string>
#include <vector>
#include <filesystem>

#include "processor.h"
#include "video_file.h"
#include "encounters.h"

class driver
{
    public:
        std::string mode;
        std::vector<std::string> logFiles;
        std::vector<std::string> vodFiles;
        std::vector<video_file> vods;
        std::vector<encounters> entries;
        processor proc;
        bool obsName;
        int currentLine;
        driver();
        void SetCombatLogLocation(std::string dirLocation);
        void SetVideoFileLocation(std::string dirLocation);
        void StartProcessing();
};
