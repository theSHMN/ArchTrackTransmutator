#include "FfmpegInspector.h"

#include <filesystem>
#include <stdexcept>
#include <string>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
} 

FileInfo FfmpegInspector::inspect(const std::string& filePath){
    AVFormatContext* formatCtx = openInput(filePath);

    try {
        const int audioStreamIdx = findAudioStreamIndex(formatCtx);
        AVStream* audioStream = formatCtx->streams[audioStreamIdx];
        AVCodecParameters* codecParams = audioStream->codecpar;

        FileInfo info = buildFileInfo(
            filePath,
            formatCtx,
            audioStream,
            codecParams
        );

        avformat_close_input(&formatCtx);
        return info;
    }
    catch (...)
    {
        avformat_close_input(&formatCtx);
        throw;
    }
}

AVFormatContext* FfmpegInspector::openInput(const std::string& filePath){
    AVFormatContext* formatCtx = nullptr;

    if(avformat_open_input(&formatCtx, filePath.c_str(),nullptr,nullptr) < 0)
    {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    
    if(avformat_find_stream_info(formatCtx,nullptr) < 0)
    {
        avformat_close_input(&formatCtx);
        throw std::runtime_error("Failed to read stream info: "+ filePath);
    }

    return formatCtx;
}

int FfmpegInspector::findAudioStreamIndex(AVFormatContext* formatCtx){
    for(unsigned int i = 0; i < formatCtx->nb_streams;++i)
    {
        if(formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            return static_cast<int>(i);
        }
        
    }

    throw std::runtime_error("No audio stream found in file");
}

int64_t FfmpegInspector::getDurationMs(AVFormatContext *formatCtx, AVStream* audioStream){
    if(audioStream->duration != AV_NOPTS_VALUE) {
        return (audioStream->duration * 1000LL * audioStream->time_base.num) 
        / audioStream->time_base.den;
    }

    if(formatCtx->duration != AV_NOPTS_VALUE) 
    {
        return formatCtx->duration / 1000;
    }

    return 0;
}

std::string FfmpegInspector::getCodecName(AVCodecParameters *codecParams){
    const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    return (codec && codec->name) ? codec->name : "unknown";
}



FileInfo FfmpegInspector::buildFileInfo(
        const std::string& filePath,
        AVFormatContext *formatCtx,
        AVStream* audioStream,
        AVCodecParameters *codecParams) {
            FileInfo info;
            info.filePath = filePath;
            info.fileName = std::filesystem::path(filePath).filename().string();
            info.formatName = (formatCtx->iformat && formatCtx->iformat->name)
                ? formatCtx->iformat->name
                :"unknown";
            info.codecName = getCodecName(codecParams);
            info.sampleRate = codecParams->sample_rate;
            info.channels = codecParams->ch_layout.nb_channels;
            info.bitRate = codecParams->bit_rate;
            info.durationMs = getDurationMs(formatCtx,audioStream);

            return info;
        }