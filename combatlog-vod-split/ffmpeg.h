#pragma once
#include <iostream>
#include <string>

#include <libavformat/avformat.h>
#include <libavutil/dict.h>


class ffmpeg
{
public:
	bool ProcessFile(const char* in_filename, const char* out_filename, double from_seconds, double end_seconds);
};

