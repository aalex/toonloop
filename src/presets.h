#ifndef __PRESETS_H__
#define __PRESETS_H__

#include <glib.h>
#include <map>
#include <string>
#include <vector>

struct MidiRule {

    std::string name_;
    std::map<std::string, std::string> attributes_;
};

void init_midi_presets();

// Constant for the presets data directory: (/usr/share/toonloop/presets/)
#define VAL(str) #str
#define TOSTRING(str) VAL(str)
#define PRESETS_DIR TOSTRING(DATADIR) "/presets/"

typedef std::vector<MidiRule>::iterator MidiRuleIterator;

class MidiBinder
{
    public:
        MidiBinder();
        bool load_xml_file(const gchar *file_name);
        //MidiBinding *current_binding;
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

#endif
