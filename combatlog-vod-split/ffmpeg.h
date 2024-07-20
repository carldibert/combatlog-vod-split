#pragma once
#include <iostream>
#include <string>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <map>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>

extern "C"
{
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
}


class ffmpeg
{
    public:
        bool ProcessFile(const char* in_filename, const char* out_filename, double from_seconds, double end_seconds);
        int64_t GetDuration(const char* filename);
};

