#include "presets.h"
#include "unused.h"
#include <boost/lexical_cast.hpp>
#include <glib.h>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define VAL(str) #str
#define TOSTRING(str) VAL(str)
#define PRESETS_DIR TOSTRING(DATADIR) "/presets/"

typedef std::vector<MidiRule>::iterator MidiRuleIterator;

struct MidiBinding
{
    public:
        // Name of the Controller's method to call
        gchar *action;
        // TODO: glist *args;
};

#if 0
We could use string commands:

gchar **g_strsplit(const gchar *string, const gchar *delimiter, gint max_tokens);

gchar **words = NULL;
words = g_strsplit("hello 1 2", " ", 10);
g_strfreev(words); // to free it. 

#endif

class MidiBinder
{
    public:
        MidiBinder();
        bool load_xml_file(const gchar *file_name);
        MidiBinding *current_binding;
        MidiRule *get_rule(const gchar *name);
        //void add_rule(const gchar *name);
        //void add_attribute_to_rule(const gchar *rule, const gchar *attr, const gchar *value);
        static void on_midi_xml_start_element(
            GMarkupParseContext *context,
            const gchar *element_name,
            const gchar **attribute_names,
            const gchar **attribute_values,
            gpointer user_data,
            GError **error);
    private:
        std::vector<MidiRule> rules_;
};

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
    // check each name and value for the current tag:
    MidiRule midi_rule;
    midi_rule.name_ = element_name;
    
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;
    std::cout << "Adding rule " << element_name << ": ";
    while (*name_cursor)
    {
        midi_rule.attributes_[*name_cursor] = *value_cursor;
        std::cout << *name_cursor << " " << *value_cursor << "\n";
        name_cursor++;
        value_cursor++;
    }
    self->rules_.push_back(midi_rule);
}

/**
 * Called when some text inside a tag is encountered.
 */
static void on_midi_xml_text(
        GMarkupParseContext *context,
        const gchar         *text,
        gsize                text_len,
        gpointer             user_data,
        GError             **error)
{
    UNUSED(context);
    UNUSED(error);
    MidiBinder *self = static_cast<MidiBinder *>(user_data);
    // Note that "text" is not a regular C string: it is
    // not null-terminated. This is the reason for the
    // unusual %*s format below.
    if (self->current_binding)
        printf("Some text contents: %*s.\n", text_len, text);
}

/**
 * Called for close tags </foo>
 */
static void on_midi_xml_end_element(
        GMarkupParseContext *context,
        const gchar *element_name,
        gpointer user_data,
        GError **error)
{
    UNUSED(context);
    UNUSED(error);
    UNUSED(element_name);
    MidiBinder *self = static_cast<MidiBinder *>(user_data);
    if (self->current_binding)
    {
        //g_free(self->current_animal_noise);
        delete self->current_binding;
        self->current_binding = NULL;
    }
}

  /* Called on error, including one set by other
   * methods in the vtable. The GError should not be freed.
   */
static void on_midi_xml_error(GMarkupParseContext *context, GError *error, gpointer user_data)
{
    UNUSED(context);
    UNUSED(user_data);
    //MidiBinder *self = static_cast<MidiBinder *>(user_data);
    g_print("Error parsing XML markup: %s\n", error->message);
}

MidiBinder::MidiBinder()
{
    //current_animal_noise = NULL;
    current_binding = NULL;
}

gchar *toon_find_midi_preset_file(const gchar *file_name)
{
    // TODO: add ~/.toonloop/
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
void MidiBinder::add_rule(const gchar *name)
{
    MidiRule e;
    e.name_ = name;
    rules_.push_back(e);
}

MidiRule *MidiBinder::get_rule(const gchar *name)
{
    for (MidiRuleIterator iter = rules_.begin(); iter != rules_.end(); ++iter)
        if (iter->name_ == name)
            return &(*iter);
    return NULL;
}
void MidiBinder::add_attribute_to_rule(const gchar *rule, const gchar *attr, const gchar *value)
{

    MidiRule *midi_rule = get_rule(rule);
    if (! midi_rule)
    {
        g_critical("No such rule %s", rule);
        return;
    }
    // if rule in rules_.
    std::cout << "Rule " << rule << ": Adding " << attr << " " << value << std::endl;
    midi_rule->attributes_[attr] = value;
    //std::cout << e.name_ << " " << boost::lexical_cast<int>(e.attributes_["note"]) << std::endl;
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
        on_midi_xml_end_element,
        on_midi_xml_text,
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

