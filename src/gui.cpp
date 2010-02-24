#include <GL/glew.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtk/gtkgl.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <gdk/gdkkeysyms.h>

#include "gltools.h"
#include "pipeline.h"
#include "draw.h"
#include "gui.h"
#include "application.h"


gboolean Gui::onWindowStateEvent(GtkWidget* widget, GdkEventWindowState *event, gpointer data)
{
    Gui *context = static_cast<Gui*>(data);
    context->isFullscreen_ = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    if (context->isFullscreen_)
        context->hideCursor();
    else
        context->showCursor();
    return TRUE;
}

void Gui::hideCursor()
{
    // FIXME: this is because gtk doesn't support GDK_BLANK_CURSOR before gtk-2.16
    char invisible_cursor_bits[] = { 0x0 };
    static GdkCursor* cursor = 0;
    if (cursor == 0)
    {
        static GdkBitmap *empty_bitmap;
        const static GdkColor color = {0, 0, 0, 0};
        empty_bitmap = gdk_bitmap_create_from_data(GDK_WINDOW(drawing_area_->window), invisible_cursor_bits, 1, 1);
        cursor = gdk_cursor_new_from_pixmap(empty_bitmap, empty_bitmap, &color, &color, 0, 0);
    }
    gdk_window_set_cursor(GDK_WINDOW(drawing_area_->window), cursor);
}

void Gui::showCursor()
{
    /// sets to default
    gdk_window_set_cursor(GDK_WINDOW(drawing_area_->window), NULL);
}

gboolean Gui::key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    Gui *context = static_cast<Gui*>(data);
    switch (event->keyval)
    {
        case GDK_Escape:
            context->toggleFullscreen(widget);
            break;
        case GDK_q:
            // Quit application on ctrl-q, this quits the main loop
            // (if there is one)
            if (event->state & GDK_CONTROL_MASK)
            {
                g_print("Ctrl-Q key pressed, quitting.\n");
                Application::get_instance().quit();
            }
            break;
        default:
            break;
    }
    return TRUE;
}

void Gui::on_delete_event(GtkWidget* widget, GdkEvent* event, gpointer data)
{
    Gui *context = static_cast<Gui*>(data);
    g_print("Window has been deleted.\n");
    Application::get_instance().quit();
}

void Gui::toggleFullscreen(GtkWidget *widget)
{
    // toggle fullscreen state
    isFullscreen_ ? makeUnfullscreen(widget) : makeFullscreen(widget);
}

void Gui::makeFullscreen(GtkWidget *widget)
{
    gtk_window_stick(GTK_WINDOW(widget)); // window is visible on all workspaces
    gtk_window_fullscreen(GTK_WINDOW(widget));
}

void Gui::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unstick(GTK_WINDOW(widget)); // window is not visible on all workspaces
    gtk_window_unfullscreen(GTK_WINDOW(widget));
}

/**
 * Exits the application if OpenGL needs are not met.
 */
Gui::Gui() :
    isFullscreen_(false)
{

    //glx_context_ = NULL;
    //gint major; 
    //gint minor;
    //gdk_gl_query_version(&major, &minor);
    //g_print("\nOpenGL extension version - %d.%d\n", major, minor);
    ///* Try double-buffered visual */

    //GdkGLConfig* glconfig;
    //// the line above does not work in C++ if the cast is not there.
    //glconfig = gdk_gl_config_new_by_mode(static_cast<GdkGLConfigMode>(GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE));
    //if (glconfig == NULL)
    //{
    //    g_print("*** Cannot find the double-buffered visual.\n");
    //    g_print("*** Trying single-buffered visual.\n");
    //    /* Try single-buffered visual */
    //    glconfig = gdk_gl_config_new_by_mode(static_cast<GdkGLConfigMode>(GDK_GL_MODE_RGB));
    //    if (glconfig == NULL)
    //    {
    //        g_print ("*** No appropriate OpenGL-capable visual found.\n");
    //        exit(1);
    //    }
    //}
    //gltools::examine_gl_config_attrib(glconfig);
    // Main GTK window
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window_, 640, 480);
    gtk_window_move (GTK_WINDOW (window_), 300, 10);
    gtk_window_set_title(GTK_WINDOW (window_), "Toonloop 1.3 experimental");
    GdkGeometry geometry;
    geometry.min_width = 1;
    geometry.min_height = 1;
    geometry.max_width = -1;
    geometry.max_height = -1;
    gtk_window_set_geometry_hints(GTK_WINDOW(window_), window_, &geometry, GDK_HINT_MIN_SIZE);
    // connect window signals:
    g_signal_connect(G_OBJECT(window_), "delete-event", G_CALLBACK(on_delete_event), this);
    g_signal_connect(G_OBJECT(window_), "key-press-event", G_CALLBACK(key_press_event), this);
    // add listener for window-state-event to detect fullscreenness
    g_signal_connect(G_OBJECT(window_), "window-state-event", G_CALLBACK(onWindowStateEvent), this);

    //area where the video is drawn
    drawing_area_ = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window_), drawing_area_);

    //avoid flickering when resizing or obscuring the main window
    gtk_widget_realize(drawing_area_);
    gdk_window_set_back_pixmap(drawing_area_->window, NULL, FALSE);
    gtk_widget_set_app_paintable(drawing_area_, TRUE);
    gtk_widget_set_double_buffered(drawing_area_, FALSE);
  
    gtk_widget_show_all(window_);
}

GtkWidget* Gui::get_drawing_area()
{
    return drawing_area_;
}

