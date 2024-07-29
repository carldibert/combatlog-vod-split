#include "video_file.h"
#include <string>
#include <windows.h>
#include <cstdlib>
#include <vector>

#include "ffmpeg.h"

//initializes file for when the constructor is not explicitly called with a file
//should combine this with the other constructor for when I refactor
void video_file::InitFile(std::string file)
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
}

//default constructor should do something about this and initialize everything
//but I should also refactor so is this really what im thinking of doing
//prolly not safe but whatever
video_file::video_file()
{
    
};

std::vector<std::string> GetOBSDateFromFileName(std::string str)
{
    std::vector<std::string> fileName;
    std::vector<std::string> fileSplit;
    std::vector<std::string> result;
    std::string current = "";

    //gets the filename from the directory
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == '/')
        {
            if (current != "")
            {
                fileName.push_back(current);
                current = "";
            }
            continue;
        }
        current += str[i];
    }

    if (current.size() != 0)
    {
        fileName.push_back(current);
    }

    //returning back a default value of -1 in the case of failing 
    if (!fileName.size() == 2)
    {
        return result;
    }
    
    //returns a split string from just the file name
    current = "";
    for (int i = 0; i < fileName[2].size(); i++)
    {
        if (fileName[2][i] == ' ')
        {
            if (current != "")
            {
                fileSplit.push_back(current);
                current = "";
            }
            continue;
        }
        current += fileName[2][i];
    }

    if (current.size() != 0)
    {
        fileSplit.push_back(current);
    }

    //checking the length and adding them together to split into individual parts
    std::string fileNameCombined = "";
    if (fileSplit.size() == 2)
    {
        fileNameCombined = fileSplit[0] + "-" + fileSplit[1];
    }

    current = "";
    for (int i = 0; i < fileNameCombined.size(); i++)
    {
        if (fileNameCombined[i] == '-')
        {
            if (current != "")
            {
                result.push_back(current);
                current = "";
            }
            continue;
        }
        current += fileNameCombined[i];
    }

    if (current.size() != 0)
    {
        result.push_back(current);
    }

    return result;
}

video_file::video_file(std::string file, bool obsDate)
{
    ffmpeg proc;
    SYSTIME startTime = { 0 };
    std::vector<std::string> splits = GetOBSDateFromFileName(file);
    if (splits.size() == 6 && obsDate)
    {
        std::vector<std::string> splits = GetOBSDateFromFileName(file);
        startTime.wYear = stoi(splits[0]);
        startTime.wMonth = stoi(splits[1]);
        startTime.wDay = stoi(splits[2]);
        startTime.wHour = stoi(splits[3]);
        startTime.wMinute = stoi(splits[4]);
        startTime.wSecond = stoi(splits[5]);
    }
    else
    {
        LPCSTR getString = fileName.c_str();
        WIN32_FILE_ATTRIBUTE_DATA attrs;
        GetFileAttributesExA(getString, GetFileExInfoStandard, &attrs);
        SYSTEMTIME FileTime = { 0 };
        SYSTEMTIME OutFileTimeLocal = { 0 };
        FileTimeToSystemTime(&attrs.ftCreationTime, &FileTime);
        SystemTimeToTzSpecificLocalTimeEx(NULL, &FileTime, &OutFileTimeLocal);

        startTime.wYear = OutFileTimeLocal.wYear;
        startTime.wMonth = OutFileTimeLocal.wMonth;
        startTime.wDay = OutFileTimeLocal.wDay;
        startTime.wHour = OutFileTimeLocal.wHour;
        startTime.wMinute = OutFileTimeLocal.wMinute;
        startTime.wSecond = OutFileTimeLocal.wSecond;
        startTime.wMilliseconds = OutFileTimeLocal.wMilliseconds;
    }

    //int dur = proc.GetDuration(file.c_str()) / 1000000;
    int hours = (proc.GetDuration(file.c_str()) / 1000000) / 3600;
    int minutes = ((proc.GetDuration(file.c_str()) / 1000000) % 3600) / 60;
    int seconds = (((proc.GetDuration(file.c_str()) / 1000000) % 3600) % 60);

    SYSTIME endTime = { 0 };
    
    if ((startTime.wHour + hours) > 24)
    {
        endTime.wHour = (startTime.wHour + hours) / 24;
        endTime.wDay = startTime.wDay + 1;
    }
    else
    {
        endTime.wHour = startTime.wHour + hours;
        endTime.wDay = startTime.wDay;
    }
    if ((startTime.wMinute + minutes) > 24)
    {
        endTime.wMinute = (startTime.wMinute + minutes) / 60;
    }
    else
    {
        endTime.wMinute = startTime.wMinute + minutes;
    }


    if ((startTime.wHour + endTime.wHour) > 24)
    {
        endTime.wDay = startTime.wDay + 1;
    }
    else
    {
        endTime.wDay = startTime.wDay;
    }
    if ((startTime.wDay != endTime.wDay))
    {
        if ((startTime.wDay + 1) > 31 && startTime.wMonth == 1 ||
            (startTime.wDay + 1) > 31 && startTime.wMonth == 3 ||
            (startTime.wDay + 1) > 31 && startTime.wMonth == 5 ||
            (startTime.wDay + 1) > 31 && startTime.wMonth == 7 ||
            (startTime.wDay + 1) > 31 && startTime.wMonth == 8 ||
            (startTime.wDay + 1) > 31 && startTime.wMonth == 10 ||
            (startTime.wDay + 1) > 31 && startTime.wMonth == 12)
            {
                endTime.wMonth = startTime.wMonth + 1;
                endTime.wYear = startTime.wYear;
            }
        else if ((startTime.wDay + 1) > 30 && startTime.wMonth == 4 ||
            (startTime.wDay + 1) > 30 && startTime.wMonth == 6 ||
            (startTime.wDay + 1) > 30 && startTime.wMonth == 9 ||
            (startTime.wDay + 1) > 30 && startTime.wMonth == 11)
            {
                endTime.wMonth = startTime.wMonth + 1;
                endTime.wYear = startTime.wYear;
            }
        else if ((startTime.wDay + 1) > 29 && startTime.wMonth == 2 && startTime.wYear % 4 == 0 ||
            (startTime.wDay + 1) > 28 && startTime.wMonth == 2 && startTime.wYear % 4 > 0)
            {
                endTime.wMonth = startTime.wMonth + 1;
                endTime.wYear = startTime.wYear;
            }
        else
        {
            endTime.wMonth = startTime.wMonth;
            endTime.wYear = startTime.wYear;
        }
        if (endTime.wMonth == 13)
        {
            endTime.wMonth = endTime.wMonth - 1;
            endTime.wYear = startTime.wYear + 1;
        }
    }
    else
    {
        endTime.wMonth = startTime.wMonth;

    }



    this->fileName = file;
    this->startTime = startTime;
    this->videoDuration = proc.GetDuration(file.c_str()) / 1000000;
};

//constructor with built in time for post stream splitting
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

//gathers end time of video which is useful for splitting afterwards but kinda pointless for live splitting
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