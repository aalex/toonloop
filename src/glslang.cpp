#include "config.h"
#include "unused.h"
#include "glslang.h"
#include <clutter/clutter.h>

// Constant for the shaders data directory: (/usr/share/toonloop/shaders/)
#define VAL(str) #str
#define TOSTRING(str) VAL(str)
#define SHADERS_DIR TOSTRING(DATADIR) "/toonloop/shaders/"

/**
 * Loads a fragment shader source from a file.
 */
gboolean toon_load_fragment_source_file(ClutterShader *shader, gchar *file_name)
{
    gchar *contents = NULL;
    gsize length;
    GError *error = NULL;

    if(! g_file_get_contents(file_name, &contents, &length, &error)) 
    {
        g_error_free(error);
        error = NULL;
        return FALSE;
    }
    clutter_shader_set_fragment_source(shader, contents, -1);
    g_free(contents);
    return TRUE;
}

gchar *toon_find_shader_file(const gchar *file_name)
{
    // TODO: add ~/.toonloop/
    const gchar *dirs[] ={"", "./shaders/", "./src/shaders/", SHADERS_DIR, NULL};
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

