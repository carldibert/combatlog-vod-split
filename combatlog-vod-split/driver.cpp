#include "driver.h"

#include <string>
#include <vector>
#include <filesystem>

#include "ffmpeg.h"
#include "video_file.h"
#include "encounters.h"

//gets the month length in seconds and returns
int GetSecondsInMonth(int month, int year)
{
    if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
    {
        return (31 * 86400);
    }
    else if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        return (30 * 86400);
    }
    else if (year % 4 == 0)
    {
        return (29 * 86400);
    }
    else
    {
        return (28 * 86400);
    }
};

bool CheckIfFightIsInVideo(SYSTIME vidTime, SYSTIME fileTime, int videoDuration)
{
    

    int differenceInSeconds =
        ((vidTime.wYear - fileTime.wYear) * 31536000) +
        (GetSecondsInMonth(vidTime.wMonth, vidTime.wYear) - GetSecondsInMonth(fileTime.wMonth, fileTime.wYear)) +
        ((vidTime.wDay - fileTime.wDay) * 86400) +
        ((vidTime.wHour - fileTime.wHour) * 3600) +
        ((vidTime.wMinute - fileTime.wMinute) * 60) + 
        (vidTime.wSecond - fileTime.wSecond);

    //compares to see if the video duration is shorter than the difference from the start of video from the log file
    //if the duration is larger then it returns true since the fight is within the video
    if (videoDuration > differenceInSeconds)
    {
        return true;
    }
    else
    {
        return false;
    }
};

void driver::LiveProcessing()
{

};

//processing for the split mode
void driver::SplitProcessing()
{
    //searches for video files that can be used in the vod directory
    std::cout << "Searching for video files" << std::endl;
    for (auto& file : vodFiles)
    {
        video_file tmp(file, obsName);
        this->vods.push_back(tmp);
    }
    std::cout << "Valid video files found: " + std::to_string(vods.size()) << std::endl;
    std::cout << "Searching for log files" << std::endl;

    //processes through the log files - false is for post processing true is live
    encounter_list fights(false);
    for (auto& file : logFiles)
    {
        fights.ReadFromLog(file);
        fights.FormatFights();
    }
    std::cout << "Number of encounters found: " + std::to_string(fights.fights.size()) << std::endl;

    ffmpeg proc;
    std::string inFilename = "";
    //starting to process encounters
    for (auto& fight : fights.fights)
    {
        video_file activeVideo;
        if (vods.size() > 1)
        {
            for (auto& vod : vods)
            {
                if (CheckIfFightIsInVideo(fight.startTime, vod.startTime, vod.videoDuration))
                {

                }
            }
        }
        else
        {
            activeVideo = vods[0];
        }

        //do the filehandling and whatnot when im not half asleep
        //proc.ProcessFile(in_filename.c_str(), out_filename.c_str(), from_seconds, end_seconds);
    }
    int i = 0;
};

//processing but made like way cleaner
void driver::StartProcessing()
{
    //removes case sensitivity for modes and initiates mode operation
    transform(mode.begin(), mode.end(), mode.begin(), ::toupper);
    if (mode == "SPLIT")
    {
        SplitProcessing();
    }
    else if (mode == "LIVE")
    {
        LiveProcessing();
    }
    
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
    this->mode = "";
    this->currentLine = -1;
    this->obsName = false;
};