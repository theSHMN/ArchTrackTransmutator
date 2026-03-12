#include <iostream>
#include "audioio/FfmpegInspector.h"

int main() {
    const std::string filePath = "/Users/sahandghesmati/Documents/NEVRMIND - Say My Name.wav";

    try {
        FileInfo info = FfmpegInspector::inspect(filePath);

        std::cout << "File path:   " << info.filePath << '\n';
        std::cout << "File name:   " << info.fileName << '\n';
        std::cout << "Format:      " << info.formatName << '\n';
        std::cout << "Codec:       " << info.codecName << '\n';
        std::cout << "Sample rate: " << info.sampleRate << '\n';
        std::cout << "Channels:    " << info.channels << '\n';
        std::cout << "Bit rate:    " << info.bitRate << '\n';
        std::cout << "Duration ms: " << info.durationMs << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Inspector failed: " << e.what() << '\n';
        return 1;
    }

    return 0;
}