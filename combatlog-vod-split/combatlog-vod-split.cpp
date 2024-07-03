#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <map>
#include <stdlib.h>

#include "file_handling.h"
#include <windows.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}


void ProcessLogs(file_handling* files, std::string logFile)
{
    combat_log log(files->exePath);
    log.ReadFile(logFile);
    files->contents.insert(std::pair<std::string, combat_log>(logFile, log));
}

int main()
{
    int threadCount = 8;

    instance_definitions defys;

    file_handling files;

    files.CheckForLogFiles();
    std::vector<std::thread> processingThreads;

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

    
    
    




    return 0;


}