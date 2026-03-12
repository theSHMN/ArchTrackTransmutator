#pragma once

#include "FileInfo.h"
#include <string>
#include <cstdint>

struct AVFormatContext;
struct AVStream;
struct AVCodecParameters;

class FfmpegInspector {
public:
    static FileInfo inspect(const std::string& filePath);
    
private:
    static FileInfo buildFileInfo(
        const std::string& filePath,
        AVFormatContext *formatCtx,
        AVStream* audioStream,
        AVCodecParameters *codecParams
    );

    static int64_t getDurationMs(AVFormatContext *formatCtx, AVStream* audioStream);
    static std::string getCodecName(AVCodecParameters *codecParams);


};