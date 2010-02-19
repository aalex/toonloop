#include <GL/glew.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtk/gtkgl.h>
#include "gltools.h"

void gltools::print_gl_config_attrib(GdkGLConfig *glconfig, const gchar *attrib_str, int attrib, gboolean is_boolean)
{
    int value;
    g_print ("%s = ", attrib_str);
    if (gdk_gl_config_get_attrib(glconfig, attrib, &value))
    {
        if (is_boolean)
        {
            g_print("%s\n", value == TRUE ? "TRUE" : "FALSE");
        } else {
            g_print("%d\n", value);
        } 
    } else {
        g_print("*** Cannot get %s attribute value\n", attrib_str);
    }
}

void gltools::examine_gl_config_attrib(GdkGLConfig *glconfig)
{
  g_print("\nOpenGL visual configurations :\n\n");
  g_print("gdk_gl_config_is_rgba (glconfig) = %s\n", gdk_gl_config_is_rgba (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_is_double_buffered (glconfig) = %s\n", gdk_gl_config_is_double_buffered (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_is_stereo (glconfig) = %s\n", gdk_gl_config_is_stereo (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_has_alpha (glconfig) = %s\n", gdk_gl_config_has_alpha (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_has_depth_buffer (glconfig) = %s\n", gdk_gl_config_has_depth_buffer (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_has_stencil_buffer (glconfig) = %s\n", gdk_gl_config_has_stencil_buffer (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_has_accum_buffer (glconfig) = %s\n",
  gdk_gl_config_has_accum_buffer (glconfig) ? "TRUE" : "FALSE");
  g_print ("\n");
  print_gl_config_attrib(glconfig, "GDK_GL_USE_GL", GDK_GL_USE_GL, TRUE);
  print_gl_config_attrib(glconfig, "GDK_GL_BUFFER_SIZE", GDK_GL_BUFFER_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_LEVEL", GDK_GL_LEVEL, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_RGBA", GDK_GL_RGBA, TRUE);
  print_gl_config_attrib(glconfig, "GDK_GL_DOUBLEBUFFER", GDK_GL_DOUBLEBUFFER, TRUE);
  print_gl_config_attrib(glconfig, "GDK_GL_STEREO", GDK_GL_STEREO, TRUE);
  print_gl_config_attrib(glconfig, "GDK_GL_AUX_BUFFERS", GDK_GL_AUX_BUFFERS, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_RED_SIZE", GDK_GL_RED_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_GREEN_SIZE", GDK_GL_GREEN_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_BLUE_SIZE", GDK_GL_BLUE_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ALPHA_SIZE", GDK_GL_ALPHA_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_DEPTH_SIZE", GDK_GL_DEPTH_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_STENCIL_SIZE", GDK_GL_STENCIL_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ACCUM_RED_SIZE", GDK_GL_ACCUM_RED_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ACCUM_GREEN_SIZE", GDK_GL_ACCUM_GREEN_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ACCUM_BLUE_SIZE", GDK_GL_ACCUM_BLUE_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ACCUM_ALPHA_SIZE", GDK_GL_ACCUM_ALPHA_SIZE, FALSE);
  g_print ("\n");
}
