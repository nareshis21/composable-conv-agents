#include "llama_stream.h"
#include <iostream>
#include <vector>
#include <cstring>

LLMStream::LLMStream(const std::string& model_path) : model(nullptr), ctx(nullptr), abort(false) {
    llama_model_params model_params = llama_model_default_params();
    // Offload layers to GPU if compiled with CUBLAS
    model_params.n_gpu_layers = 99; 

    model = llama_load_model_from_file(model_path.c_str(), model_params);
    if (!model) {
        std::cerr << "Failed to load llama model" << std::endl;
        return;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048; // Context size
    ctx = llama_new_context_with_model(model, ctx_params);
}

LLMStream::~LLMStream() {
    if (ctx) llama_free(ctx);
    if (model) llama_free_model(model);
}

void LLMStream::generate(const std::string& prompt, std::function<void(const std::string&)> token_callback) {
    if (!model || !ctx) return;
    abort = false;
    
    // Clear KV cache to allow re-evaluating the full prompt from scratch
    // This is necessary because DialogueController constructs the full prompt each time.
    llama_memory_t mem = llama_get_memory(ctx);
    llama_memory_clear(mem, true);

    // 1. Get Vocab
    const llama_vocab* vocab = llama_model_get_vocab(model);

    // 2. Tokenize
    std::vector<llama_token> tokens_list;
    tokens_list.resize(prompt.size() + 1);
    int n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens_list.data(), tokens_list.size(), true, false);
    if (n_tokens < 0) {
        tokens_list.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens_list.data(), tokens_list.size(), true, false);
    }
    tokens_list.resize(n_tokens);

    // 3. Prepare Batch
    // Allocate batch capability for context size to be safe (or at least n_tokens)
    int n_batch_cap = 2048; 
    if (n_tokens > n_batch_cap) n_batch_cap = n_tokens;
    
    llama_batch batch = llama_batch_init(n_batch_cap, 0, 1); // max tokens, embd, seqs

    // 4. Decode Prompt
    // Fill batch manually
    batch.n_tokens = n_tokens;
    for(int i=0; i<n_tokens; i++) {
        batch.token[i] = tokens_list[i];
        batch.pos[i] = i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = false; // logic: only last token needs logits?
    }
    batch.logits[n_tokens - 1] = true;

    if (llama_decode(ctx, batch) != 0) {
        std::cerr << "Prompt decode failed" << std::endl;
        llama_batch_free(batch);
        return;
    }

    // 5. Generate Loop
    int n_cur = n_tokens;
    std::vector<llama_token> history_tokens;
    history_tokens.reserve(2048);

    while (!abort && n_cur < n_batch_cap) { // Safety check
        auto* logits = llama_get_logits(ctx); // Logits from last decode
        int n_vocab = llama_n_vocab(vocab);

        // Apply Repetition Penalty (Quick & Dirty implementation)
        const float penalty = 1.2f; // Strong penalty to break loops
        const int penalty_window = 64; 
        
        // Create a penalized copy of logits? No, modifying in place is fine for next step greedy 
        // (but we are pointing to internal memory, so actually we should copy if we want to be safe, 
        // AND we must be careful not to mess up internal state if we were doing beam search, but here we are single stream).
        // Llama.cpp logits pointer is usually read-only or ephemeral per batch.
        
        // Let's Find max with penalty on the fly
        llama_token new_token_id = 0;
        float max_prob = -1e9;

        for (int i=0; i<n_vocab; i++) {
            float val = logits[i];
            
            // Check if token 'i' is in recent history
            int start_idx = std::max(0, (int)history_tokens.size() - penalty_window);
            for (int k = start_idx; k < history_tokens.size(); ++k) {
                if (history_tokens[k] == i) {
                    if (val > 0) val /= penalty;
                    else val *= penalty;
                    break; // Apply once
                }
            }
            
            if (val > max_prob) {
                max_prob = val;
                new_token_id = i;
            }
        }

        // Output Text (Filter tags)
        char buf[256];
        int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true); 
        std::string token_str(buf, n);
        
        // Stop Check (More aggressive)
        if (token_str == "</s>" || token_str.find("im_end") != std::string::npos || token_str.find("im_start") != std::string::npos || token_str == "<|endoftext|>") break;
        if (n_cur > 2000) break;

        token_callback(token_str); // Only callback if not a stop token 
        history_tokens.push_back(new_token_id);

        // Decode Next Token
        batch.n_tokens = 1;
        batch.token[0] = new_token_id;
        batch.pos[0] = n_cur;
        batch.n_seq_id[0] = 1;
        batch.seq_id[0][0] = 0;
        batch.logits[0] = true; // We need logits for next generation

        if (llama_decode(ctx, batch) != 0) {
             std::cerr << "Generate decode failed" << std::endl;
             break;
        }
        n_cur++;
    }
    
    llama_batch_free(batch);
}

void LLMStream::stop() {
    abort = true;
}

bool LLMStream::isAborted() const {
    return abort;
}
