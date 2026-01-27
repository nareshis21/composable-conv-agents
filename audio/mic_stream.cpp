#include "mic_stream.h"
#include <iostream>

MicrophoneStream::MicrophoneStream(int rate, int block) 
    : sample_rate(rate), frames_per_buffer(block), stream(nullptr), is_running(false) {
    Pa_Initialize();
}

MicrophoneStream::~MicrophoneStream() {
    stop_stream();
    Pa_Terminate();
}

int MicrophoneStream::paCallback(const void *inputBuffer, void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* timeInfo,
                                 PaStreamCallbackFlags statusFlags,
                                 void *userData) {
    MicrophoneStream* self = static_cast<MicrophoneStream*>(userData);
    if (inputBuffer && self->audio_callback) {
        const int16_t* in = static_cast<const int16_t*>(inputBuffer);
        std::vector<int16_t> data(in, in + framesPerBuffer);
        self->audio_callback(data);
    }
    return paContinue;
}

void MicrophoneStream::start_stream(std::function<void(const std::vector<int16_t>&)> callback) {
    audio_callback = callback;
    
    Pa_OpenDefaultStream(&stream,
                         1,             // 1 Input Channel
                         0,             // 0 Output Channels
                         paInt16,       // Sample format
                         sample_rate,
                         frames_per_buffer,
                         &MicrophoneStream::paCallback,
                         this);
    
    Pa_StartStream(stream);
    is_running = true;
    
    // In a real app, you might want this to run in a separate thread or block here 
    // depending on the architecture. For this callback-based approach, it runs in PA thread.
}

void MicrophoneStream::stop_stream() {
    if (is_running) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        is_running = false;
    }
}
