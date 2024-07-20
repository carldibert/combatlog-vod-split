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
}

bool ffmpeg::ProcessFile(const char* in_filename, const char* out_filename, double from_seconds, double end_seconds)
{
    const AVOutputFormat* ofmt = NULL;
    AVFormatContext* ifmt_ctx = NULL, * ofmt_ctx = NULL;
    AVPacket* pkt = NULL;
    int ret, i;
    int stream_index = 0;
    int stream_mapping_size = 0;

    //const char* in_filename, * out_filename;
    //used for testing purposes to hard code some values
    //double from_seconds = 5;
    //double end_seconds = 15;

    //hard coded file paths for testing
    //in_filename = "X:\\input.mkv";
    //out_filename = "X:\\output.mkv";

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

    int64_t* dts_start_from = new int64_t[ifmt_ctx->nb_streams];
    int64_t* pts_start_from = new int64_t[ifmt_ctx->nb_streams];
    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);

    while (1)
    {
        AVStream* in_stream, * out_stream;

        ret = av_read_frame(ifmt_ctx, pkt);
        if (ret < 0)
        {
            break;
        }

        in_stream = ifmt_ctx->streams[pkt->stream_index];
        out_stream = ofmt_ctx->streams[pkt->stream_index];

        if (av_q2d(in_stream->time_base) * pkt->pts > end_seconds)
        {
            av_packet_unref(pkt);
            break;
        }

        if (dts_start_from[pkt->stream_index] == 0)
        {
            dts_start_from[pkt->stream_index] = pkt->dts;
        }
        if (pts_start_from[pkt->stream_index] == 0)
        {
            pts_start_from[pkt->stream_index] = pkt->pts;
        }

        pkt->pts = av_rescale_q_rnd(pkt->pts - pts_start_from[pkt->stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_PASS_MINMAX);
        pkt->dts = av_rescale_q_rnd(pkt->dts - dts_start_from[pkt->stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_PASS_MINMAX);
        if (pkt->pts < 0)
        {
            pkt->pts = 0;
        }
        if (pkt->dts < 0)
        {
            pkt->dts = 0;
        }
        pkt->duration = (int)av_rescale_q((int64_t)pkt->duration, in_stream->time_base, out_stream->time_base);
        pkt->pos = -1;

        ret = av_interleaved_write_frame(ofmt_ctx, pkt);
        if (ret < 0)
        {
            std::cout << "Error muxing packet" << std::endl;
            break;
        }
        av_packet_unref(pkt);
    }
    delete[] dts_start_from;
    delete[] pts_start_from;

    av_write_trailer(ofmt_ctx);

    av_packet_free(&pkt);
    avformat_close_input(&ifmt_ctx);

    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
    {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);

    return true;
}

int64_t ffmpeg::GetDuration(const char* filename)
{
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    avformat_open_input(&pFormatCtx, filename, NULL, NULL);
    int64_t duration = pFormatCtx->duration;
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    return duration;
};