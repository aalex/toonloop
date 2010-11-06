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
#include "presets.h"
#include "unused.h"
#include <boost/lexical_cast.hpp>
#include <glib.h>
#include <iostream>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>

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
    MidiRule midi_rule;
    std::cout << "Adding rule " << element_name << ": ";
    midi_rule.name_ = element_name;
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;
    while (*name_cursor)
    {
        midi_rule.attributes_[*name_cursor] = *value_cursor;
        std::cout << *name_cursor << " " << *value_cursor << "\n";
        name_cursor++;
        value_cursor++;
    }
    self->rules_.push_back(midi_rule);
}

MidiBinder::MidiBinder()
{
    // pass
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
gchar *toon_find_midi_preset_file(const gchar *file_name)
{
    // TODO: add ~/.toonloop/ first
    const gchar *dirs[] ={"", "../presets/", "./presets/", PRESETS_DIR, NULL};
    int i;
    for (i = 0; dirs[i]; i++)
    {
        gchar *path = g_strdup_printf("%s%s", dirs[i], file_name);
        if (g_file_test(path, G_FILE_TEST_EXISTS))
            return path;
        g_free(path);
    }
    return NULL;
}
/*
MidiRule *MidiBinder::get_rule(const gchar *name)
{
    for (MidiRuleIterator iter = rules_.begin(); iter != rules_.end(); ++iter)
        if (iter->name_ == name)
            return &(*iter);
    return NULL;
}
*/
/**
 * Code to grab the file into memory and parse it. 
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
        printf("Couldn't load XML\n");
        return false;
    }
    if (g_markup_parse_context_parse(context, xml_text, length, NULL) == FALSE)
    {
        printf("Parse failed\n");
        return false;
    }
    g_free(xml_text);
    g_markup_parse_context_free(context);
    return true;
}

// TODO: make a class...
void init_midi_presets()
{
    MidiBinder binder = MidiBinder();
    std::string full_name(toon_find_midi_preset_file("midi.xml"));
    std::cout << "Found MIDI bindings file " << full_name << std::endl;
    
    if (binder.load_xml_file(full_name.c_str()))
        g_print("success\n");
}

