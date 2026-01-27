#ifndef PERF_MONITOR_H
#define PERF_MONITOR_H

#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>
#include <map>

struct InteractionMetrics {
    int turn_id;
    double vad_latency_ms;      // Time from speech end to VAD trigger
    double asr_latency_ms;      // Time for whisper to transcribe
    double llm_ttft_ms;         // Time to First Token
    double tts_latency_ms;      // Time to first audio chunk
    double total_e2e_ms;        // Total Voice-to-Audio latency
    int token_count;
    std::string timestamp;
    std::string user_text;      // The phrase used for the test
};

class PerfMonitor {
public:
    static PerfMonitor& getInstance() {
        static PerfMonitor instance;
        return instance;
    }

    void startTimer(const std::string& key);
    double stopTimer(const std::string& key);
    void logTurn(InteractionMetrics metrics);
    void saveCSV(const std::string& filename = "benchmark_results.csv");

private:
    PerfMonitor() {}
    std::map<std::string, std::chrono::high_resolution_clock::time_point> timers;
    std::vector<InteractionMetrics> history;
    std::mutex mtx;
};

#endif // PERF_MONITOR_H
