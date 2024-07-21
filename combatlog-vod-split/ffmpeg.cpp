#include "ffmpeg.h"
#include <string>
#include <iostream>
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
    #include <libavcodec/avcodec.h>
}

static void log_packet(const AVFormatContext* fmt_ctx, const AVPacket* pkt, const char* tag)
{
    AVRational* time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
}

bool ffmpeg::ProcessFile(const char* in_filename, const char* out_filename, double from_seconds, double end_seconds)
{
    const AVOutputFormat* ofmt = NULL;
    AVFormatContext* ifmt_ctx = NULL, * ofmt_ctx = NULL;
    AVPacket* pkt = NULL;
    int ret, i;
    int stream_index = 0;
    int stream_mapping_size = 0;
    //from_seconds = 10;
    //end_seconds = 20;

    //reserving memory for packet
    pkt = av_packet_alloc();

    //if packet is unable to be allocated returns a failure
    if (!pkt)
    {
        std::cout << "Error allocating space for packet" << std::endl;
        return false;
    }

    //opens file and if unable to open return failure
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
    {
        std::string errorString = in_filename;
        std::cout << "Error opening file: " + errorString << std::endl;
        return false;
    }

    //retrieves input stream information
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        std::cout << "Failed to retrieve input stream information" << std::endl;
        return false;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    //created output context
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx)
    {
        std::cout << "Error creating output context" << std::endl;
        return false;
    }

    //gathers stream mapping information for all available streams
    int* stream_mapping = new int[ifmt_ctx->nb_streams];
    if (!stream_mapping)
    {
        std::cout << "Error allocating memory for streams" << std::endl;
        return false;
    }

    ofmt = ofmt_ctx->oformat;

    //gathers infomation about streams
    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        AVStream* out_stream;
        AVStream* in_stream = ifmt_ctx->streams[i];
        AVCodecParameters* in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE)
        {
            stream_mapping[i] = -1;
            continue;
        }

        stream_mapping[i] = stream_index++;

        //formats output streams
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream)
        {
            std::cout << "Error formatting output streams" << std::endl;
            return false;
        }

        //copies input codecs to output file
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0)
        {
            std::cout << "Error copying streams to output file" << std::endl;
            return false;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    //outputs formatting of output file
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    //attempts to write flags to output file
    if (!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            std::cout << "Error opening file" << std::endl;
            return false;
        }
    }

    //reformats output header
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        std::cout << "Error reformatting output header" << std::endl;
        return false;
    }

    //sets the start time of the video to the seconds sent in from request
    ret = av_seek_frame(ifmt_ctx, -1, from_seconds * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        fprintf(stderr, "Error seek\n");
        return false;
    }

    while (1)
    {
        
        AVStream* in_stream, * out_stream;

        //reads frame and if unable to be read exits
        ret = av_read_frame(ifmt_ctx, pkt);
        if (ret < 0)
        {
            break;
        }

        //gathers stream information
        in_stream = ifmt_ctx->streams[pkt->stream_index];
        out_stream = ofmt_ctx->streams[pkt->stream_index];

        //if the time is less than the end time copies frames and moves into the output file if not ends transmission
        if ((av_q2d(in_stream->time_base) * pkt->pts) < end_seconds)
        {
            pkt->stream_index = stream_mapping[pkt->stream_index];
            out_stream = ofmt_ctx->streams[pkt->stream_index];
            av_packet_rescale_ts(pkt, in_stream->time_base, out_stream->time_base);
            pkt->pos = -1;
            ret = av_interleaved_write_frame(ofmt_ctx, pkt);
            if (ret < 0) {
                fprintf(stderr, "Error muxing packet\n");
                break;
            }
        }
        else
        {
            break;
        }
        
    }

    //writes to file
    av_write_trailer(ofmt_ctx);

    //frees up memory
    av_packet_free(&pkt);
    avformat_close_input(&ifmt_ctx);

    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
    {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);

    return true;
};

//gets duration of video for file sorting purposes
int64_t ffmpeg::GetDuration(const char* filename)
{
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    avformat_open_input(&pFormatCtx, filename, NULL, NULL);
    int64_t duration = pFormatCtx->duration;
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    return duration;
};