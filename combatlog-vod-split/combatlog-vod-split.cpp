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

    Encounters_Total fights(files.contents);

    
    using clock_type = std::chrono::high_resolution_clock;
    

    for (auto& fight : fights.orderedEncounters)
    {
        for (auto& vid : videos)
        {
            std::string date = std::to_string(vid.fileCreateTime.wMonth) + "/" + std::to_string(vid.fileCreateTime.wDay);
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

    if (conf.mode == "split")
    {
        split_mode_processing(&conf);
    }
    else if (conf.mode == "live")
    {

    }

    
    


    
    

    return 0;


};