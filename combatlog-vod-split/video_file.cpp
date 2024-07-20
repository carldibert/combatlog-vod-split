#include "video_file.h"
#include <string>
#include <windows.h>

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
    this->fileCreateTime = OutFileTimeLocal;
    this->duration = dur;
};