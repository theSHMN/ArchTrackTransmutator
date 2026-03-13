#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "audioio/FfmpegDecoder.h"
#include "audioio/FfmpegInspector.h"

void printSeparator() {
    std::cout << "----------------------------------------\n";
}

void runInspectTests(const std::vector<std::string>& testFiles) {
    std::cout << "\n=== INSPECT TESTS ===\n";

    for (const std::string& path : testFiles) {
        printSeparator();
        std::cout << "Inspecting: " << path << '\n';

        try {
            FileInfo info = FfmpegInspector::inspect(path);

            std::cout << "File path:   " << info.filePath << '\n';
            std::cout << "File name:   " << info.fileName << '\n';
            std::cout << "Format:      " << info.formatName << '\n';
            std::cout << "Codec:       " << info.codecName << '\n';
            std::cout << "Sample rate: " << info.sampleRate << '\n';
            std::cout << "Channels:    " << info.channels << '\n';
            std::cout << "Bit rate:    " << info.bitRate << '\n';
            std::cout << "Duration ms: " << info.durationMs << '\n';
        } catch (const std::exception& e) {
            std::cerr << "Inspect failed: " << e.what() << '\n';
        }
    }

    printSeparator();
}

void runDecodeTests(const std::vector<std::string>& testFiles) {
    std::cout << "\n=== DECODE TESTS ===\n";

    for (const std::string& path : testFiles) {
        printSeparator();
        std::cout << "Decoding: " << path << '\n';

        try {
            DecodedBuffer decoded = FfmpegDecoder::decodeFile(path);

            std::cout << "Decoded sample rate: " << decoded.sampleRate << '\n';
            std::cout << "Decoded channels:    " << decoded.channels << '\n';
            std::cout << "Decoded samples:     " << decoded.samples.size() << '\n';
            std::cout << "Decoded frames:      " << decoded.frameCount() << '\n';

            if (!decoded.samples.empty()) {
                std::cout << "First samples:       ";

                const std::size_t count = std::min<std::size_t>(8, decoded.samples.size());
                for (std::size_t i = 0; i < count; ++i) {
                    std::cout << decoded.samples[i];
                    if (i + 1 < count) {
                        std::cout << ", ";
                    }
                }
                std::cout << '\n';
            }
        } catch (const std::exception& e) {
            std::cerr << "Decode failed: " << e.what() << '\n';
        }
    }

    printSeparator();
}

int main() {
    const std::string filePath =
        "/Users/sahandghesmati/VSCode/ArchTrackTransmutator/assets/NEVRMIND - Say My Name.wav";
    const std::string filePath1 =
        "/Users/sahandghesmati/VSCode/ArchTrackTransmutator/assets/Let Me Fly.mp3";
    const std::string filePath2 =
        "/Users/sahandghesmati/VSCode/ArchTrackTransmutator/assets/01. Afu-Ra - Asun-the Message.aif";

    std::vector<std::string> testFiles = {
        filePath,
        filePath1,
        filePath2
    };

    runInspectTests(testFiles);
    runDecodeTests(testFiles);

    return 0;
}