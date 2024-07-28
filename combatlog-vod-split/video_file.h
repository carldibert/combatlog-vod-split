#pragma once
#include <string>
#include <windows.h>
#include <cstdlib>

#include "ffmpeg.h"

struct SYSTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

class video_file
{
    public:
        std::string fileName;
        SYSTIME startTime;
        int videoDuration;
        video_file(std::string file, bool obsDate);

        //to be depricated
        SYSTIME endTime;
        std::string date;
        int64_t duration;
        int durationInSeconds;
        video_file();
        video_file(std::string file, int64_t dur);
        SYSTIME GetEndTime(float seconds);
        void InitFile(std::string file);
};