#ifndef TTS_STREAM_H
#define TTS_STREAM_H

#include <string>
#include "simple_tts.h"

class TTSEngine {
public:
    TTSEngine();
    ~TTSEngine();
    
    void speak(const std::string& text);
    void flush();
    void stop();
    void playBackchannel(const std::string& type = "generic");

private:
    SimpleTTS* impl = nullptr;
};

#endif // TTS_STREAM_H
