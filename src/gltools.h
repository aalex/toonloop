#ifndef __GLTOOLS_H__
#define __GLTOOLS_H__

#include <gtk/gtkgl.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtk/gtkgl.h>

namespace gltools 
{
    void print_gl_config_attrib(GdkGLConfig *glconfig, const gchar *attrib_str, int attrib, gboolean is_boolean);

    void examine_gl_config_attrib(GdkGLConfig *glconfig);
}

#endif // __GLTOOLS_H__

