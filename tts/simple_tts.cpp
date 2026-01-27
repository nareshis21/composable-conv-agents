#include "simple_tts.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

namespace fs = std::filesystem;

SimpleTTS::SimpleTTS() {
    // Portability: Search for Piper and Voice Model in relative paths
    std::vector<std::string> piperSearch = {"piper/piper.exe", "piper.exe", "../piper/piper.exe", "C:\\piper\\piper.exe"};
    for(auto& p : piperSearch) if(fs::exists(p)) { piperPath = p; break; }

    std::vector<std::string> modelSearch = {"models/en_US-lessac-medium.onnx", "en_US-lessac-medium.onnx", "../models/en_US-lessac-medium.onnx"};
    for(auto& m : modelSearch) if(fs::exists(m)) { modelPath = m; break; }
}

SimpleTTS::~SimpleTTS() {
    stop();
}

void SimpleTTS::stop() {
#ifdef _WIN32
    system("taskkill /F /IM ffplay.exe /T > NUL 2>&1");
    // We don't kill piper because it usually exits after input. 
    // But if we have a stuck pipe, killing it is safe.
    system("taskkill /F /IM piper.exe /T > NUL 2>&1");
#endif
}

void SimpleTTS::speak(const std::string& text) {
    if (text.empty()) return;
    
    // Clean text / preprocessing
    // Clean text / preprocessing
    std::string clean = text;

    // 1. Truncate at common stop tokens (found early or late)
    std::vector<std::string> stops = {"<|im_end|>", "<|im_end", "|im_end", "im_end", "assistant", "system", "user"};
    for (const auto& s : stops) {
        size_t p = clean.find(s);
        if (p != std::string::npos) {
            clean = clean.substr(0, p);
        }
    }

    // 2. Remove ANY remaining <|...|> or similar brackets if they somehow survived
    size_t lt = clean.find("<");
    if (lt != std::string::npos) {
        clean = clean.substr(0, lt);
    }

    // 2. Remove [tags] (e.g. [laughter], [metrics])
    // Simple regex-like replacement: loop and remove
    bool inside_bracket = false;
    std::string no_brackets = "";
    for (char c : clean) {
        if (c == '[') inside_bracket = true;
        else if (c == ']') inside_bracket = false;
        else if (!inside_bracket) no_brackets += c;
    }
    clean = no_brackets;

    // 3. Remove Hashtags? (User said "DONT ADD TAGS", assuming #funny should be removed or just the #)
    // Let's remove words starting with # if they look like metadata tags
    // Or just remove the # character itself so it speaks the word? 
    // Usually metadata hashtags come at the end. 
    // Simplest approach: Remove any '#' character.
    clean.erase(std::remove(clean.begin(), clean.end(), '#'), clean.end());

    // 4. Remove Non-ASCII (Emojis)
    // Piper might choke on them or speak them weirdly.
    std::string ascii_only = "";
    for (char c : clean) {
        // Allow basic ASCII printable (32-126) and newlines
        if ((unsigned char)c < 128) {
             ascii_only += c;
        }
    }
    clean = ascii_only;

    // Remove asterisks (*) often used for actions *waves*
    clean.erase(std::remove(clean.begin(), clean.end(), '*'), clean.end());

    // Basic quote escaping for command line
    size_t pos = 0;
    while ((pos = clean.find("\"", pos)) != std::string::npos) {
        clean.replace(pos, 1, "\\\"");
        pos += 2;
    }

    // Prepare Piper Command
    // We use 22050Hz for Lessac Medium
    std::string cmd;
    
    // Construct the full command
    // We use a temporary file or just echo. Echo -e or similar isn't available on standard cmd.
    // For Windows, echo "text" works reasonably well if text is simple.
    // Better: write to a temp file and read from it to avoid shell escaping hell.
    
    std::string tempTextFile = "tts_input.txt";
    std::ofstream ofs(tempTextFile);
    ofs << clean;
    ofs.close();

    cmd = "type " + tempTextFile + " | " + piperPath + 
          " --model " + modelPath + " --output_raw | ffplay -f s16le -ar 22050 -nodisp - -autoexit";

    std::cout << "[SimpleTTS] Speaking: " << clean << std::endl;
    std::cout << "[SimpleTTS] Executing: " << cmd << std::endl;
    
    // Use system() instead of popen for the complex pipe to simplify
    system(cmd.c_str());
}

void SimpleTTS::playBackchannel(const std::string& type) {
    std::string text = "uh-huh";
    if (type == "agreement") text = "yeah";
    if (type == "thinking") text = "hmm";
    
    std::string cmd = "echo " + text + " | " + piperPath + 
                      " --model " + modelPath + " --output_raw | ffplay -f s16le -ar 22050 -nodisp - -autoexit";
    
    // Run in background
#ifdef _WIN32
    std::string bg_cmd = "start /B " + cmd;
    system(bg_cmd.c_str());
#else
    std::string bg_cmd = cmd + " &";
    system(bg_cmd.c_str());
#endif
}

void SimpleTTS::execute_command(const std::string& cmd) {
    system(cmd.c_str());
}
