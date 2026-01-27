#include "dialogue_controller.h"
#include "../utils/perf_monitor.h" // Research: Metrics logging
#include <cstdlib> // For rand() placeholder

DialogueController::DialogueController(LLMStream* l, PersonaState* p, TTSEngine* t)
    : llm(l), persona(p), tts(t), agentSpeaking(false), interruptConfidence(0.0f) {}

void DialogueController::onUserSpeech(const std::string& text, bool whileAgentSpeaking, double asr_latency_ms) {
    if (whileAgentSpeaking) {
        interruptConfidence += 0.5f;
        if (interruptConfidence > 1.0f) handleInterrupt();
    } else {
        respond(text, asr_latency_ms);
    }
}

void DialogueController::handleInterrupt() {
    llm->stop();
    tts->stop();
    agentSpeaking = false;
    interruptConfidence = 0.0f;
    std::cout << "[Controller] Interrupt triggered!" << std::endl;
}

void DialogueController::respond(const std::string& userText, double asr_latency_ms) {
    if(userText.empty()) return;

    // Start Research Timers
    auto& monitor = PerfMonitor::getInstance();
    monitor.startTimer("LLM_PRE"); // Time to First Token

    // 1. Build Prompt with History
    std::string prompt = "<|im_start|>system\n" + persona->promptInjection() + "<|im_end|>\n";
    
    // Add history (User/Assistant exchanges)
    for (const auto& exchange : history) {
        prompt += "<|im_start|>user\n" + exchange.first + "<|im_end|>\n";
        prompt += "<|im_start|>assistant\n" + exchange.second + "<|im_end|>\n";
    }

    // Add current user prompt
    prompt += "<|im_start|>user\n" + userText + "<|im_end|>\n";
    prompt += "<|im_start|>assistant\n";
    
    agentSpeaking = true;
    llm->abort = false; // Reset abort flag for new response
    
    bool firstToken = true;
    double ttft_ms = 0.0;
    int tokenCount = 0;
    
    std::cout << "[LLM] Generating..." << std::endl;
    std::string fullResponse;
    
    llm->generate(prompt, [this, &firstToken, &monitor, &ttft_ms, &tokenCount, &fullResponse](const std::string& token){
        if (llm->isAborted()) return; // Fast exit callback
        if (firstToken) {
            ttft_ms = monitor.stopTimer("LLM_PRE"); 
            firstToken = false;
        }
        tokenCount++;
        std::cout << token << std::flush; // Visual stream
        fullResponse += token;
    });
    
    std::cout << "\n[LLM] Generation Done. Calling TTS..." << std::endl;
    
    // Save to history (even if aborted, we store what we got)
    history.push_back({userText, fullResponse});
    if (history.size() > maxHistory) {
        history.erase(history.begin());
    }

    
    // Check if we were interrupted during LLM generation
    if (llm->isAborted()) {
        std::cout << "[Controller] Aborted before TTS." << std::endl;
        agentSpeaking = false;
        return;
    }

    // Total E2E is from "User stops" to "Agent starts audio"
    double e2e_ms = monitor.stopTimer("E2E");
    
    // Speak full response for stability (one-shot pipe)
    tts->speak(fullResponse);
    std::cout << "[TTS] Speak Done." << std::endl;
    
    // Log final stats for this turn to CSV/Console
    InteractionMetrics m;
    m.turn_id = rand() % 10000;
    m.vad_latency_ms = 0;
    m.asr_latency_ms = asr_latency_ms;
    m.llm_ttft_ms = ttft_ms;
    m.tts_latency_ms = 0;
    m.total_e2e_ms = e2e_ms;
    m.token_count = tokenCount;
    m.timestamp = ""; 
    m.user_text = userText;
    
    monitor.logTurn(m); // Save to history
    monitor.saveCSV();  // Auto-save logic
    
    agentSpeaking = false;
}
