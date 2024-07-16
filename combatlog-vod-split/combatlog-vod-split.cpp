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
#include "video_file.h"

//for threading processing of logs 
void ProcessLogs(file_handling* files, std::string logFile)
{
    combat_log log(files->exePath);
    log.ReadFile(logFile);
    files->contents.push_back(log);
}

int main()
{
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

    Encounters_Total fights(files.contents);

    
    //system call for if I want to be lazy and not actually implement the ffmpeg library cause I know this works
    //system("ffmpeg -ss 00:20:00 -to 00:30:00 -i X:\\2024-07-09_19-57-00.mkv -c copy X:\\proof.webm");

    
    

    return 0;


};