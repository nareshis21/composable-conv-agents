#ifndef PERSONA_STATE_H
#define PERSONA_STATE_H

#include <string>

class PersonaState {
public:
    std::string role;
    std::string tone;
    std::string verbosity;

    PersonaState();
    std::string promptInjection();
};

#endif // PERSONA_STATE_H
