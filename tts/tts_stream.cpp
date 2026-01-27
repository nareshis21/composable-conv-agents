#include "tts_stream.h"
#include <iostream>

TTSEngine::TTSEngine() {
    impl = new SimpleTTS();
}

TTSEngine::~TTSEngine() {
    if (impl) delete impl;
}

void TTSEngine::speak(const std::string& text) {
    if (impl) impl->speak(text);
}

void TTSEngine::playBackchannel(const std::string& type) {
    if (impl) impl->playBackchannel(type);
}

void TTSEngine::flush() {
    // check
}

void TTSEngine::stop() {
    if (impl) impl->stop();
}

