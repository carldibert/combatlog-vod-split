#pragma once
#include <string>
#include <windows.h>

class video_file
{
    public:
        std::string fileName;
        SYSTEMTIME fileCreateTime;
        std::string startTime;
        std::string endTime;
        std::string date;
        int64_t duration;
        video_file(std::string file, int64_t dur);        
};