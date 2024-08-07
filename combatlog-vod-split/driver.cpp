#include "driver.h"

#include <string>
#include <vector>
#include <filesystem>

#include "ffmpeg.h"
#include "video_file.h"
#include "encounters.h"

//splits processing video and splits it
bool SplitVideo(std::string in_filename, std::string out_filename, double from_seconds, double end_seconds, ffmpeg* proc)
{
    if (proc->ProcessFile(in_filename.c_str(), out_filename.c_str(), from_seconds, end_seconds))
    {
        return true;
    }
    else
    {
        return false;
    }
};

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

//gets the fight time from the start of the video returned in seconds
double GetFightTimeFromStart(SYSTIME vidTime, SYSTIME fightTime)
{
    return ((fightTime.wYear - vidTime.wYear) * 31536000) +
        (GetSecondsInMonth(fightTime.wMonth, fightTime.wYear) - GetSecondsInMonth(fightTime.wMonth, vidTime.wYear)) +
        ((fightTime.wDay - vidTime.wDay) * 86400) +
        ((fightTime.wHour - vidTime.wHour) * 3600) +
        ((fightTime.wMinute - vidTime.wMinute) * 60) +
        (fightTime.wSecond - vidTime.wSecond);
};

//checks to see if the fight is in the video and if it is returns true
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

//checks to see if the old video is newer than the new video in list
//used for comparing two files to get the most recent vod to use
bool CheckIfFileIsNewer(video_file oldVideo, video_file newVideo)
{
    if (oldVideo.startTime.wYear < newVideo.startTime.wYear)
    {
        return true;
    }
    if (oldVideo.startTime.wMonth < newVideo.startTime.wMonth)
    {
        return true;
    }
    if (oldVideo.startTime.wDay < newVideo.startTime.wDay)
    {
        return true;
    }
    if (oldVideo.startTime.wHour < newVideo.startTime.wHour)
    {
        return true;
    }
    if (oldVideo.startTime.wMinute < newVideo.startTime.wMinute)
    {
        return true;
    }
    if (oldVideo.startTime.wSecond < newVideo.startTime.wSecond)
    {
        return true;
    }

    return false;
}

//gets the most recent video in video directory and populates that in the currentVideo object
void GetMostRecentVideo(video_file* currentVideo, std::vector<video_file> vods)
{
    video_file temp;
    for (auto& vid : vods)
    {
        if (temp.fileName == "")
        {
            temp = vid;
        }
        else
        {
            if (CheckIfFileIsNewer(temp, vid))
            {
                temp = vid;
            }
        }
    }
    currentVideo->fileName = temp.fileName;
    currentVideo->startTime = temp.startTime;
    currentVideo->videoDuration = temp.videoDuration;
    currentVideo->endTime = temp.endTime;
    currentVideo->date = temp.date;
    currentVideo->duration = temp.duration;
    currentVideo->durationInSeconds = temp.durationInSeconds;
};

//checks to see if the new log file is newer than the existing newest file
bool CheckIfLogIsNewer(std::string oldFile, std::string newFile)
{
    //gets the systime for the new and old files
    LPCSTR oldFileString = oldFile.c_str();
    WIN32_FILE_ATTRIBUTE_DATA oldFileAttrs;
    GetFileAttributesExA(oldFileString, GetFileExInfoStandard, &oldFileAttrs);
    SYSTEMTIME OldFileTime = { 0 };
    FileTimeToSystemTime(&oldFileAttrs.ftCreationTime, &OldFileTime);

    LPCSTR newFileString = newFile.c_str();
    WIN32_FILE_ATTRIBUTE_DATA newFileAttrs;
    GetFileAttributesExA(newFileString, GetFileExInfoStandard, &newFileAttrs);
    SYSTEMTIME NewFileTime = { 0 };
    FileTimeToSystemTime(&newFileAttrs.ftCreationTime, &NewFileTime);
    
    //does the file checks and returns true of new files is newer
    if (OldFileTime.wYear < NewFileTime.wYear)
    {
        return true;
    }
    if (OldFileTime.wMonth < NewFileTime.wMonth)
    {
        return true;
    }
    if (OldFileTime.wDay < NewFileTime.wDay)
    {
        return true;
    }
    if (OldFileTime.wHour < NewFileTime.wHour)
    {
        return true;
    }
    if (OldFileTime.wMinute < NewFileTime.wMinute)
    {
        return true;
    }
    if (OldFileTime.wSecond < NewFileTime.wSecond)
    {
        return true;
    }

    return false;
};

