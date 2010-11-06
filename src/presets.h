#ifndef __PRESETS_H__
#define __PRESETS_H__

#include <map>
#include <string>

struct MidiRule {

    std::string name_;
    std::map<std::string, std::string> attributes_;
};

void init_midi_presets();
#endif
