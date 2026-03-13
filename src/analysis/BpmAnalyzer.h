#pragma once

#include "BpmAnalysisResult.h"
#include "../audioio/DecodedBuffer.h"
#include <vector>

class BpmAnalyzer {
public:
    static BpmAnalysisResult analyze(const DecodedBuffer& decodedBuffer);

private:
    static bool validateInput(const DecodedBuffer& decodedBuffer);
    static std::vector<float> downmixToMono(const DecodedBuffer& decodedBuffer);
    
    /*
    static std::vector<double> collectBeatTimes(
        const std::vector<float>& monoSamples,
        unsigned int sampleRate
        );
    
    static double calculateBpmFromBeatTimes(const std::vector<double>& beatTimes);
    */
};