//checks through logs and returns the most recent file in terms of creation date
std::string GetMostRecentLog(std::vector<std::string> logFiles)
{
    std::string temp;

    for (auto& file : logFiles)
    {
        if (temp == "")
        {
            temp = file;
        }
        else
        {
            if (CheckIfLogIsNewer(temp, file))
            {
                temp = file;
            }
        }
    }

    return temp;
};

//processing for live splitting vod during fight
bool driver::LiveProcessing()
{
    //searching for active video
    std::cout << "Searching for video file" << std::endl;
    for (auto& file : vodFiles)
    {
        video_file tmp(file, obsName);
        this->vods.push_back(tmp);
    }

    //sets current video to the most recent video in the video directory
    video_file currentVideo;
    if (vods.size() > 1)
    {
        GetMostRecentVideo(&currentVideo, vods);
    }
    else
    {
        currentVideo = vods[0];
    }
    std::cout << "Currently Selected vod: " + currentVideo.fileName << std::endl;

    //sets current log to the most recent log file in the log directory
    std::string currentLog;
    if (logFiles.size() > 1)
    {
        currentLog = GetMostRecentLog(logFiles);
    }
    else
    {
        currentLog = logFiles[0];
    }
    std::cout << "Currently Selected log: " + currentLog << std::endl;
    
    //checks for file name to have been read as well as log to be found
    //error handling returns failure if it cannot
    if (currentVideo.fileName == "")
    {
        std::cout << "Video is unable to be read" << std::endl;
        return false;
    }
    if (currentLog == "")
    {
        std::cout << "Log file is unable to be read" << std::endl;
        return false;
    }

    return true;
};

//processing for the split mode
bool driver::SplitProcessing()
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
    //starting to process encounters
    for (auto& fight : fights.fights)
    {
        for (auto& vod : vods)
        {
            //checks to see if fight is within the video
            if (CheckIfFightIsInVideo(fight.startTime, vod.startTime, vod.videoDuration))
            {
                //generates filename for output
                std::string outFileName =
                    vodDirectory + "\\" +
                    std::to_string(fight.startTime.wYear) + "-" +
                    std::to_string(fight.startTime.wMonth) + "-" +
                    std::to_string(fight.startTime.wDay) + " " +
                    fight.encounterName + " " +
                    fight.difficulty + " " +
                    std::to_string(fight.fightNumber) + ".mkv";

                //gets start time as well as checking to see if adding in the padding for pre-pulls as well as accounting for the B frames
                double startTime = GetFightTimeFromStart(vod.startTime, fight.startTime);
                double startTimeAdjusted = 0;
                if ((startTime - 20) <= 0)
                {
                    startTimeAdjusted = 0;
                }
                else
                {
                    startTimeAdjusted = startTime - 20;
                }

                //processing video and outputs if there is a failure in processing
                if (!SplitVideo(vod.fileName, outFileName, startTimeAdjusted, (startTime + fight.duration) + 20, &proc))
                {
                    std::cout << "Error processing file: " + outFileName << std::endl;
                }
                
            }
        }
    }
    std::cout << "Finished processing" << std::endl;
    return true;
};

//processing but made like way cleaner
bool driver::StartProcessing()
{
    //removes case sensitivity for modes and initiates mode operation
    transform(mode.begin(), mode.end(), mode.begin(), ::toupper);
    if (mode == "SPLIT")
    {
        if (!SplitProcessing())
        {
            std::cout << "Error has occured. Exiting" << std::endl;
            return false;
        }
    }
    else if (mode == "LIVE")
    {
        if (!LiveProcessing())
        {
            std::cout << "Error has occured. Exiting" << std::endl;
            return false;
        }
    }
    return true;
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
    this->vodDirectory = dirLocation;
};

//for init on loading
driver::driver()
{
    this->mode = "";
    this->currentLine = -1;
    this->obsName = false;
};