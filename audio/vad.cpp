#include "vad.h"
#include "silero_vad.h"
#include <vector>
#include <iostream>

VAD::VAD(const std::wstring& model_path, int sample_rate, float threshold) {
    silero = new SileroVAD(model_path, sample_rate, threshold);
}

VAD::~VAD() {
    if (silero) delete silero;
}

bool VAD::isSpeech(const int16_t* pcm, int length, int sample_rate) {
    if (!silero) return false;

    // Silero VAD expects float audio in [-1, 1] range
    std::vector<float> float_pcm(length);
    for (int i = 0; i < length; ++i) {
        float_pcm[i] = static_cast<float>(pcm[i]) / 32768.0f;
    }

    // Silero usually works best with 512 samples for 16kHz
    // If length is different, SileroVAD::getSpeechProb handles resizing/padding
    return silero->isSpeech(float_pcm);
}
