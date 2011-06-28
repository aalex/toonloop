/*
 * Toonloop
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
 *
 * Toonloop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toonloop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the gnu general public license
 * along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PRESETS_H__
#define __PRESETS_H__

#include <glib.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/** Types of MidiRule structs */
typedef enum // TODO: use const string with names of the XML tags instead of enum
{
    NOTE_ON_RULE,
    NOTE_OFF_RULE,
    CONTROL_ON_RULE,
    CONTROL_OFF_RULE,
    CONTROL_MAP_RULE, // float
    PROGRAM_CHANGE_RULE,
    PITCH_WHEEL_RULE
    //TODO: CONTROL_MAP_INT_RULE
} RuleType;

/** 
 * Instruction to do an action when a MIDI event happens. 
 *
 * This type contains 
 * */
struct MidiRule
{
    public:
        /** Type of MIDI event for which this rule will trigger an action. */
        RuleType type_; // TODO: change for string
        /** Note pitch or control number */
        int number_;
        /** identifies the Command to trigger. */
        std::string action_; // TODO: pass it a Command instance. keep the same instance and only change its values.
        /** Optional string argument to pass to the command. Useful for select_clip, for example. */
        std::string args_;
        /** Maps [0,127] to [from_,to_] */
        float from_;
        /** Maps [0,127] to [from_,to_] */
        float to_;
        //friend std::ostream &operator<< (std::ostream &theStream, const MidiRule &self);
};

typedef std::vector<MidiRule>::iterator MidiRuleIterator;

/**
 * Maps the MIDI input events to Toonloop actions using the rules set in the XML configuration file.
 *
 * The default MIDI preset file is in presets/midi.xml, which is installed to /usr/share/toonloop/presets/midi.xml
 *
 * Introducing the MIDI rules
 * --------------------------
 * The MIDI rules are set using a XML tag whose name is one or "note_on", "note_off", "control_on", etc. Their name match some MIDI input events. Here is the list of attribute each of these rule tag can have:
 *  - "args": Argument that can be passed to the Toonloop command that will be called. It needs to be a valid int or float if the expected type for the given command is one of those.
 *  - "number": The MIDI note or control number.
 *  - "action": The Toonloop command to call.
 *  - "from": Minimum output range of the mapping for a number value. Default is 0.0.
 *  - "to": Maximum output range of the mapping for a number value. Default is 1.0.
 *
 * Supported rules
 * ---------------
 * Here is the list of supported rules:
 *  - note_on: Its "number" attribute must match the MIDI note number.
 *  - note_off: Its "number" attribute must match the MIDI note number.
 *  - control_on: Its "number" attribute must match the MIDI control number.
 *  - control_off: Its "number" attribute must match the MIDI control number.
 *  - control_map: Its "number" attribute must match the MIDI control number.
 *  - program_change: The program number is passed as an argument to the Toonloop action. Only supports the "select_clip" action.
 *  - pitch_bend: Only supports the "set_float" Toonloop action.
 *
 * Supported Toonloop commands
 * --------------------------
 * The MIDI bindings are useful to trigger Toonloop commands. Each rule must have an "action" attribute, and maybe some "args" attribute as well.
 *
 * Here is the list of Toonloop commands that can be called:
 *  - add_image
 *  - remove_image
 *  - quit
 *  - video_record_on
 *  - video_record_off
 *  - save_current_clip
 *  - select_clip i:clip_number (the rule's "args" attribute is passed as clip number)
 *  - set_float s:name f:value (the rule's "args" attribute is used as property name, and the value is mapped if using the "control_map" MIDI rule type)
 *  - set_int s:name i:value (the rule's "args" attribute is used as property name, and the value is mapped if using the "control_map" MIDI rule type)
 *
 * Examples
 * --------
 * For example, one would write the following rules in its ~/.toonloop/midi.xml file:
 *   <note_on number="41" action="select_clip" args="0" />
 *
 * This way, everytime the note number 41 will be pressed, it will select clip zero.
 *
 * An other example:
 *   <control_on  number="64" action="add_image" />
 *
 * To map control values to property, use the set_float action:
 *   <control_map number="74" action="set_float" args="fx.brcosa.contrast" from="0.0" to="2.0" />
 *
 * Everytime the MIDI control 64 will have a non-zero value, it will add an image. Usually, 64 is the sustain pedal.
 */
class MidiBinder
{
    public:
        MidiBinder(bool verbose);
        // TODO: merge find_pitch_wheel_rule + find_program_change_rule into find_rule(RuleType)
        const MidiRule *find_program_change_rule();
        const MidiRule *find_pitch_wheel_rule();
        /** number: for note pitch of control number */
        const MidiRule *find_rule(RuleType rule_type, int number);
        void set_verbose(bool verbose);
    private:
        // TODO: replace by a map<string,vector<MidiRule> >
        std::vector<MidiRule> note_on_rules_;
        std::vector<MidiRule> note_off_rules_;
        std::vector<MidiRule> control_on_rules_;
        std::vector<MidiRule> control_off_rules_;
        std::vector<MidiRule> control_map_rules_;
        std::vector<MidiRule> program_change_rules_;
        std::vector<MidiRule> pitch_wheel_rules_;

        bool load_xml_file(const gchar *file_name);
        static void on_midi_xml_start_element(
            GMarkupParseContext *context,
            const gchar *element_name,
            const gchar **attribute_names,
            const gchar **attribute_values,
            gpointer user_data,
            GError **error);
        bool verbose_;
};

#endif

