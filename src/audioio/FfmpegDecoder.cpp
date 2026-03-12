#pragma once

#include "FfmpegDecoder.h"
#include "FfmpegHelpers.h"

#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libavutil/error.h>
}

namespace {
std::string makeFfmpegErrorString(int errorCode) {
    char errorBuffer[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(errorCode, errorBuffer, sizeof(errorBuffer));
    return std::string(errorBuffer);
}

void throwOnError(int errorCode, const std::string& message) {
    if (errorCode < 0) {
        throw std::runtime_error(message + ": " + makeFfmpegErrorString(errorCode));
    }
}
}

DecodedBuffer FfmpegDecoder::decodeFile(const std::string& filePath){
    AVFormatContext *formatCtx  = nullptr;
    AVCodecContext *codecCtx    = nullptr;
    SwrContext *swrCtx          = nullptr;
    AVPacket* packet            = nullptr;
    AVFrame* frame              = nullptr;

    AVChannelLayout outChLayout{};
    bool outChLayoutInit = false;

    try {
        formatCtx = FfmpegHelpers::openInput(filePath);
    
        const int audioStreamIdx = FfmpegHelpers::findAudioStreamIndex(formatCtx);
        AVStream* audioStream = formatCtx->streams[audioStreamIdx];
        AVCodecParameters* codecParams = audioStream->codecpar;

        codecCtx = createCodecContext(codecParams);

        throwOnError(
            av_channel_layout_copy(&outChLayout,&codecCtx->ch_layout),
            "Failed to copy output channel layout"
        );

        outChLayoutInit = true;
        const int outSampleRate = codecCtx->sample_rate;
        swrCtx = createResampler(codecCtx,&outChLayout,outSampleRate);

        packet = av_packet_alloc();
        frame = av_frame_alloc();
        if(!packet || !frame)
        {
            throw std::runtime_error("Failed to allocate packet or frame");
        }

        DecodedBuffer decodedBuffer;
        decodedBuffer.sampleRate = outSampleRate;
        decodedBuffer.channels = outChLayout.nb_channels;

        decodePackets(
            formatCtx,
            codecCtx,
            swrCtx,
            audioStreamIdx,
            outChLayout,
            outSampleRate,
            decodedBuffer
        );

        flushDecoder(
            codecCtx,
            swrCtx,
            outChLayout,
            outSampleRate,
            decodedBuffer,
            frame
        );

        releaseDecodeResources(formatCtx, codecCtx, swrCtx, packet, frame, outChLayout, outChLayoutInit);

        return decodedBuffer;
    }
    catch (...)
    {
        releaseDecodeResources(formatCtx, codecCtx, swrCtx, packet, frame, outChLayout, outChLayoutInit);

        throw;
    }
}

static void releaseDecodeResources(
    AVFormatContext*& formatCtx,
    AVCodecContext*& codecCtx,
    SwrContext*& swrCtx,
    AVPacket*& packet,
    AVFrame*& frame,
    AVChannelLayout& outChLayout,
    bool& outChLayoutInit
) {
    av_frame_free(&frame);
    av_packet_free(&packet);
    swr_free(&swrCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);

    if (outChLayoutInit) {
        av_channel_layout_uninit(&outChLayout);
        outChLayoutInit = false;
    }
}

