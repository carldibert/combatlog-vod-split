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

void log_watching(bool* control)
{
    while (control)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
    }
};

void live_mode_processing(configuration* conf)
{
    ffmpeg proc;
    combat_log log;
    file_handling files;
    files.CheckForLogFiles(conf->log_directory);
    files.CheckForVodFiles(conf->video_directory);

    bool cont = true;
    std::thread logRunner(log_watching, &cont);

    

    cont = false;
    //checks if combat_log_vod_split directory exists and create it does not exist
    std::string configDirectory = conf->log_directory + "\\combat_log_vod_split_processed_logs";
    std::filesystem::path tmp = configDirectory;
    CreateDirectory(tmp.c_str(), NULL);
};

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
    int extraHours = 0;
    
    if (fight.wHour < vid.wHour)
    {
        extraHours = 24;
    }

    int fightSeconds = ((fight.wHour + extraHours) * 3600) + (fight.wMinute * 60) + fight.wSecond;
    int vidSeconds = (vid.wHour * 3600) + (vid.wMinute * 60) + vid.wSecond;
    
    return fightSeconds - vidSeconds;
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
    files.CheckForVodFiles(conf->video_directory);

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
        vid.durationInSeconds = vid.duration / 1000000;
        vid.date = std::to_string(vid.startTime.wMonth) + "/" + std::to_string(vid.startTime.wDay);
        vid.endTime = vid.GetEndTime(seconds);
    }

    //executes vod splitting in its individual threads 8 threads max as default
    for (auto& vid : videos)
    {
        for (auto& fight : fights.orderedEncounters)
        {
            if (CheckIfDatesAreValid(fight.start, videos[0].startTime))
            {
                //checks to see if the event is a key or a raid in retail or classic
                if (fight.keyLevel <= 0)
                {
                    fight.startSeconds = GetStartOrEndPoints(fight.start, videos[0].startTime);
                    fight.endSeconds = GetStartOrEndPoints(fight.end, videos[0].startTime);
                    fight.inFilename = videos[0].fileName;
                    fight.outFilename = conf->log_directory +
                        std::to_string(fight.start.wYear) + "_" +
                        std::to_string(fight.start.wMonth) + "-" +
                        std::to_string(fight.start.wDay) + "-" +
                        fight.name + "_" + fight.difficulty + "_" + std::to_string(fight.fightNumber) + ".mkv";
                }
                else
                {
                    fight.startSeconds = GetStartOrEndPoints(fight.start, videos[0].startTime);
                    fight.endSeconds = GetStartOrEndPoints(fight.end, videos[0].startTime);
                    fight.inFilename = videos[0].fileName;
                    fight.outFilename = conf->log_directory +
                        std::to_string(fight.start.wYear) + "_" +
                        std::to_string(fight.start.wMonth) + "-" +
                        std::to_string(fight.start.wDay) + "-" +
                        fight.zone + "_" + std::to_string(fight.keyLevel) + "_" + std::to_string(fight.fightNumber) + ".mkv";
                }
            }
        }
    }
    

    //processes video fights with available video files
    for (auto& fight : fights.orderedEncounters)
    {
        if (SplitVideo(fight.inFilename, fight.outFilename, fight.startSeconds - 15, fight.startSeconds + fight.duration + 15, &proc))
        {
            fight.processed = true;
        }
        else
        {
            fight.processed = false;
        }  
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

    //runs based on mode that is set up within config
    if (conf.mode == "split")
    {
        split_mode_processing(&conf);
    }
    else if (conf.mode == "live")
    {
        live_mode_processing(&conf);
    }

    return 0;
};