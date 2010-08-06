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
#include <GL/gl.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <clutter-gst/clutter-gst.h>
#include <clutter-gtk/clutter-gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <iostream>

#include "pipeline.h"
#include "draw.h"
#include "gui.h"
#include "application.h"
#include "config.h"

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
    Clip *current_clip = Application::get_instance().get_current_clip();
    int clip;

    switch (event->keyval)
    {
        case GDK_Up:
            current_clip->increase_playhead_fps();
            break;
        case GDK_Down:
            current_clip->decrease_playhead_fps();
            break;
        //case GDK_Left:
        //case GDK_Right:
        //case GDK_Return:
        case GDK_BackSpace:
            Application::get_instance().get_pipeline().remove_frame();
            break;
        case GDK_Escape:
            context->toggleFullscreen(widget);
            break;
        case GDK_space:
            Application::get_instance().get_pipeline().grab_frame();
            break;
        case GDK_Page_Up:
            clip = Application::get_instance().get_current_clip_number();
            if (clip < MAX_CLIPS - 1)
                Application::get_instance().set_current_clip_number(clip + 1);
            current_clip = Application::get_instance().get_current_clip();
            break;
        case GDK_Page_Down:
            clip = Application::get_instance().get_current_clip_number();
            if (clip > 0)
                Application::get_instance().set_current_clip_number(clip - 1);
            current_clip = Application::get_instance().get_current_clip();
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


void Gui::video_widget_realize_cb (GtkWidget * widget, gpointer data)
{
    Gui *context = static_cast<Gui*>(data);
#if GTK_CHECK_VERSION(2,18,0)
    // This is here just for pedagogical purposes, GDK_WINDOW_XID will call
    // it as well in newer Gtk versions
    /* if (!gdk_window_ensure_native (widget->window))
           g_error ("Couldn't create native window needed for GstXOverlay!");
    */       
#endif

#ifdef GDK_WINDOWING_X11
    context->video_xwindow_id_ = GDK_WINDOW_XID(widget->window);
    g_print("Drawing area has been realized with xid %lu.\n", (long unsigned int) context->video_xwindow_id_);
#endif
}

/**
 * Exits the application if OpenGL needs are not met.
 */
Gui::Gui() :
    isFullscreen_(false)
{
    video_xwindow_id_ = 0;
    // Main GTK window
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window_, 640, 480); // TODO: make configurable
    gtk_window_move(GTK_WINDOW(window_), 300, 10); // TODO: make configurable
    gtk_window_set_title(GTK_WINDOW(window_), std::string(std::string("Toonloop ") + std::string(PACKAGE_VERSION) + std::string(" experimental")).c_str());
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
    g_signal_connect(drawing_area_, "realize", G_CALLBACK(video_widget_realize_cb), this);

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

