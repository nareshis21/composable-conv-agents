#ifndef MIC_STREAM_H
#define MIC_STREAM_H

#include <portaudio.h>
#include <vector>
#include <functional>
#include <atomic>

class MicrophoneStream {
public:
    int sample_rate;
    int frames_per_buffer;
    PaStream *stream;
    std::atomic<bool> is_running;
    std::function<void(const std::vector<int16_t>&)> audio_callback;

    MicrophoneStream(int rate=16000, int block=480); // 30ms block at 16k
    ~MicrophoneStream();

    void start_stream(std::function<void(const std::vector<int16_t>&)> callback);
    void stop_stream();
    
private:
    static int paCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData);
};

#endif // MIC_STREAM_H
