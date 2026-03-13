#include "FfmpegDecoder.h"
#include "FfmpegHelpers.h"

#include <stdexcept>
#include <string>

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

DecodedBuffer FfmpegDecoder::decodeFile(const std::string& filePath) {
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    SwrContext* swrCtx = nullptr;

    AVChannelLayout outChLayout{};
    bool outChLayoutInit = false;

    try {
        formatCtx = FfmpegHelpers::openInput(filePath);

        const int audioStreamIdx = FfmpegHelpers::findAudioStreamIndex(formatCtx);
        AVStream* audioStream = formatCtx->streams[audioStreamIdx];
        AVCodecParameters* codecParams = audioStream->codecpar;

        codecCtx = createCodecContext(codecParams);

        throwOnError(
            av_channel_layout_copy(&outChLayout, &codecCtx->ch_layout),
            "Failed to copy output channel layout"
        );
        outChLayoutInit = true;

        const int outSampleRate = codecCtx->sample_rate;
        swrCtx = createResampler(codecCtx, &outChLayout, outSampleRate);

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
            decodedBuffer
        );

        releaseDecodeResources(
            formatCtx,
            codecCtx,
            swrCtx,
            outChLayout,
            outChLayoutInit
        );

        return decodedBuffer;
    } catch (...) {
        releaseDecodeResources(
            formatCtx,
            codecCtx,
            swrCtx,
            outChLayout,
            outChLayoutInit
        );
        throw;
    }
}

AVCodecContext* FfmpegDecoder::createCodecContext(AVCodecParameters* codecParams) {
    const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        throw std::runtime_error("Failed to find decoder");
    }

    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        throw std::runtime_error("Failed to allocate codec context");
    }

    try {
        throwOnError(
            avcodec_parameters_to_context(codecCtx, codecParams),
            "Failed to copy codec parameters"
        );

        throwOnError(
            avcodec_open2(codecCtx, codec, nullptr),
            "Failed to open codec"
        );

        return codecCtx;
    } catch (...) {
        avcodec_free_context(&codecCtx);
        throw;
    }
}

SwrContext* FfmpegDecoder::createResampler(
    AVCodecContext* codecCtx,
    AVChannelLayout* outChLayout,
    int outSampleRate
) {
    SwrContext* swrCtx = nullptr;

    throwOnError(
        swr_alloc_set_opts2(
            &swrCtx,
            outChLayout,
            AV_SAMPLE_FMT_FLT,
            outSampleRate,
            &codecCtx->ch_layout,
            codecCtx->sample_fmt,
            codecCtx->sample_rate,
            0,
            nullptr
        ),
        "Failed to allocate resampler"
    );

    try {
        throwOnError(
            swr_init(swrCtx),
            "Failed to initialize resampler"
        );

        return swrCtx;
    } catch (...) {
        swr_free(&swrCtx);
        throw;
    }
}

void FfmpegDecoder::decodePackets(
    AVFormatContext* formatCtx,
    AVCodecContext* codecCtx,
    SwrContext* swrCtx,
    int audioStreamIdx,
    const AVChannelLayout& outChLayout,
    int outSampleRate,
    DecodedBuffer& decodedBuffer
) {
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    if (!packet || !frame) {
        av_packet_free(&packet);
        av_frame_free(&frame);
        throw std::runtime_error("Failed to allocate packet or frame");
    }

    try {
        while (av_read_frame(formatCtx, packet) >= 0) {
            if (packet->stream_index == audioStreamIdx) {
                throwOnError(
                    avcodec_send_packet(codecCtx, packet),
                    "Failed to send packet to decoder"
                );

                while (true) {
                    const int result = avcodec_receive_frame(codecCtx, frame);

                    if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                        break;
                    }

                    throwOnError(result, "Failed to receive decoded frame");

                    appendFrameSamples(
                        codecCtx,
                        swrCtx,
                        outChLayout,
                        outSampleRate,
                        frame,
                        decodedBuffer
                    );

                    av_frame_unref(frame);
                }
            }

            av_packet_unref(packet);
        }

        av_frame_free(&frame);
        av_packet_free(&packet);
    } catch (...) {
        av_frame_free(&frame);
        av_packet_free(&packet);
        throw;
    }
}

void FfmpegDecoder::flushDecoder(
    AVCodecContext* codecCtx,
    SwrContext* swrCtx,
    const AVChannelLayout& outChLayout,
    int outSampleRate,
    DecodedBuffer& decodedBuffer
) {
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        throw std::runtime_error("Failed to allocate flush frame");
    }

    try {
        throwOnError(
            avcodec_send_packet(codecCtx, nullptr),
            "Failed to flush decoder"
        );

        while (true) {
            const int result = avcodec_receive_frame(codecCtx, frame);

            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                break;
            }

            throwOnError(result, "Failed to receive flushed frame");

            appendFrameSamples(
                codecCtx,
                swrCtx,
                outChLayout,
                outSampleRate,
                frame,
                decodedBuffer
            );

            av_frame_unref(frame);
        }

        av_frame_free(&frame);
    } catch (...) {
        av_frame_free(&frame);
        throw;
    }
}

void FfmpegDecoder::appendFrameSamples(
    AVCodecContext* codecCtx,
    SwrContext* swrCtx,
    const AVChannelLayout& outChLayout,
    int outSampleRate,
    AVFrame* frame,
    DecodedBuffer& decodedBuffer
) {
    const int outChannels = outChLayout.nb_channels;

    const int maxOutSamples = av_rescale_rnd(
        swr_get_delay(swrCtx, codecCtx->sample_rate) + frame->nb_samples,
        outSampleRate,
        codecCtx->sample_rate,
        AV_ROUND_UP
    );

    uint8_t** convertedData = nullptr;
    int convertedLineSize = 0;

    throwOnError(
        av_samples_alloc_array_and_samples(
            &convertedData,
            &convertedLineSize,
            outChannels,
            maxOutSamples,
            AV_SAMPLE_FMT_FLT,
            0
        ),
        "Failed to allocate converted sample buffer"
    );

    try {
        const int convertedSamples = swr_convert(
            swrCtx,
            convertedData,
            maxOutSamples,
            const_cast<const uint8_t**>(frame->extended_data),
            frame->nb_samples
        );

        throwOnError(convertedSamples, "Failed to convert samples");

        const int totalSamples = convertedSamples * outChannels;
        float* floatData = reinterpret_cast<float*>(convertedData[0]);

        decodedBuffer.samples.insert(
            decodedBuffer.samples.end(),
            floatData,
            floatData + totalSamples
        );

        av_freep(&convertedData[0]);
        av_freep(&convertedData);
    } catch (...) {
        if (convertedData) {
            av_freep(&convertedData[0]);
            av_freep(&convertedData);
        }
        throw;
    }
}

void FfmpegDecoder::releaseDecodeResources(
    AVFormatContext*& formatCtx,
    AVCodecContext*& codecCtx,
    SwrContext*& swrCtx,
    AVChannelLayout& outChLayout,
    bool& outChLayoutInit
) {
    swr_free(&swrCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);

    if (outChLayoutInit) {
        av_channel_layout_uninit(&outChLayout);
        outChLayoutInit = false;
    }
}