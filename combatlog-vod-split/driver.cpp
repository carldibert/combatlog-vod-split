#include "driver.h"

#include <string>
#include <vector>
#include <filesystem>

#include "processor.h"
#include "video_file.h"
#include "encounters.h"

//processing but made like way cleaner
void driver::StartProcessing()
{
    //searches for video files that can be used in the vod directory
    std::cout << "Searching for video files" << std::endl;
    for (auto& file : vodFiles)
    {
        video_file tmp(file, obsName);
        this->vods.push_back(tmp);
    }
    std::cout << "valid video files found: " + std::to_string(vods.size()) << std::endl;
    std::cout << "Searching for log files" << std::endl;

    //processes through the log files - false is for post processing true is live
    encounter_list tmp(false);
    for (auto& file : logFiles)
    {
        tmp.ReadFromLog(file);
        tmp.FormatFights();
    }
    int i = 0;
};

//sets the combatlog and iterates through all of the files within the logDirectory location
//adds all of the files that are text files that contain WoWCombatLog which is how the game formats the log files
void driver::SetCombatLogLocation(std::string dirLocation)
{
    for(const auto& file : std::filesystem::directory_iterator(dirLocation))
    {
        if (file.path().extension() == ".txt")
        {
            //default naming scheme and if wow decides to change this later on this should be updated
            //or if I get other people to coax me to try and adopt this for some other purpose
            if (file.path().generic_string().find("WoWCombatLog") != std::string::npos)
            {
                this->logFiles.push_back(file.path().generic_string());
            }
        }
    }
};

//accepts in the log directory and outputs the video files for processing
void driver::SetVideoFileLocation(std::string dirLocation)
{
    for(const auto& file : std::filesystem::directory_iterator(dirLocation))
    {
        //only using mkv as an input file because you can manipulate the file without worrying
        //about the file being potentially corrupt for mp4 files or whatnot
        if (file.path().extension() == ".mkv")
        {
            this->vodFiles.push_back(file.path().generic_string());
        }
    }
};

//for init on loading
driver::driver()
{
    processor proc;
    this->mode = "";
    this->proc = proc;
    this->currentLine = -1;
    this->obsName = false;
};