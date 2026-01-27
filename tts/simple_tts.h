#pragma once
#include <string>
#include <vector>

class SimpleTTS {
public:
    SimpleTTS();
    ~SimpleTTS();

    // Text-to-Speech execution (fire and forget or blocking depending on implementation)
    void speak(const std::string& text);
    
    // Quick backchannel response
    void playBackchannel(const std::string& type);
    
    // Stop current playback
    void stop();

private:
     std::string piperPath;
     std::string modelPath;
     void execute_command(const std::string& cmd);
};
