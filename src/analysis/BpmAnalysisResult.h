#pragma once

#include <string>

struct BpmAnalysisResult {
    bool success = false;
    double bpm = 0.0;
    std::string message;
};