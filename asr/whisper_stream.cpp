#include "whisper_stream.h"
#include <iostream>

WhisperASR::WhisperASR(const std::string& model_path) {
    // whisper_init_from_file returns whisper_context*
    ctx = whisper_init_from_file(model_path.c_str());
    if (!ctx) {
        std::cerr << "Failed to initialize whisper context" << std::endl;
    }
}

WhisperASR::~WhisperASR() {
    if (ctx) whisper_free(ctx);
}

void WhisperASR::transcribe(const std::vector<int16_t>& audio, std::function<void(const std::string&)> callback) {
    if (!ctx) return;

    // Convert int16 to float normalized -1..1
    std::vector<float> pcmf32(audio.size());
    for (size_t i = 0; i < audio.size(); i++) {
        pcmf32[i] = (float)audio[i] / 32768.0f;
    }

    params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_progress = false;
    
    if (whisper_full(ctx, params, pcmf32.data(), pcmf32.size()) != 0) {
        std::cerr << "Failed to process audio" << std::endl;
        return;
    }

    const int n_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < n_segments; ++i) {
        const char* text = whisper_full_get_segment_text(ctx, i);
        callback(std::string(text));
    }
}
#include <fstream>

void WhisperASR::transcribe_wav(const std::string& wav_path, std::function<void(const std::string&)> callback) {
    if (!ctx) return;

    std::ifstream file(wav_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[Whisper] Could not open wav file: " << wav_path << std::endl;
        return;
    }

    // Skip WAV header (44 bytes for standard RIFF)
    file.seekg(44, std::ios::beg);

    std::vector<int16_t> audio;
    int16_t sample;
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(int16_t))) {
        audio.push_back(sample);
    }
    
    transcribe(audio, callback);
}
