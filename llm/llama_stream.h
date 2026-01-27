#ifndef LLAMA_STREAM_H
#define LLAMA_STREAM_H

#include "llama.h"
#include <string>
#include <functional>
#include <vector>

#include <atomic>

class LLMStream {
public:
    llama_model* model;
    llama_context* ctx;
    std::atomic<bool> abort;

    LLMStream(const std::string& model_path);
    ~LLMStream();

    void generate(const std::string& prompt, std::function<void(const std::string&)> token_callback);
    void stop();
    bool isAborted() const;
};

#endif // LLAMA_STREAM_H
