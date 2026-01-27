#include "perf_monitor.h"
#include <iomanip>
#include <ctime>
#include <sstream>

void PerfMonitor::startTimer(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    timers[key] = std::chrono::high_resolution_clock::now();
}

double PerfMonitor::stopTimer(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    if (timers.find(key) == timers.end()) return 0.0;
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - timers[key];
    return ms.count();
}

void PerfMonitor::logTurn(InteractionMetrics metrics) {
    std::lock_guard<std::mutex> lock(mtx);
    
    // Auto-timestamp
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    metrics.timestamp = oss.str();
    
    history.push_back(metrics);
    
    // Real-time console log for the researcher
    std::cout << "\n[METRICS] Turn " << metrics.turn_id << " Stats:" << std::endl;
    std::cout << "  - ASR : " << metrics.asr_latency_ms << " ms" << std::endl;
    std::cout << "  - LLM (TTFT) : " << metrics.llm_ttft_ms << " ms" << std::endl;
    std::cout << "  - Total E2E  : " << metrics.total_e2e_ms << " ms" << std::endl;
    std::cout << "  - Tokens : " << metrics.token_count << std::endl;
}

void PerfMonitor::saveCSV(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx);
    std::ofstream file(filename);
    
    file << "TurnID,Timestamp,VAD_Latency,ASR_Latency,LLM_TTFT,TTS_Latency,Total_E2E,Tokens,UserText\n";
    
    for(const auto& m : history) {
        file << m.turn_id << ","
             << m.timestamp << ","
             << m.vad_latency_ms << ","
             << m.asr_latency_ms << ","
             << m.llm_ttft_ms << ","
             << m.tts_latency_ms << ","
             << m.total_e2e_ms << ","
             << m.token_count << ",\""
             << m.user_text << "\"\n";
    }
    std::cout << "[RESEARCH] Data exported to " << filename << std::endl;
}
