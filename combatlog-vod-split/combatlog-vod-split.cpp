#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <map>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>

#include "file_handling.h"
#include "encounters.h"
#include "video_file.h"

extern "C"
{
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
}

//for threading processing of logs 
void ProcessLogs(file_handling* files, std::string logFile)
{
    combat_log log(files->exePath);
    log.ReadFile(logFile);
    files->contents.push_back(log);
}

//trying null pointers in the example library in the documentation makes me want to eat my shoe
bool ffmpegSample()
{
    const AVOutputFormat* ofmt = NULL;
    AVFormatContext* ifmt_ctx = NULL, * ofmt_ctx = NULL;
    AVPacket* pkt = NULL;
    const char* in_filename, * out_filename;
    int ret, i;
    int stream_index = 0;
    //int* stream_mapping = NULL;
    int stream_mapping_size = 0;
    int streamVideoIndex = 0;

    in_filename = "X:\\2024-07-12 02-39-33 - Copy.mkv";
    out_filename = "X:\\output.mp4";

    pkt = av_packet_alloc();

    if (!pkt)
    {
        return false;
    }

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
    {
        return false;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        return false;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx)
    {
        return false;
    }

    stream_mapping_size = ifmt_ctx->nb_streams;
    int* stream_mapping = new int[stream_mapping_size];
    if (!stream_mapping)
    {
        return false;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        AVStream* out_stream;
        AVStream* in_stream = ifmt_ctx->streams[i];
        AVCodecParameters* in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        if (in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
        {
            streamVideoIndex = i;
        }

        stream_mapping[i] = stream_index++;

        out_stream = avformat_new_stream(ofmt_ctx, NULL);

        if (!out_stream)
        {
            return false;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0)
        {
            return false;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    if (!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            return false;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        return false;
    }

    int64_t* dts_start_from = new int64_t[stream_mapping_size];
    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    int64_t* pts_start_from = new int64_t[stream_mapping_size];
    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    

    AVFrame* frame;
    AVPacket inPacket, outPacket;
    AVIOContext* outFormatContext;

    if (avio_open(&outFormatContext, in_filename, AVIO_FLAG_WRITE) < 0) {
        fprintf(stderr, "convert(): cannot open out file\n");
        return false;
    }

    

    while (1)
    {
        AVStream* in_stream, * out_stream;
        ret = av_read_frame(ifmt_ctx, pkt);

        if (ret < 0)
        {
            break;
        }

        in_stream = ifmt_ctx->streams[pkt->stream_index];
        if (pkt->stream_index >= ifmt_ctx->nb_streams ||
            stream_mapping[pkt->stream_index] < 0) {
            av_packet_unref(pkt);
            continue;
        }

        pkt->stream_index = stream_mapping[pkt->stream_index];

        AVRational default_timebase;
        default_timebase.num = 1;
        default_timebase.den = AV_TIME_BASE;

        int64_t starttime_int64 = av_rescale_q((int64_t)(10 * AV_TIME_BASE), default_timebase, in_stream->time_base);
        int64_t endtime_int64 = av_rescale_q((int64_t)(20 * AV_TIME_BASE), default_timebase, in_stream->time_base);

        if (avformat_seek_file(ifmt_ctx, streamVideoIndex, INT64_MIN, starttime_int64, INT64_MAX, 0) < 0) {
            return false;
        }
    }

    /*
    while (1)
    {
        AVStream* in_stream, * out_stream;
        ret = av_read_frame(ifmt_ctx, pkt);

        if (ret < 0)
        {
            break;
        }

        in_stream = ifmt_ctx->streams[pkt->stream_index];
        if (pkt->stream_index >= ifmt_ctx->nb_streams ||
            stream_mapping[pkt->stream_index] < 0) {
            av_packet_unref(pkt);
            continue;
        }

        pkt->stream_index = stream_mapping[pkt->stream_index];
        out_stream = ofmt_ctx->streams[pkt->stream_index];
        out_stream->duration = 20;

        pkt->pts;

        if (dts_start_from[pkt->stream_index] == 0)
        {
            dts_start_from[pkt->stream_index] = pkt->dts;
        }
        if (pts_start_from[pkt->stream_index] == 0)
        {
            pts_start_from[pkt->stream_index] = pkt->pts;
        }
        pkt->pts = av_rescale_q_rnd(pkt->pts - pts_start_from[pkt->stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
        pkt->dts = av_rescale_q_rnd(pkt->dts - pts_start_from[pkt->stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
        if (pkt->pts < 0) {
            pkt->pts = 0;
        }
        if (pkt->dts < 0) {
            pkt->dts = 0;
        }
        pkt->duration = (int)av_rescale_q((int64_t)pkt->duration, in_stream->time_base, out_stream->time_base);

        //av_packet_rescale_ts(pkt, in_stream->time_base, out_stream->time_base);
        //pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
        //pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
        //pkt->duration = av_rescale_q(pkt->duration, (AVRational) { 1, 60 }, out_stream->time_base);
        pkt->pos = -1;

        ret = av_interleaved_write_frame(ofmt_ctx, pkt);
        if (ret < 0)
        {
            return false;
        }
    }
    */

    

    

    av_write_trailer(ofmt_ctx);

    av_packet_free(&pkt);
    avformat_close_input(&ifmt_ctx);

    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
    {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);

    return true;
};

bool ffmpegConvertAndCut(char *file, float startTime, float endTime)
{
    AVFrame* frame;
    AVPacket inPacket, outPacket;
    AVIOContext* outFormatContext;

    if (avio_open(&outFormatContext, file, AVIO_FLAG_WRITE) < 0) {
        fprintf(stderr, "convert(): cannot open out file\n");
        return false;
    }

    AVRational default_timebase;
    default_timebase.num = 1;
    default_timebase.den = AV_TIME_BASE;

    //int64_t starttime_int64 = av_rescale_q((int64_t)(startTime * AV_TIME_BASE), default_timebase, inVideoStream->time_base);
    //int64_t endtime_int64 = av_rescale_q((int64_t)(endTime * AV_TIME_BASE), default_timebase, inVideoStream->time_base);

    return true;
};

int main()
{
    file_handling files;

    files.CheckForLogFiles();
    std::vector<std::thread> processingThreads;

    //processes through available files and adds log info
    for (int i = 0; i < files.logFiles.size(); i++)
    {
        processingThreads.push_back(std::thread(&ProcessLogs, &files, files.logFiles[i]));
        Sleep(20);
    }
    for (auto& threads : processingThreads)
    {
        threads.join();
    }

    Encounters_Total fights(files.contents);

    
    //system call for if I want to be lazy and not actually implement the ffmpeg library cause I know this works
    //system("ffmpeg -ss 00:20:00 -to 00:30:00 -i X:\\2024-07-09_19-57-00.mkv -c copy X:\\proof.webm");

    ffmpegSample();
    //std::string fileName = "X:\\2024-07-12_02-41-37.mkv";
    //char* cstr = fileName.data();
    //float start = 10;
    //float end = 20;
    //ffmpegConvertAndCut(cstr, start, end);


    
    

    return 0;


};