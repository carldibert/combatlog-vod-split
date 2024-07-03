#include "ffmpeg.h"
#include <string>


extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

ffmpeg::ffmpeg()
{
	
};

void ffmpeg::OutputData()
{

    /*
    AVFormatContext* fmt_ctx = NULL;
    AVDictionaryEntry* tag = NULL;
    int ret;
    const char* filename = "D:\\7okk_jTZ2JMr9zzu.mp4";

    ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL);
    while (tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))
    {
        printf("%s=%s\n", tag->key, tag->value);
    }

    avformat_close_input(&fmt_ctx);
    */
    
}