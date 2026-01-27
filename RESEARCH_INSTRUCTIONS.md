# 🔬 Research Experiment Setup

This project is now instrumented to emit "research-grade" latency metrics (`benchmark_results.csv`) and use neural TTS (Piper).

## 1. Get Neural TTS (Piper)
To make the agent speak with a high-quality voice, we use **Piper** via CLI piping.

## 1. Get Neural TTS (Piper)
To make the agent speak with a high-quality voice, we use **Piper** via CLI piping.

1.  **Download Piper v1.2.0 (Last C++ Binary Release)**:
    *   **Link:** [piper_windows_amd64.zip (v1.2.0)](https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_windows_amd64.zip)
    *   **Why v1.2.0?** Newer versions (v1.3.0+) are Python-only. We need the `piper.exe` C++ binary for this project.
    *   **Action:** Extract `piper.exe` and `piper_phonemize.exe` into a folder (e.g., `C:\piper`) and **add it to your System PATH**.

2.  **Download Voice Model (Lessac Medium)**:
    *   We need the `.onnx` model and the `.json` config.
    *   **Model:** [en_US-lessac-medium.onnx](https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/medium/en_US-lessac-medium.onnx)
    *   **Config:** [en_US-lessac-medium.onnx.json](https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/medium/en_US-lessac-medium.onnx.json)
    *   **Action:** Place these two files directly in your `voice_agent_cpp` project folder.

3.  **Install Audio Player (`ffplay`)**:
    *   Install **FFmpeg** and ensure `ffplay` is in your PATH.
    *   Piper outputs raw audio to stdout -> we pipe to `ffplay`.

## 2. Running Benchmarks
1.  Build the project.
2.  Run `voice_agent.exe`.
3.  Speak to the agent.
4.  After every turn, check the console for `[METRICS]` logs.
5.  A file `benchmark_results.csv` will be generated/updated automatically.

## 3. Key Metrics Tracked
*   **ASR Latency**: Time to transcribe audio (currently placeholder 0ms).
*   **LLM TTFT (Time To First Token)**: Critical metric for voice. Should be <300ms.
*   **Total E2E**: From "User stops speaking" to "Agent starts audio".

## 4. Next Steps for Publication
*   **Graph your TTFT** vs Context Length.
*   Compare `Qwen2.5-3B` vs `Phi-3.5` using these metrics.
*   Implement "Smart Interruption" in `DialogueController`.
