#include "FfmpegHelpers.h"

#include <stdexcept>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

AVFormatContext* FfmpegHelpers::openInput(const std::string& filePath) {
    AVFormatContext* formatCtx = nullptr;

    if (avformat_open_input(&formatCtx, filePath.c_str(), nullptr, nullptr) < 0) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        avformat_close_input(&formatCtx);
        throw std::runtime_error("Failed to read stream info: " + filePath);
    }

    return formatCtx;
}

int FfmpegHelpers::findAudioStreamIndex(AVFormatContext* formatCtx) {
    for (unsigned int i = 0; i < formatCtx->nb_streams; ++i) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            return static_cast<int>(i);
        }
    }

    throw std::runtime_error("No audio stream found in file");
}