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

// TODO: rename to midibinder.cpp

#include "midibinder.h"
#include "unused.h"
#include <boost/lexical_cast.hpp>
#include <cstdlib> // for getenv
#include <glib.h>
#include <iostream>
#include <map>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

// Constant for the presets data directory: (/usr/share/toonloop/presets/)
#define VAL(str) #str
#define TOSTRING(str) VAL(str)
#define PRESETS_DIR TOSTRING(DATADIR) "/toonloop/presets/"

// TODO:2010-11-07:aalex:override << for MidiRule
#if 0
std::ostream &MidiRule::operator<< (std::ostream &theStream, const MidiRule &self)
{
    theStream << "MidiRule: type=" << self.type_ << " action=" << self.action_ << " args=" << self.args_ << " number=" << self.number_ << " from|to=" << self.from_ << " " << self.to_;
    return theStream;
}
#endif
/**
 * Returns 0 if none found.
 */
const MidiRule *MidiBinder::find_program_change_rule()
{
    if (program_change_rules_.size() >= 1)
        try {
            return &(program_change_rules_.at(0));
        } catch (std::out_of_range &e) {
            std::cout << "ERROR: Program change rule: out of range! " << e.what() << std::endl;
            return 0;
        }
    else
        return 0;
}

/**
 * Returns 0 if none found.
 */
const MidiRule *MidiBinder::find_pitch_wheel_rule()
{
    if (pitch_wheel_rules_.size() >= 1)
        try {
            return &(pitch_wheel_rules_.at(0));
        } catch (std::out_of_range &e) {
            std::cout << "ERROR: Pitch wheel rule: out of range! " << e.what() << std::endl;
            return 0;
        }
    else
        return 0;
}
/**
 * Returns 0 if none found.
 */
const MidiRule *MidiBinder::find_rule(RuleType rule_type, int number)
{
    MidiRuleIterator iter;
    MidiRuleIterator end;
    switch (rule_type)
    {
        case NOTE_ON_RULE:
            iter = note_on_rules_.begin();
            end = note_on_rules_.end();
            break;
        case NOTE_OFF_RULE:
            iter = note_off_rules_.begin();
            end = note_off_rules_.end();
            break;
        case CONTROL_ON_RULE:
            iter = control_on_rules_.begin();
            end = control_on_rules_.end();
            break;
        case CONTROL_OFF_RULE:
            iter = control_off_rules_.begin();
            end = control_off_rules_.end();
            break;
        case CONTROL_MAP_RULE:
            iter = control_map_rules_.begin();
            end = control_map_rules_.end();
            break;
        case PITCH_WHEEL_RULE:
            iter = pitch_wheel_rules_.begin();
            end = pitch_wheel_rules_.end();
            break;
        default:
            g_critical("ERROR: Unsupported MIDI binding rule type");
            return 0;
            break;
    }
    for ( ; iter != end; ++iter)
    {
        if ((*iter).number_ == number)
        {
            if (verbose_)
                std::cout << __FUNCTION__ << ":" " found a rule for #" << number << std::endl;
            return &(*iter);
        }
    }
    if (verbose_)
        std::cout << __FUNCTION__ << ":" " did not find a rule for #" << number << std::endl;
    return 0;
}

/*
 * Called for open tags <foo bar="baz">
 */
