#ifndef DIALOGUE_CONTROLLER_H
#define DIALOGUE_CONTROLLER_H

#include <string>
#include "../llm/llama_stream.h"
#include "../persona/persona_state.h"
#include "../tts/tts_stream.h"

#include <atomic>

#include <vector>
#include <utility>

class DialogueController {
public:
    LLMStream* llm;
    PersonaState* persona;
    TTSEngine* tts;

    std::atomic<bool> agentSpeaking;
    float interruptConfidence;

    DialogueController(LLMStream* l, PersonaState* p, TTSEngine* t);
    
    void onUserSpeech(const std::string& text, bool whileAgentSpeaking, double asr_latency_ms);
    void handleInterrupt();
    void respond(const std::string& userText, double asr_latency_ms);

private:
    std::vector<std::pair<std::string, std::string>> history;
    const size_t maxHistory = 5; // Keep last 5 exchanges
};


#endif // DIALOGUE_CONTROLLER_H
