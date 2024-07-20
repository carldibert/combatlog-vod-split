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
    int operationResult;

    AVPacket* avPacket = NULL;
    AVFormatContext* avInputFormatContext = NULL;
    AVFormatContext* avOutputFormatContext = NULL;

    avPacket = av_packet_alloc();
    if (!avPacket) {
        //qCritical("Failed to allocate AVPacket.");
        return false;
    }

    try {
        operationResult = avformat_open_input(&avInputFormatContext, in_filename, 0, 0);
        if (operationResult < 0) {
            //throw std::runtime_error(QString("Failed to open the input file '%1'.").arg(inputFilePath).toStdString().c_str());
        }

        operationResult = avformat_find_stream_info(avInputFormatContext, 0);
        if (operationResult < 0) {
            //throw std::runtime_error(QString("Failed to retrieve the input stream information.").toStdString().c_str());
        }

        avformat_alloc_output_context2(&avOutputFormatContext, NULL, NULL, out_filename);
        if (!avOutputFormatContext) {
            operationResult = AVERROR_UNKNOWN;
            //throw std::runtime_error(QString("Failed to create the output context.").toStdString().c_str());
        }

        int streamIndex = 0;
        int* streamMapping = new int[avInputFormatContext->nb_streams];
        int* streamRescaledStartSeconds = new int[avInputFormatContext->nb_streams];
        int* streamRescaledEndSeconds = new int[avInputFormatContext->nb_streams];

        // Copy streams from the input file to the output file.
        for (int i = 0; i < avInputFormatContext->nb_streams; i++) {
            AVStream* outStream;
            AVStream* inStream = avInputFormatContext->streams[i];

            streamRescaledStartSeconds[i] = av_rescale_q(from_seconds * AV_TIME_BASE, AV_TIME_BASE_Q, inStream->time_base);
            streamRescaledEndSeconds[i] = av_rescale_q(end_seconds * AV_TIME_BASE, AV_TIME_BASE_Q, inStream->time_base);

            if (inStream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
                inStream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                inStream->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                streamMapping[i] = -1;
                continue;
            }

            streamMapping[i] = streamIndex++;

            outStream = avformat_new_stream(avOutputFormatContext, NULL);
            if (!outStream) {
                operationResult = AVERROR_UNKNOWN;
                //throw std::runtime_error(QString("Failed to allocate the output stream.").toStdString().c_str());
            }

            operationResult = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
            if (operationResult < 0) {
                //throw std::runtime_error(
                    //QString("Failed to copy codec parameters from input stream to output stream.").toStdString().c_str());
            }
            outStream->codecpar->codec_tag = 0;
        }

        if (!(avOutputFormatContext->oformat->flags & AVFMT_NOFILE)) {
            operationResult = avio_open(&avOutputFormatContext->pb, out_filename, AVIO_FLAG_WRITE);
            if (operationResult < 0) {
                //throw std::runtime_error(
                    //QString("Failed to open the output file '%1'.").arg(outputFilePath).toStdString().c_str());
            }
        }

        operationResult = avformat_write_header(avOutputFormatContext, NULL);
        if (operationResult < 0) {
            //throw std::runtime_error(QString("Error occurred when opening output file.").toStdString().c_str());
        }

        operationResult = avformat_seek_file(avInputFormatContext, -1, INT64_MIN, from_seconds * AV_TIME_BASE,
            from_seconds * AV_TIME_BASE, 0);
        if (operationResult < 0) {
            //throw std::runtime_error(
                //QString("Failed to seek the input file to the targeted start position.").toStdString().c_str());
        }

        while (true) {
            operationResult = av_read_frame(avInputFormatContext, avPacket);
            if (operationResult < 0) break;

            // Skip packets from unknown streams and packets after the end cut position.
            if (avPacket->stream_index >= avInputFormatContext->nb_streams || streamMapping[avPacket->stream_index] < 0 ||
                avPacket->pts > streamRescaledEndSeconds[avPacket->stream_index]) {
                av_packet_unref(avPacket);
                continue;
            }

            avPacket->stream_index = streamMapping[avPacket->stream_index];
            //logPacket(avInputFormatContext, avPacket, "in");

            // Shift the packet to its new position by subtracting the rescaled start seconds.
            avPacket->pts -= streamRescaledStartSeconds[avPacket->stream_index];
            avPacket->dts -= streamRescaledStartSeconds[avPacket->stream_index];

            av_packet_rescale_ts(avPacket, avInputFormatContext->streams[avPacket->stream_index]->time_base,
                avOutputFormatContext->streams[avPacket->stream_index]->time_base);
            avPacket->pos = -1;
            //logPacket(avOutputFormatContext, avPacket, "out");

            operationResult = av_interleaved_write_frame(avOutputFormatContext, avPacket);
            if (operationResult < 0) {
                //throw std::runtime_error(QString("Failed to mux the packet.").toStdString().c_str());
            }
        }

        av_write_trailer(avOutputFormatContext);
    }
    catch (std::runtime_error e) {
        //qCritical("%s", e.what());
    }

    av_packet_free(&avPacket);

    avformat_close_input(&avInputFormatContext);

    if (avOutputFormatContext && !(avOutputFormatContext->oformat->flags & AVFMT_NOFILE))
        avio_closep(&avOutputFormatContext->pb);
    avformat_free_context(avOutputFormatContext);

    if (operationResult < 0 && operationResult != AVERROR_EOF) {
        //qCritical("%s", QString("Error occurred: %1.").arg(av_err2str(operationResult)).toStdString().c_str());
        return false;
    }

    return true;
};

/*
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
    //int* streamRescaledStartSeconds = new int[ifmt_ctx->nb_streams];
    //int* streamRescaledEndSeconds = new int[ifmt_ctx->nb_streams];

    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        AVStream* out_stream;
        AVStream* in_stream = ifmt_ctx->streams[i];
        //streamRescaledStartSeconds[i] = av_rescale_q(from_seconds * AV_TIME_BASE, AV_TIME_BASE_Q, in_stream->time_base);
        //streamRescaledEndSeconds[i] = av_rescale_q(end_seconds * AV_TIME_BASE, AV_TIME_BASE_Q, in_stream->time_base);

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

*/



int64_t ffmpeg::GetDuration(const char* filename)
{
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    avformat_open_input(&pFormatCtx, filename, NULL, NULL);
    int64_t duration = pFormatCtx->duration;
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    return duration;
};