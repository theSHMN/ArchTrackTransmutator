#pragma once

#include <vector>

struct DecodedBuffer {
    int sampleRate      = 0;
    int channels        = 0;

    std::vector<float> samples;

    bool isValid() const {
        return sampleRate > 0 
        && channels > 0 
        && !samples.empty();
    }

    std::size_t frameCount() const {
        if(channels <=0) return 0;
        return samples.size() / static_cast<std::size_t>(channels);
    }


};