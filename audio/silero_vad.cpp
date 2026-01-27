#include "silero_vad.h"
#include <iostream>
#include <algorithm>
#include <cstring>

SileroVAD::SileroVAD(const std::wstring& model_path, int sr_val, float thresh)
    : env(ORT_LOGGING_LEVEL_WARNING, "SileroVAD"),
      memory_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU)),
      threshold(thresh),
      sample_rate(sr_val) {
    
    session_options.SetIntraOpNumThreads(1);
    session_options.SetInterOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    try {
        session = std::make_unique<Ort::Session>(env, model_path.c_str(), session_options);
    } catch (const std::exception& e) {
        std::cerr << "[SileroVAD] Failed to load model: " << e.what() << std::endl;
    }

    // Initialize state
    _state.assign(2 * 1 * 128, 0.0f);
    _sr.assign(1, (int64_t)sample_rate);
}

SileroVAD::~SileroVAD() {}

float SileroVAD::getSpeechProb(const std::vector<float>& chunk) {
    if (!session) return 0.0f;

    // Silero expects exactly 512 samples for 16kHz
    std::vector<float> input_data = chunk;
    if (input_data.size() < 512) input_data.resize(512, 0.0f);

    std::vector<Ort::Value> inputs;
    inputs.push_back(Ort::Value::CreateTensor<float>(memory_info, input_data.data(), 512, input_dims, 2));
    inputs.push_back(Ort::Value::CreateTensor<float>(memory_info, _state.data(), _state.size(), state_dims, 3));
    inputs.push_back(Ort::Value::CreateTensor<int64_t>(memory_info, _sr.data(), 1, sr_dims, 1));

    auto outputs = session->Run(Ort::RunOptions{nullptr}, input_node_names.data(), inputs.data(), inputs.size(), output_node_names.data(), output_node_names.size());

    float prob = outputs[0].GetTensorMutableData<float>()[0];
    
    // Update state for next chunk
    float* next_state = outputs[1].GetTensorMutableData<float>();
    std::memcpy(_state.data(), next_state, _state.size() * sizeof(float));

    return prob;
}

bool SileroVAD::isSpeech(const std::vector<float>& chunk) {
    return getSpeechProb(chunk) >= threshold;
}
