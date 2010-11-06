#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

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
};

/*
 * Called for open tags <foo bar="baz">
 */
static void on_midi_xml_start_element(
        GMarkupParseContext *context,
        const gchar *element_name,
        const gchar **attribute_names,
        const gchar **attribute_values,
        gpointer user_data,
        GError **error)
{
    MidiBinder *self = static_cast<MidiBinder *>(user_data);
    // check each name and value for the current tag:
    if (g_strcmp0(element_name, "note_on") == 0)
    {
        self->current_binding = new MidiBinding();
        const gchar **name_cursor = attribute_names;
        const gchar **value_cursor = attribute_values;
        while (*name_cursor)
        {
            if (g_strcmp0(*name_cursor, "action") == 0)
            {
                self->current_binding->action = g_strdup(*value_cursor);
                g_print("Found action %s=%s in %s\n", *name_cursor, *value_cursor, element_name);
            }
            name_cursor++;
            value_cursor++;
        }
    }
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
    //MidiBinder *self = static_cast<MidiBinder *>(user_data);
    g_print("Error parsing XML markup: %s\n", error->message);
}

MidiBinder::MidiBinder()
{
    //current_animal_noise = NULL;
    current_binding = NULL;
}

/**
 * Code to grab the file into memory and parse it. 
 */
bool MidiBinder::load_xml_file(const gchar *file_name)
{
    /* The list of what handler does what. */
    GMarkupParser parser = {
        on_midi_xml_start_element,
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

int main()
{
    MidiBinder binder = MidiBinder();
    std::string file_name = "midibindings.xml";
    if (binder.load_xml_file(file_name.c_str()))
        g_print("success\n");
    return 0;
}
