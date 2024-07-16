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
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavcodec/avcodec.h>
}

//for threading processing of logs 
void ProcessLogs(file_handling* files, std::string logFile)
{
    combat_log log(files->exePath);
    log.ReadFile(logFile);
    files->contents.push_back(log);
}

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


    std::string fileName = "X:\\2024-07-09 19-57-00.mkv";
    video_file video(fileName);
    output_video output;
    output.fileName = "X:\\Smolderon-5.mkv";

    AVOutputFormat* ofmt = NULL;
    AVFormatContext* ifmt_ctx = NULL;
    const char* in_filename, * out_filename;
    in_filename = video.fileName.c_str();
    out_filename = output.fileName.c_str();
    int ret, i;

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        return -1;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        return -1;
    }
    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    int videoStreamIdx = -1;
    int audioStreamIdx = -1;
    AVStream* inVidStream = NULL;
    AVStream* inAudioStream = NULL;
    for (int i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIdx = i;
            inVidStream = ifmt_ctx->streams[i];
            break;
        }
    }
    for (int i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIdx = i;
            inAudioStream = ifmt_ctx->streams[i];
            break;
        }
    }

    const AVOutputFormat* outfmt = NULL;
    AVFormatContext* m_outformat = NULL;
    std::string outfile = output.fileName + "clip_out.webm";
    outfmt = av_guess_format(NULL, outfile.c_str(), NULL);
    if (outfmt == NULL)
    {
        ret = -1;
    }
    else
    {
        m_outformat = avformat_alloc_context();
        if (m_outformat)
        {
            m_outformat->oformat = outfmt;
        }
        else
        {
            return -1;
        }
    }

    const AVCodec* out_vid_codec, * out_aud_codec;
    AVStream* m_out_vid_strm = NULL;
    AVStream* m_out_aud_strm = NULL;
    out_vid_codec = out_aud_codec = NULL;
    if (outfmt->video_codec != AV_CODEC_ID_NONE && inVidStream != NULL)
    {
        out_vid_codec = avcodec_find_encoder(outfmt->video_codec);
        if (NULL == out_vid_codec)
        {
            return -2;
        }
        else
        {
            m_out_vid_strm = inVidStream;
            if (NULL == m_out_vid_strm)
            {
                return -3;
            }
            else
            {
                m_out_vid_strm->codecpar = ifmt_ctx->streams[videoStreamIdx]->codecpar;
                m_out_vid_strm->codecpar->sample_aspect_ratio.den;
                m_out_vid_strm->sample_aspect_ratio.num = inVidStream->codecpar->sample_aspect_ratio.num;
                m_out_vid_strm->codecpar->codec_id = inVidStream->codecpar->codec_id;
                m_out_vid_strm->r_frame_rate = inVidStream->codecpar->framerate;
                m_out_vid_strm->duration = 20;
            }
        }
    }
    if (outfmt->audio_codec != AV_CODEC_ID_NONE && inVidStream != NULL)
    {
        out_aud_codec = avcodec_find_encoder(outfmt->audio_codec);
        if (NULL == out_aud_codec)
        {
            return -2;
        }
        else
        {
            m_out_aud_strm = inAudioStream;
        }
    }
    
    /*
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
    }
    av_dump_format(ifmt_ctx, 0, in_filename, 0);
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
    }
    ofmt == ofmt_ctx->oformat;
    */
    
    

    return 0;


};