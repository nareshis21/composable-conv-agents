# C++ Full-Duplex Voice Agent (Research Benchmarking Suite)

A high-performance, ultra-low latency C++ Voice Agent designed for research into natural human-computer interaction. It implements a "PersonaPlex-style" interaction model using a modular cascade (Whisper ASR, Llama LLM, Piper TTS) with advanced full-duplex features.

## 🚀 Key Features
- **Ultra-Low Latency**: End-to-end response times in the sub-800ms range (hardware dependent).
- **Full-Duplex Interruption**: Instantly cancels LLM generation and TTS playback if the user interrupts the agent.
- **Backchanneling**: Natural interjections (e.g., "uh-huh," "right") to signal active listening.
- **Chat History**: Maintains short-term conversational context for natural follow-up questions.
- **Research Dashboard**: A live web-based dashboard (Flask + Chart.js) to visualize P50/P90 latencies and tokens-per-second.
- **Automated Test Bench**: One-click scripts to "replay" recorded speech sets and collect scientific benchmarking data.

---

## 🛠️ Technology Stack
- **ASR**: `whisper.cpp` (Medium.en model, Q5_0 quantization)
- **LLM**: `llama.cpp` (Qwen 2.5 3B Instruct, Q4_K_M quantization)
- **TTS**: [Piper](https://github.com/rhasspy/piper) (Lessac Medium voice)
- **VAD**: [Silero VAD](https://github.com/snakers4/silero-vad) via ONNX Runtime
- **Build System**: CMake (MSVC on Windows / GCC on Linux)
- **Hardware Acceleration**: Full CUDA support for Whisper and Llama inference.

---

## 📥 Model Downloads
To run this agent, create a `models/` directory in the project root and download the following:

1.  **Whisper Model**: [ggml-medium.en-q5_0.bin](https://huggingface.co/ggerganov/whisper.cpp/blob/main/ggml-medium.en-q5_0.bin)
2.  **LLM**: [qwen2.5-3b-instruct-q4_k_m.gguf](https://huggingface.co/Qwen/Qwen2.5-3B-Instruct-GGUF/blob/main/qwen2.5-3b-instruct-q4_k_m.gguf)
3.  **VAD**: [silero_vad.onnx](https://github.com/snakers4/silero-vad/raw/master/files/silero_vad.onnx)
4.  **Piper Voice**: [en_US-lessac-medium.onnx](https://huggingface.co/rhasspy/piper-voices/blob/v1.0.0/en/en_US/lessac/medium/en_US-lessac-medium.onnx)

*Note: Ensure `piper.exe` is installed and accessible (Default path: `C:\piper\piper.exe`).*

---

## 🔨 Installation & Build

### Prerequisites
- CMake (3.15+)
- Visual Studio 2019/2022 or GCC
- CUDA Toolkit (for GPU acceleration)
- Python 3.10+ (for dashboard and automated tests)

### Compilation
```powershell
mkdir build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## 📊 Benchmarking & Research

### 1. Generate Test Audio
First, generate the standardized 40-utterance peer-reviewed speech set:
```powershell
pip install edge-tts flask flask-cors pandas
python generate_test_audio.py
```

### 2. Run Automated Test Bench
Start the full automated run. This will launch the C++ agent, the Live Dashboard, and replay all WAV files through the system:
```powershell
python automated_test.py
```

### 3. View Results
Access the live dashboard at `http://127.0.0.1:5000` to see real-time latency distributions and performance metrics.

---

## 📝 License
This project is intended for research and educational purposes. Underlying model licenses (Whisper, Qwen, Piper) apply.
