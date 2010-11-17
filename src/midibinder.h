/*
 * Toonloop
 *
 * Copyright 2010 Alexandre Quessy
 * <alexandre@quessy.net>
 * http://www.toonloop.com
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
