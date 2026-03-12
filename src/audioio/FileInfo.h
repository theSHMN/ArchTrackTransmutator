#pragma once

#include <cstdint>
#include <string>


struct FileInfo{
    std::string filePath;
    std::string fileName;
    std::string formatName;
    std::string codecName;

    int sampleRate          = 0;
    int channels            = 0;
    int64_t bitRate         = 0;
    int64_t durationMs      = 0;

    bool isValid() const {
        return !filePath.empty() 
        && sampleRate > 0 
        && channels >0;
    }
};
