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
#include <clutter-gst/clutter-gst.h>
#include <clutter-gtk/clutter-gtk.h>
#include <clutter/clutter.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <iostream>

#include "pipeline.h"
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
    // FIXME:2010-08-06:aalex:Hiding the cursor is currently broken
    char invisible_cursor_bits[] = { 0x0 };
    static GdkCursor* cursor = 0;
    if (cursor == 0)
    {
        static GdkBitmap *empty_bitmap;
        const static GdkColor color = {0, 0, 0, 0};
        empty_bitmap = gdk_bitmap_create_from_data(GDK_WINDOW(clutter_widget_->window), invisible_cursor_bits, 1, 1);
        cursor = gdk_cursor_new_from_pixmap(empty_bitmap, empty_bitmap, &color, &color, 0, 0);
    }
    gdk_window_set_cursor(GDK_WINDOW(clutter_widget_->window), cursor);
}

void Gui::showCursor()
{
    /// sets to default
    gdk_window_set_cursor(GDK_WINDOW(clutter_widget_->window), NULL);
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

/** 
 * Timeline handler.
 * Called on every frame. 
 */
void on_new_frame(ClutterTimeline *timeline, gint msecs, gpointer data)
{
//    std::cout << "on_new_frame" << std::endl; 
//     Gui *gui = (Gui*)data;
}

void on_stage_allocation_changed(ClutterActor *stage, ClutterActorBox *box, ClutterAllocationFlags *flags, gpointer user_data) {
    //g_print("on_stage_allocation_changed\n");
    Gui *gui = static_cast<Gui*>(user_data);
    gui->resize_actors();
}

void Gui::resize_actors() {
    gfloat set_x, set_y, set_width, set_height;
    gfloat stage_width, stage_height;

    clutter_actor_get_size(stage_, &stage_width, &stage_height);
    set_height = (video_input_height_ * stage_width) / video_input_width_;
    if (set_height <= stage_height) {
        set_width = stage_width;
        set_x = 0;
        set_y = (stage_height - set_height) / 2;
    } else {
        set_width  = (video_input_width_ * stage_height) / video_input_height_;
        set_height = stage_height;
        set_x = (stage_width - set_width) / 2;
        set_y = 0;
    }
    clutter_actor_set_position(CLUTTER_ACTOR(live_input_texture_), set_x, set_y);
    clutter_actor_set_size(CLUTTER_ACTOR(live_input_texture_), set_width, set_height);
}

void on_texture_size_changed(ClutterTexture *texture, gfloat width, gfloat height, gpointer user_data) {
    //g_print("on_texture_size_changed\n");
    Gui *gui = static_cast<Gui*>(user_data);
    gui->video_input_width_ = (float) width;
    gui->video_input_height_ = (float) height;

    ClutterActor *stage;
  
    stage = clutter_actor_get_stage(CLUTTER_ACTOR(texture));
    if (stage == NULL)
        return;
    gui->resize_actors();
}
/**
 * Exits the application if OpenGL needs are not met.
 */
Gui::Gui() :
    video_input_height_(1),
    video_input_width_(1),
    isFullscreen_(false)
{
    //video_xwindow_id_ = 0;
    // Main GTK window
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // TODO:2010-08-06:aalex:make window size configurable
    gtk_widget_set_size_request(window_, WINWIDTH, WINHEIGHT); 
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

    // vbox:
    vbox_ = gtk_vbox_new(FALSE, 6); // homogeneous, spacing
    gtk_container_add(GTK_CONTAINER(window_), vbox_);
    // Clutter widget:
    clutter_widget_ = gtk_clutter_embed_new();
    gtk_widget_set_size_request(clutter_widget_, WINWIDTH, WINHEIGHT);
    gtk_container_add(GTK_CONTAINER(vbox_), clutter_widget_);
    stage_ = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(clutter_widget_));


    clutter_stage_set_user_resizable(CLUTTER_STAGE(stage_), TRUE);
    g_signal_connect(stage_, "allocation-changed", G_CALLBACK(on_stage_allocation_changed), this);
    clutter_stage_set_minimum_size(CLUTTER_STAGE(stage_), 320, 240);

    /* We need to set certain props on the target texture currently for
     * efficient/corrent playback onto the texture (which sucks a bit)  
    */
    live_input_texture_ = (ClutterActor *) g_object_new(CLUTTER_TYPE_TEXTURE, "sync-size", FALSE, "disable-slicing", TRUE, NULL);
    g_signal_connect(CLUTTER_TEXTURE(live_input_texture_), "size-change", G_CALLBACK(on_texture_size_changed), this);

    // Background color:
    ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };
    clutter_stage_set_color(CLUTTER_STAGE(stage_), &stage_color);

    /* Create a timeline to manage animation */
    timeline_ = clutter_timeline_new(6000);
    g_object_set(timeline_, "loop", TRUE, NULL);   /* have it loop */
    /* fire a callback for frame change */
    g_signal_connect(timeline_, "new-frame", G_CALLBACK(on_new_frame), this);
    /* and start it */
    clutter_timeline_start(timeline_);

    /* of course, we need to show the texture in the stage. */

    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(live_input_texture_));
  
    gtk_widget_show_all(window_);

    /* Only show the actors after parent show otherwise it will just be
     * unrealized when the clutter foreign window is set. widget_show
     * will call show on the stage.
     */
    clutter_actor_show_all(CLUTTER_ACTOR(live_input_texture_));
}

ClutterActor* Gui::get_live_input_texture()
{
    return live_input_texture_;
}

