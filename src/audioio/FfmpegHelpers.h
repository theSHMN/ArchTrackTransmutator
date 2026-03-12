#pragma once

#include <string>

struct AVFormatContext;

class FfmpegHelpers {
public:
    static AVFormatContext* openInput(const std::string& filePath);
    static int findAudioStreamIndex(AVFormatContext* formatCtx);
};