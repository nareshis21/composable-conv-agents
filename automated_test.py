import json
import subprocess
import time
import os
import csv

import webbrowser
import os

import threading

def run_automated_tests():
    # 0. Clear previous results for a clean dashboard run
    csv_path = "benchmark_results.csv"
    if os.path.exists(csv_path):
        with open(csv_path, "w") as f:
            f.write("TurnID,Timestamp,VAD_Latency,ASR_Latency,LLM_TTFT,TTS_Latency,Total_E2E,Tokens,UserText\n")

    # 1. Start Dashboard Server in background
    print("🌐 Launching Live Research Dashboard...")
    dashboard_proc = subprocess.Popen(["python", "dashboard/app.py"], 
                                    stdout=subprocess.DEVNULL, 
                                    stderr=subprocess.DEVNULL)
    
    # Wait a moment for server to bind
    time.sleep(2)
    webbrowser.open("http://127.0.0.1:5000")

    # 2. Launch the voice agent
    print("🤖 Launching Voice Agent...")
    exe_path = "./build/Release/voice_agent.exe"
    process = subprocess.Popen(
        exe_path,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
        universal_newlines=True
    )

    # 3. Background thread to DRAIN and PRINT stdout
    def drain_logs(proc):
        for line in iter(proc.stdout.readline, ""):
            if line:
                print(f"| [Agent] {line.strip()}")

    log_thread = threading.Thread(target=drain_logs, args=(process,), daemon=True)
    log_thread.start()

    # Helper to send commands
    def send_cmd(cmd):
        process.stdin.write(cmd + "\n")
        process.stdin.flush()

    # 4. Load the test suite
    with open("automated_test_suite.json", "r") as f:
        suite = json.load(f)

    print("🚀 Starting Automated Test Bench for Voice Agent")

    # 3. Iterate through categories
    results = []
    
    # Give it a few seconds to load models
    print("⏳ Initializing models (Whisper/Llama)...")
    time.sleep(10)

    audio_root = "test_audio"

    for category, utterances in suite.items():
        print(f"\n📊 Testing Category: {category}")
        for i, text in enumerate(utterances):
            # Resolve the WAV path based on our naming convention
            safe_name = "".join([c if c.isalnum() else "_" for c in text[:20]]).lower()
            wav_path = os.path.abspath(os.path.join(audio_root, category, f"{i}_{safe_name}.wav"))
            
            if os.path.exists(wav_path):
                print(f"   [{i+1}/{len(utterances)}] Sending File: {wav_path}")
                send_cmd(f"file: {wav_path}")
            else:
                print(f"   [{i+1}/{len(utterances)}] Skipping (WAV missing): {text}")
                # Optional: fallback to text if needed
                # send_cmd(f"text: {text}")
            
            # Allow time for ASR + LLM + TTS to fully complete
            time.sleep(12) 
            
    # 4. Cleanup
    print("\n✅ All tests sent. Shutting down...")
    send_cmd("quit")
    process.terminate()
    dashboard_proc.terminate()

    print("\n📈 Benchmark session finished. Closing dashboard server.")

if __name__ == "__main__":
    run_automated_tests()
