#ifndef SILERO_VAD_H
#define SILERO_VAD_H

#include <vector>
#include <string>
#include <memory>
#include "onnxruntime_cxx_api.h"

class SileroVAD {
public:
    SileroVAD(const std::wstring& model_path, int sample_rate = 16000, float threshold = 0.5f);
    ~SileroVAD();

    // Returns probability of speech [0.0 - 1.0] for a 32ms chunk (512 samples @ 16kHz)
    float getSpeechProb(const std::vector<float>& chunk);

    // High level: returns true if speech is detected based on threshold
    bool isSpeech(const std::vector<float>& chunk);

private:
    Ort::Env env;
    Ort::SessionOptions session_options;
    std::unique_ptr<Ort::Session> session;
    Ort::MemoryInfo memory_info;

    // Model state (recurrent tokens)
    std::vector<float> _state;
    std::vector<int64_t> _sr;
    
    // Node names
    std::vector<const char*> input_node_names = {"input", "state", "sr"};
    std::vector<const char*> output_node_names = {"output", "stateN"};

    int64_t input_dims[2] = {1, 512};
    int64_t state_dims[3] = {2, 1, 128};
    int64_t sr_dims[1] = {1};

    float threshold;
    int sample_rate;
};

#endif
