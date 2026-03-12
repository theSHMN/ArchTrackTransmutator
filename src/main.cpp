#include <cmath>
#include <iostream>

extern "C" {
#include <aubio/aubio.h>
}

int main() {
    constexpr uint_t sampleRate = 44100;
    constexpr uint_t winSize = 2048;
    constexpr uint_t hopSize = 256;
    constexpr float testFreq = 440.0f;

    fvec_t* input = new_fvec(hopSize);
    fvec_t* output = new_fvec(1);
    aubio_pitch_t* pitch = new_aubio_pitch("yinfft", winSize, hopSize, sampleRate);

    if (!input || !output || !pitch) {
        std::cerr << "aubio setup failed\n";
        if (pitch) del_aubio_pitch(pitch);
        if (output) del_fvec(output);
        if (input) del_fvec(input);
        return 1;
    }

    aubio_pitch_set_unit(pitch, "Hz");
    aubio_pitch_set_silence(pitch, -90.0f);
    aubio_pitch_set_tolerance(pitch, 0.8f);

    float phase = 0.0f;
    const float phaseInc =
        2.0f * 3.14159265358979323846f * testFreq / static_cast<float>(sampleRate);

    float detectedHz = 0.0f;
    float confidence = 0.0f;

    for (int block = 0; block < 60; ++block) {
        for (uint_t i = 0; i < hopSize; ++i) {
            fvec_set_sample(input, 0.8f * std::sin(phase), i);
            phase += phaseInc;
            if (phase > 2.0f * 3.14159265358979323846f) {
                phase -= 2.0f * 3.14159265358979323846f;
            }
        }

        aubio_pitch_do(pitch, input, output);
        detectedHz = fvec_get_sample(output, 0);
        confidence = aubio_pitch_get_confidence(pitch);
    }

    std::cout << "Expected:   " << testFreq << " Hz\n";
    std::cout << "Detected:   " << detectedHz << " Hz\n";
    std::cout << "Confidence: " << confidence << "\n";

    del_aubio_pitch(pitch);
    del_fvec(output);
    del_fvec(input);
    aubio_cleanup();

    return 0;
}