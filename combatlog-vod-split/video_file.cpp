#include "video_file.h"
#include <string>
#include <windows.h>
#include <cstdlib>

video_file::video_file(std::string file)
{
    this->fileName = file;
    LPCSTR getString = fileName.c_str();
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    GetFileAttributesExA(getString, GetFileExInfoStandard, &attrs);
    SYSTEMTIME FileTime = { 0 };
    SYSTEMTIME OutFileTimeLocal = { 0 };
    FileTimeToSystemTime(&attrs.ftCreationTime, &FileTime);
    SystemTimeToTzSpecificLocalTimeEx(NULL, &FileTime, &OutFileTimeLocal);

    SYSTIME startTime
    {
        OutFileTimeLocal.wYear,
        OutFileTimeLocal.wMonth,
        OutFileTimeLocal.wDay,
        OutFileTimeLocal.wHour,
        OutFileTimeLocal.wMinute,
        OutFileTimeLocal.wSecond,
        OutFileTimeLocal.wMilliseconds
    };

    this->startTime = startTime;
};

video_file::video_file(std::string file, int64_t dur)
{
    this->fileName = file;
    LPCSTR getString = fileName.c_str();
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    GetFileAttributesExA(getString, GetFileExInfoStandard, &attrs);
    SYSTEMTIME FileTime = { 0 };
    SYSTEMTIME OutFileTimeLocal = { 0 };
    FileTimeToSystemTime(&attrs.ftCreationTime, &FileTime);
    SystemTimeToTzSpecificLocalTimeEx(NULL, &FileTime, &OutFileTimeLocal);

    SYSTIME startTime
    {
        OutFileTimeLocal.wYear,
        OutFileTimeLocal.wMonth,
        OutFileTimeLocal.wDay,
        OutFileTimeLocal.wHour,
        OutFileTimeLocal.wMinute,
        OutFileTimeLocal.wSecond,
        OutFileTimeLocal.wMilliseconds
    };

    this->startTime = startTime;
    this->duration = dur;
};

SYSTIME video_file::GetEndTime(float seconds)
{
    SYSTIME returnTime
    {
        0,
        0,
        0,
        0,
        0,
        0,
        startTime.wMilliseconds
    };
    float extraTime = 0;
    auto extra = std::div(startTime.wSecond + seconds, 60);
    returnTime.wSecond = extra.rem;
    extraTime = extra.quot;
    extra = std::div(startTime.wMinute + extraTime, 60);
    returnTime.wMinute = extra.rem;
    extraTime = extra.quot;
    extra = std::div(startTime.wHour + extraTime, 60);
    returnTime.wHour = extra.rem;
    extraTime = extra.quot;
    extra = std::div(startTime.wDay + extraTime, 24);
    returnTime.wDay = extra.rem;
    extraTime = extra.quot;
    int daysInMonth;
    if (startTime.wMonth == 1 ||
        startTime.wMonth == 3 ||
        startTime.wMonth == 5 ||
        startTime.wMonth == 7 ||
        startTime.wMonth == 8 ||
        startTime.wMonth == 10 ||
        startTime.wMonth == 12)
    {
        daysInMonth = 31;
    }
    else if (startTime.wMonth == 4 ||
        startTime.wMonth == 6 ||
        startTime.wMonth == 9 ||
        startTime.wMonth == 11)
    {
        daysInMonth = 30;
    }
    else if (startTime.wMonth == 2 || !startTime.wYear % 4 == 0)
    {
        daysInMonth = 28;
    }
    else
    {
        daysInMonth = 29;
    }
    if (extra.quot > 0)
    {
        returnTime.wYear = startTime.wYear + 1;
    }
    else
    {
        returnTime.wYear = startTime.wYear;
    }
    extra = std::div(startTime.wMonth + extraTime, daysInMonth);
    returnTime.wMonth = extra.rem;
    extraTime = extra.quot;
    int daysInYear;
    if (!startTime.wYear % 4 == 0)
    {
        daysInYear = 365;
    }
    else
    {
        daysInYear = 366;
    }

    
    return returnTime;
}