void MidiBinder::on_midi_xml_start_element(
        GMarkupParseContext *context,
        const gchar *element_name,
        const gchar **attribute_names,
        const gchar **attribute_values,
        gpointer user_data,
        GError **error)
{
    UNUSED(context);
    UNUSED(error);
    MidiBinder *self = static_cast<MidiBinder *>(user_data);

    MidiRule rule;
    // default values:
    rule.action_ = "";
    rule.args_ = "";
    rule.from_ = 0.0f;
    rule.to_ = 1.0f;
    rule.number_ = 0;
    if (g_strcmp0(element_name, "toonloop_midi_rules") == 0) // root
        return;
    else if (g_strcmp0(element_name, "note_on") == 0)
        rule.type_ = NOTE_ON_RULE;
    else if (g_strcmp0(element_name, "note_off") == 0)
        rule.type_ = NOTE_OFF_RULE;
    else if (g_strcmp0(element_name, "control_on") == 0)
        rule.type_ = CONTROL_ON_RULE;
    else if (g_strcmp0(element_name, "control_off") == 0)
        rule.type_ = CONTROL_OFF_RULE;
    else if (g_strcmp0(element_name, "control_map") == 0)
        rule.type_ = CONTROL_MAP_RULE;
    else if (g_strcmp0(element_name, "program_change") == 0)
        rule.type_ = PROGRAM_CHANGE_RULE;
    else if (g_strcmp0(element_name, "pitch_bend") == 0)
        rule.type_ = PITCH_WHEEL_RULE;
    else
        g_critical("Invalid MIDI rule type: %s", element_name);
    if (self->verbose_)
        std::cout << "Adding rule " << element_name << ":";
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;
    while (*name_cursor)
    {
        if (self->verbose_)
            std::cout << " " << *name_cursor << "=" << *value_cursor;
        if (g_strcmp0(*name_cursor, "args") == 0)
            rule.args_ = std::string(*value_cursor);
        else if (g_strcmp0(*name_cursor, "action") == 0)
            rule.action_ = std::string(*value_cursor);
        else if (g_strcmp0(*name_cursor, "number") == 0)
        {
            try {
                rule.number_ = boost::lexical_cast<int>(*value_cursor);
            } catch(boost::bad_lexical_cast &) {
                g_critical("Invalid int for %s in XML file: %s", *name_cursor, *value_cursor);
                return;
            }
        }
        else if (g_strcmp0(*name_cursor, "from") == 0)
        {
            try {
                rule.from_ = boost::lexical_cast<float>(*value_cursor);
            } catch(boost::bad_lexical_cast &) {
                g_critical("Invalid float for %s in XML file: %s", *name_cursor, *value_cursor);
                return;
            }
        }
        else if (g_strcmp0(*name_cursor, "to") == 0)
        {
            try {
                rule.to_ = boost::lexical_cast<float>(*value_cursor);
            } catch(boost::bad_lexical_cast &) {
                g_critical("Invalid float for %s in XML file: %s", *name_cursor, *value_cursor);
                return;
            }
        } 
        else
        {
            g_warning("Unknown MIDI binding rule XML attribute: %s with value %s", *name_cursor, *value_cursor);
        }
        name_cursor++;
        value_cursor++;
    }
    if (self->verbose_)
        std::cout << std::endl;
    if (rule.action_ == "")
    {
        g_critical("No action for rule %s", element_name);
        return;
    }
    switch (rule.type_)
    {
        case NOTE_ON_RULE:
            self->note_on_rules_.push_back(rule);
            break;
        case NOTE_OFF_RULE:
            self->note_off_rules_.push_back(rule);
            break;
        case CONTROL_ON_RULE:
            self->control_on_rules_.push_back(rule);
            break;
        case CONTROL_OFF_RULE:
            self->control_off_rules_.push_back(rule);
            break;
        case CONTROL_MAP_RULE:
            self->control_map_rules_.push_back(rule);
            break;
        case PROGRAM_CHANGE_RULE:
            self->program_change_rules_.push_back(rule);
            break;
        case PITCH_WHEEL_RULE:
            self->pitch_wheel_rules_.push_back(rule);
            break;
        default:
            g_critical("Invalid rule type!");
            break;
    }
}

/** 
 * Called on error, including one set by other
 * methods in the vtable. The GError should not be freed.
 */
static void on_midi_xml_error(GMarkupParseContext *context, GError *error, gpointer user_data)
{
    UNUSED(context);
    UNUSED(user_data);
    //MidiBinder *self = static_cast<MidiBinder *>(user_data);
    g_print("Error parsing XML markup: %s\n", error->message);
}

/**
 * Finds a preset file somewhere in the location it could be.
 * 
 * These are the presets dirs in /usr/share/toonloop/presets and the like.
 */
gchar *toon_find_midi_preset_file(const gchar *file_name, bool verbose)
{
    // adding ~/.toonloop/
    std::string config_dir = std::string(std::getenv("HOME")) + "/.toonloop/";
    const gchar *dirs[] ={"", "../presets/", "./presets/", config_dir.c_str(), PRESETS_DIR, NULL};
    int i;
    for (i = 0; dirs[i]; i++)
    {
        if (verbose)
            std::cout << "Looking for " << file_name << " in " << dirs[i] << "..." << std::endl;
        gchar *path = g_strdup_printf("%s%s", dirs[i], file_name);
        if (g_file_test(path, G_FILE_TEST_EXISTS))
            return path;
        g_free(path);
    }
    return NULL;
}
/**
 * Code to load the XML file into memory and parse it. 
 */
bool MidiBinder::load_xml_file(const gchar *file_name)
{
    /* The list of what handler does what. */
    GMarkupParser parser = {
        MidiBinder::on_midi_xml_start_element,
        NULL,
        NULL,
        NULL,
        on_midi_xml_error
    };
    gpointer user_data = (gpointer) this;
    GMarkupParseContext *context = g_markup_parse_context_new(&parser, G_MARKUP_TREAT_CDATA_AS_TEXT, user_data, NULL);
    /* seriously crummy error checking */
    char *xml_text;
    gsize length;
    if (g_file_get_contents(file_name, &xml_text, &length, NULL) == FALSE)
    {
        g_critical("Couldn't load XML\n");
        return false;
    }
    if (g_markup_parse_context_parse(context, xml_text, length, NULL) == FALSE)
    {
        g_critical("Parse failed\n");
        return false;
    }
    g_free(xml_text);
    g_markup_parse_context_free(context);
    return true;
}

// TODO:2010-11-07:aalex:Be able to set MidiBinder verbose or not
MidiBinder::MidiBinder(bool verbose) : 
    verbose_(verbose)
{
    gchar *found = toon_find_midi_preset_file("midi.xml", verbose_);
    if (! found)
        g_error("Could not find XML midi file!");
    std::string full_name(found);
    //if (verbose_)
    std::cout << "Using MIDI bindings file " << full_name << std::endl;
    if (load_xml_file(full_name.c_str()))
    {
        if (verbose_)
            g_print("successfully loaded XML file\n");
    }
}
void MidiBinder::set_verbose(bool verbose)
{
    verbose_ = verbose;
}

