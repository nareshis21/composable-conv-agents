#include <windows.h> // For SetConsoleOutputCP
#include "audio/mic_stream.h"
#include "audio/vad.h"
#include "persona/persona_state.h"
#include "llm/llama_stream.h"
#include "asr/whisper_stream.h" 
#include "tts/tts_stream.h"
#include "controller/dialogue_controller.h"
#include "utils/perf_monitor.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <atomic>

// Thread-safe queue for audio chunks
std::queue<std::vector<int16_t>> audio_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;
std::atomic<bool> running(true);

// Global flag for response thread management
std::atomic<bool> response_in_progress(false);
std::atomic<bool> test_mode_active(false);

void processing_thread(VAD* vad, WhisperASR* asr, DialogueController* controller) {
    std::vector<int16_t> audio_buffer;
    bool is_speaking = false;
    int silence_frames = 0;
    auto& monitor = PerfMonitor::getInstance();

    // Interruption state
    int interrupt_frames = 0;

    // Backchanneling state
    auto last_backchannel_time = std::chrono::steady_clock::now();
    int speech_chunk_count = 0;

    while (running) {
        std::vector<int16_t> chunk;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, [] { return !audio_queue.empty() || !running; });
            if (!running && audio_queue.empty()) break;
            chunk = std::move(audio_queue.front());
            audio_queue.pop();
        }

        if (test_mode_active) {
            // Ignore microphone while automation is running
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                while(!audio_queue.empty()) audio_queue.pop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        bool vad_active = vad->isSpeech(chunk.data(), (int)chunk.size());

        // FULL DUPLEX 1: Interruption (with debounce to avoid echo trigger)
        if (controller->agentSpeaking) {
            if (vad_active) {
                interrupt_frames++;
                if (interrupt_frames > 8) { // ~250ms of sustained sound
                    std::cout << "\n[INTERRUPT] User speech detected while agent speaking!" << std::endl;
                    controller->handleInterrupt();
                    is_speaking = true;
                    audio_buffer.clear();
                    audio_buffer.insert(audio_buffer.end(), chunk.begin(), chunk.end());
                    silence_frames = 0;
                    interrupt_frames = 0;
                    continue; 
                }
            } else {
                interrupt_frames = 0;
            }
        } else {
            interrupt_frames = 0;
        }
        
        if (vad_active) {
            if (!is_speaking) {
                std::cout << "\n[User detected]: " << std::flush;
                is_speaking = true;
                audio_buffer.clear();
                speech_chunk_count = 0;
                last_backchannel_time = std::chrono::steady_clock::now();
            }
            audio_buffer.insert(audio_buffer.end(), chunk.begin(), chunk.end());
            silence_frames = 0;
            std::cout << "." << std::flush;

            // FULL DUPLEX 2: Backchanneling
            // If user speaks for > 4 seconds, throw in an "uh-huh" or "yeah"
            speech_chunk_count++;
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_backchannel_time).count();
            
            if (speech_chunk_count > 120 && duration > 4) { // 120 chunks * 32ms ~= 3.8s
                std::cout << "\n[Backchanneling...]" << std::flush;
                controller->tts->playBackchannel("generic");
                last_backchannel_time = std::chrono::steady_clock::now();
            }

        } else {
            if (is_speaking) {
                silence_frames++;
                // 17 frames ~= 500ms
                if (silence_frames > 17) {
                    std::cout << " [Processing...]" << std::endl;
                    
                    monitor.startTimer("E2E");
                    monitor.startTimer("ASR");
                    
                    std::vector<int16_t> buffer_copy = audio_buffer;
                    
                    std::thread([asr, controller, buffer_copy, &monitor](){
                        if (response_in_progress) return;
                        response_in_progress = true;

                        asr->transcribe(buffer_copy, [&](const std::string& text){
                            double asr_ms = monitor.stopTimer("ASR");
                            if (text.empty()) return;
                            if (text == "[BLANK_AUDIO]" || text == "[Silence]" || text.find("(Video Ad)") != std::string::npos) return;
                            
                            std::cout << "User: " << text << " (ASR: " << asr_ms << "ms)" << std::endl;
                            controller->onUserSpeech(text, controller->agentSpeaking, asr_ms);
                        });

                        {
                            std::lock_guard<std::mutex> lock(queue_mutex);
                            while(!audio_queue.empty()) audio_queue.pop();
                        }
                        
                        response_in_progress = false;
                    }).detach();

                    is_speaking = false;
                    audio_buffer.clear();
                    silence_frames = 0;
                }
            }
        }
    }
}

// Suppress Llama logs
void llama_log_callback(ggml_log_level level, const char * text, void * user_data) {
    (void)level; (void)text; (void)user_data;
    // No-op to suppress logs
}

int main(int argc, char** argv) {
    // Set Console to UTF-8 to handle Emojis
    SetConsoleOutputCP(CP_UTF8);
    
    // Disable Llama/GGML verbose logging
    llama_log_set(llama_log_callback, nullptr);

    std::cout << "Starting Voice Agent (Press Ctrl+C to stop)..." << std::endl;

    VAD vad(L"models/silero_vad.onnx"); 
    MicrophoneStream mic(16000, 512); 
    PersonaState persona;
    
    std::string modelPath = "models/qwen2.5-3b-instruct-q4_k_m.gguf"; 
    LLMStream llm(modelPath);
    WhisperASR asr("models/ggml-medium.en-q5_0.bin");
    TTSEngine tts;
    DialogueController controller(&llm, &persona, &tts);

    std::thread worker(processing_thread, &vad, &asr, &controller);
    
    mic.start_stream([&](const std::vector<int16_t>& audio_chunk){
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            audio_queue.push(audio_chunk);
        }
        queue_cv.notify_one();
    });

    std::cout << "\n[CLI] Automated Test Mode Active." << std::endl;
    std::cout << "[CLI]  - Type 'text: hello' to simulate speech input." << std::endl;
    std::cout << "[CLI]  - Type 'file: test_audio/file.wav' to run ASR on a file." << std::endl;
    std::cout << "[CLI]  - Type 'quit' to exit.\n" << std::endl;

    std::string input;
    while (running && std::getline(std::cin, input)) {
        if (input == "quit") break;
        
        if (input.substr(0, 5) == "text:") {
            test_mode_active = true;
            std::string text = input.substr(5);
            std::cout << "[Test] Injected text: " << text << std::endl;
            
            auto& monitor = PerfMonitor::getInstance();
            monitor.startTimer("E2E"); // Start E2E for text injection
            controller.onUserSpeech(text, false, 0.0);
        }
        else if (input.substr(0, 5) == "file:") {
            test_mode_active = true;
            std::string path = input.substr(5);
            path.erase(0, path.find_first_not_of(" "));
            path.erase(path.find_last_not_of(" ") + 1);
            
            auto& monitor = PerfMonitor::getInstance();
            monitor.startTimer("E2E"); // E2E starts when we begin processing the 'audio'
            monitor.startTimer("ASR"); // Track file transcription time
            
            std::cout << "[Test] Processing WAV: " << path << std::endl;
            asr.transcribe_wav(path, [&](const std::string& text){
                double asr_ms = monitor.stopTimer("ASR");
                std::cout << "[Test ASR] File Output: " << text << " (took " << asr_ms << "ms)" << std::endl;
                controller.onUserSpeech(text, false, asr_ms);
            });
        }
    }

    running = false;
    queue_cv.notify_all();
    if (worker.joinable()) worker.join();

    return 0;
}
