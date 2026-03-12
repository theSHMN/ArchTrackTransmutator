#pragma once

#include "DecodedBuffer.h"
#include <string>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodecParameters;
struct AVFrame;
struct AVPacket;
struct SwrContext;
struct AVChannelLayout;

class FfmpegDecoder {
public:
    static DecodedBuffer decodeFile(const std::string &filePath);

private:
    static AVCodecContext *createCodecContext(AVCodecParameters* codecParms);
    static SwrContext *createResampler(
        AVCodecContext *codecCtx,
        AVChannelLayout *outChLayout,
        int outSampleRate);
    
    static void decodePackets(
        AVFormatContext *formatCtx,
        AVCodecContext *codecCtx,
        SwrContext *swrCtx,
        int audioStreamIdx,
        const AVChannelLayout& outChLayout,
        int outSampleRate,
        DecodedBuffer& decodedBuffer);

    static void flushDecoder(
        AVCodecContext* condecCtx,
        SwrContext *swrCtx,
        const AVChannelLayout& outChLayout,
        int outSampleRate,
        DecodedBuffer& decodedBuffer,
        AVFrame *frame);

    static void appendFrameSamples
        (AVCodecContext* codecCtx,
        SwrContext *swrCtx,
        const AVChannelLayout & outChLayout,
        int outSampleRate,
        AVFrame *frame,
        DecodedBuffer& decodedBuffer);
    
    static void releaseDecodeResources(
        AVFormatContext*& formatCtx,
        AVCodecContext*& codecCtx,
        SwrContext*& swrCtx,
        AVPacket*& packet,
        AVFrame*& frame,
        AVChannelLayout& outChLayout,
        bool& outChLayoutInit
);
};