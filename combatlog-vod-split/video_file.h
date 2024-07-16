#pragma once
#include <string>
#include <windows.h>

class video_file
{
    public:
        std::string fileName;
        SYSTEMTIME fileCreateTime;
        video_file(std::string file);
};

class output_video
{
    public:
        std::string fileName;
};
