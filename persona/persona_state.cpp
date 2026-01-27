#include "persona_state.h"

PersonaState::PersonaState() : role("assistant"), tone("calm"), verbosity("medium") {}

std::string PersonaState::promptInjection() {
    return
        "Chat casually like a normal person. "
        "Playful, relaxed, slightly sarcastic. "
        "Short, natural replies. No emojis or meta talk.";
}

