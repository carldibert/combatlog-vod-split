#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <map>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>

#include "file_handling.h"
#include "encounters.h"
#include "ffmpeg.h"
#include "configuration.h"
#include "video_file.h"

//for threading processing of logs 
void ProcessLogs(file_handling* files, std::string logFile)
{
    combat_log log(files->exePath);
    log.ReadFile(logFile);
    files->contents.push_back(log);
};

//for threading the processing of available files
void SplitVideo(std::string in_filename, std::string out_filename, double from_seconds, double end_seconds, ffmpeg* proc)
{
    proc->ProcessFile(in_filename.c_str(), out_filename.c_str(), from_seconds, end_seconds);
}

//checks to see if vod is within the combat log windows
bool CheckIfDatesAreValid(SYSTIME fight, SYSTIME vid)
{
    if (!fight.wMonth - vid.wMonth == 0 ||
        !fight.wMonth - vid.wMonth == 1 ||
        !fight.wMonth - vid.wMonth == 11)
    {
        return false;
    }
    if (!fight.wDay - vid.wDay == 0 ||
        !fight.wDay - vid.wDay == 1 ||
        !fight.wDay - vid.wDay == 27 ||
        !fight.wDay - vid.wDay == 28 ||
        !fight.wDay - vid.wDay == 29 ||
        !fight.wDay - vid.wDay == 30)
    {
        return false;
    }
    return true;
};

//returns the start or endpoints
double GetStartOrEndPoints(SYSTIME fight, SYSTIME vid)
{
    double running = 0;
    if (fight.wHour != vid.wHour)
    {
        if (fight.wHour < vid.wHour)
        {
            running += ((fight.wHour + 24) - vid.wHour) * 3600;
        }
        else
        {
            running += (fight.wHour - vid.wHour) * 3600;
        }
    }
    if (fight.wMinute != vid.wMinute)
    {
        if (fight.wMinute < vid.wMinute)
        {
            running += ((fight.wMinute + 60) - vid.wMinute) * 60;
        }
        else
        {
            running += (fight.wMinute - vid.wMinute) * 60;
        }
    }
    if (fight.wSecond != vid.wSecond)
    {
        if (fight.wSecond < vid.wSecond)
        {
            running += ((fight.wSecond + 60) - vid.wSecond) * 60;
        }
        else
        {
            running += (fight.wSecond - vid.wSecond) * 60;
        }
    }
    return running;
};

//split mode post processing for running after vod has completed processing
bool split_mode_processing(configuration* conf)
{
    //checks for log files within log directory
    file_handling files;
    ffmpeg proc;
    std::vector<std::thread> processingThreads;
    std::vector<video_file> videos;

    //gets log and vod files from directories
    files.CheckForLogFiles(conf->log_directory);
    files.CheckForVodFiles(conf->log_directory);

    //gets vod length
    for (auto& vod : files.vodFiles)
    {
        video_file vid(vod, proc.GetDuration(vod.c_str()));
        videos.push_back(vid);
    }

    //processes through available files and adds log info
    for (int i = 0; i < files.logFiles.size(); i++)
    {
        processingThreads.push_back(std::thread(&ProcessLogs, &files, files.logFiles[i]));
        Sleep(20);
    }
    for (auto& threads : processingThreads)
    {
        threads.join();
    }

    //populates fights and start and end times for all available vods
    Encounters_Total fights(files.contents);
    for (auto& vid : videos)
    {
        float seconds = vid.duration / 1000000;
        vid.date = std::to_string(vid.startTime.wMonth) + "/" + std::to_string(vid.startTime.wDay);
        vid.endTime = vid.GetEndTime(seconds);
    }

    //executes vod splitting in its individual threads 8 threads max as default
    for (auto& fight : fights.orderedEncounters)
    {
        for (auto& vid : videos)
        {
            if (CheckIfDatesAreValid(fight.start, vid.startTime))
            {
                fight.startSeconds = GetStartOrEndPoints(fight.start, vid.startTime);
                fight.endSeconds = GetStartOrEndPoints(fight.end, vid.startTime);
                fight.inFilename = vid.fileName;
                fight.outFilename = conf->log_directory + fight.name + "_" + fight.difficulty + "_" + std::to_string(fight.fightNumber) + ".mkv";
                break;
            }
        }
    }

    //uses threads from config file to concurrently split files
    //I will deal with multithreading this later because its 7am and I should have gone to bed like 9 hours ago
    for (auto& fight : fights.orderedEncounters)
    {
        SplitVideo(fight.inFilename, fight.outFilename, fight.startSeconds-15, fight.endSeconds+15, &proc);
    }

    return true;
};

int main()
{
    //checks for configuration file and returns error when missing information
    configuration conf;
    conf.SetConfiguration();
    if (!conf.CheckForConfigFile())
    {
        return 1;
    }

    if (conf.mode == "split")
    {
        split_mode_processing(&conf);
    }
    else if (conf.mode == "live")
    {

    }

    
    


    
    

    return 0;


};