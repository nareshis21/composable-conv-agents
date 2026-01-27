#ifndef VAD_H
#define VAD_H

#include <cstdint>
#include <string>
#include <vector>

// Forward declaration
class SileroVAD;

class VAD {
public:
    SileroVAD* silero;

    VAD(const std::wstring& model_path, int sample_rate = 16000, float threshold = 0.5f);
    ~VAD();

    bool isSpeech(const int16_t* pcm, int length, int sample_rate = 16000);
};

#endif // VAD_H
