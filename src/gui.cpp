#include <GL/glew.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtk/gtkgl.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>

#include "gltools.h"
#include "draw.h"

/**
 * Sets up the orthographic projection.
 * 
 * Makes sure height is always 1.0 in GL modelview coordinates.
 * 
 * Coordinates should give a rendering area height of 1
 * and a width of 1.33, when in 4:3 ratio.
*/
void _set_view(float ratio)
{
    float w = ratio;
    float h = 1.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w, w, -h, h, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void on_realize(GtkWidget *widget, gpointer data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  
    /*** OpenGL BEGIN ***/
    if (! gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
        return;
    }
    glDisable(GL_DEPTH_TEST);
  
    glClearColor(0.0, 0.0, 0.0, 1.0); // black background
    glViewport(0, 0, widget->allocation.width, widget->allocation.height);
    _set_view(widget->allocation.width / float(widget->allocation.height));
    gdk_gl_drawable_gl_end(gldrawable);
    /*** OpenGL END ***/
}

static gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
    /*** OpenGL BEGIN ***/
    if (! gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
        return FALSE;
    }
    glViewport(0, 0, widget->allocation.width, widget->allocation.height);
    _set_view(widget->allocation.width / float(widget->allocation.height));
    gdk_gl_drawable_gl_end(gldrawable);
    /*** OpenGL END ***/
    return TRUE;
}

// draws the stuff
void _draw()
{
        // DRAW STUFF HERE
        glDisable(GL_TEXTURE_RECTANGLE_ARB);
        glColor4f(1.0, 0.8, 0.2, 1.0);
        glPushMatrix();
        glScalef(0.5, 0.5, 1.0);
        draw::draw_square();
        glPopMatrix();

        glColor4f(1.0, 1.0, 0.0, 0.8);
        int num = 64;
        float x;
        for (int i = 0; i < num; i++)
        {
            x = (i / float(num)) * 4 - 2;
            draw::draw_line(float(x), -2.0, float(x), 2.0);
            draw::draw_line(-2.0, float(x), 2.0, float(x));
        }
}

static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    /*** OpenGL BEGIN ***/
    if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
      return FALSE;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _draw();

    if (gdk_gl_drawable_is_double_buffered(gldrawable))
    {
        gdk_gl_drawable_swap_buffers(gldrawable);
    } else {
        glFlush();
    }
    gdk_gl_drawable_gl_end(gldrawable);
    /*** OpenGL END ***/
    return TRUE;
}

static void on_delete_event(GtkWidget* widget, GdkEvent* event, void* data)
{
    g_print("Close\n");
    gtk_main_quit();
}

void run_gui(gint argc, gchar* argv[])
{
    gtk_init(&argc, &argv);
    // Init GTK GL:
    gtk_gl_init(&argc, &argv);
    gint major; 
    gint minor;
    gdk_gl_query_version(&major, &minor);
    g_print("\nOpenGL extension version - %d.%d\n", major, minor);
    /* Try double-buffered visual */

    GdkGLConfig* glconfig;
    // the line above does not work in C++ if the cast is not there.
    glconfig = gdk_gl_config_new_by_mode(static_cast<GdkGLConfigMode>(GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE));
    if (glconfig == NULL)
    {
        g_print("*** Cannot find the double-buffered visual.\n");
        g_print("*** Trying single-buffered visual.\n");
        /* Try single-buffered visual */
        glconfig = gdk_gl_config_new_by_mode(static_cast<GdkGLConfigMode>(GDK_GL_MODE_RGB));
        if (glconfig == NULL)
        {
            g_print ("*** No appropriate OpenGL-capable visual found.\n");
            exit(1);
        }
    }
    gltools::examine_gl_config_attrib(glconfig);

    // Main GTK window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window, 640, 480);
    gtk_window_set_title(GTK_WINDOW (window), "Toonloop 1.3 experimental");
    GdkGeometry geometry;
    geometry.min_width = 1;
    geometry.min_height = 1;
    geometry.max_width = -1;
    geometry.max_height = -1;
    gtk_window_set_geometry_hints(GTK_WINDOW(window), window, &geometry, GDK_HINT_MIN_SIZE);
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(on_delete_event), NULL);

    //area where the video is drawn
    GtkWidget* drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    //avoid flickering when resizing or obscuring the main window
    //gtk_widget_realize(drawing_area);
    //gdk_window_set_back_pixmap(drawing_area->window, NULL, FALSE);
    //gtk_widget_set_app_paintable(drawing_area, TRUE);
    //gtk_widget_set_double_buffered(drawing_area, FALSE);

    /* Set OpenGL-capability to the widget. */
    gtk_widget_set_gl_capability(drawing_area, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
  
    g_signal_connect_after(G_OBJECT(drawing_area), "realize", G_CALLBACK(on_realize), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "configure_event", G_CALLBACK(on_configure_event), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "expose_event", G_CALLBACK(on_expose_event), NULL);

    gtk_widget_show_all(window);
    gtk_main();
}
