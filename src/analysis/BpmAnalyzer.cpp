#include "BpmAnalyzer.h"

#include <stdexcept>
#include <vector>

extern "C"{
    #include <aubio/aubio.h>
#include "BpmAnalyzer.h"
}

 bool BpmAnalyzer::validateInput(const DecodedBuffer& decodedBuffer){
    return decodedBuffer.sampleRate > 0
        && decodedBuffer.channels > 0
        && !decodedBuffer.samples.empty();
 }

 std::vector<float> BpmAnalyzer::downmixToMono(const DecodedBuffer& decodedBuffer){
    std::vector<float> monoSamples;
    monoSamples.reserve(decodedBuffer.frameCount());

    const std::size_t totalFrames = decodedBuffer.frameCount();
    const int channels = decodedBuffer.channels;

    for(std::size_t frame = 0; frame < totalFrames; ++frame)
    {
        float mono = 0.0f;
        
        for (int ch = 0; ch < channels; ++ch)
        {
            mono += decodedBuffer.samples[frame * channels + ch];
        }

        mono/= static_cast<float>(channels);
        monoSamples.push_back(mono);
        
    }
    return monoSamples;
    
 }

 /*
 std::vector<double> BpmAnalyzer::collectBeatTimes(const std::vector<float> &monoSamples, unsigned int sampleRate) {
    const uint_t winSize = 1024;
    const uint_t hopSize = 512;
    
    aubio_tempo_t*
    
    
    
    return std::vector<double>();
}
*/