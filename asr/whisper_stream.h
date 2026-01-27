#ifndef WHISPER_STREAM_H
#define WHISPER_STREAM_H

#include "whisper.h" 
#include <string>
#include <vector>
#include <functional>

class WhisperASR {
public:
    struct whisper_context* ctx;
    struct whisper_full_params params;

    WhisperASR(const std::string& model_path);
    ~WhisperASR();

    void transcribe(const std::vector<int16_t>& audio, std::function<void(const std::string&)> callback);
    void transcribe_wav(const std::string& wav_path, std::function<void(const std::string&)> callback);
};


#endif // WHISPER_STREAM_H
