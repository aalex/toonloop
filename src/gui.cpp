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
#include "timer.h"

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
    //Gui *context = static_cast<Gui*>(data);
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

void iterate_playhead()
{
    Gui gui = Application::get_instance().get_gui();
    Pipeline pipeline = Application::get_instance().get_pipeline();
    
    static int number_of_frames_in_last_second = 0; // counting FPS
    static bool first_draw = true;
    static int prev_image_number = -1;
    static Clip *prevclip = NULL;
    static Timer fps_calculation_timer = Timer();
    static Timer playback_timer = Timer(); // TODO: move to Clip
//    static GLuint frametexture;
//    GLint texturelocation;
    bool move_playhead = false;
    Clip *thisclip = Application::get_instance().get_current_clip();
    bool need_refresh = false;
//
//    thisclip->lock_mutex();
//
    ++ number_of_frames_in_last_second;
    playback_timer.tick();
    fps_calculation_timer.tick();

    // check if it is time to move the playhead
    if ((playback_timer.get_elapsed()) >=  (1.0f / thisclip->get_playhead_fps() * 1.0) || playback_timer.get_elapsed() < 0.0f)
    {
        move_playhead = true;
        playback_timer.reset();
    }
    // calculate rendering FPS
    if (fps_calculation_timer.get_elapsed() >= 1.0f)
    {
        std::cout << "Rendering FPS: " << number_of_frames_in_last_second << std::endl;
        number_of_frames_in_last_second = 0;
        fps_calculation_timer.reset();
    }
    
    if(thisclip->size() > 0) 
    {     
        if (! CLUTTER_ACTOR_IS_VISIBLE(gui.playback_texture_))
            clutter_actor_show_all(CLUTTER_ACTOR(gui.playback_texture_));
    } else {
        if (CLUTTER_ACTOR_IS_VISIBLE(gui.playback_texture_))
            clutter_actor_hide_all(CLUTTER_ACTOR(gui.playback_texture_));
    }
    if(thisclip->size() > 0) 
    {     
            
        // FIXME: we don't need to create a texture on every frame!!
        //double spf = (1 / thisclip->get_playhead_fps());
        if (move_playhead) // if it's time to move the playhead
            thisclip->iterate_playhead(); // updates the clip's playhead number
        int image_number = thisclip->get_playhead();
        //bool pixels_are_loaded = false;
        //width = thisclip->get_width(); // FIXME: do not override data given by gst-plugins-gl!
        //height = thisclip->get_height();// FIXME: do not override data given by gst-plugins-gl!
        char *buf;
        //GdkPixbuf *pixbuf;
        //bool loaded_pixbuf = false;
        Image* thisimage = &(thisclip->get_image(image_number));

        if ( (prevclip != thisclip) || (prev_image_number != image_number) )
              need_refresh = true;
        if (prevclip != thisclip) {
            prevclip = thisclip;
        }
        if (thisimage == NULL)
        {
            std::cout << "No image at index" << image_number << "." << std::endl;
        } else {
            /*FIXME: we may not need this dimension update in general. But I get a weirdly cropped frame, for the right side rendering grabbed frame. It seems
            the grabbed frame dimensions don't match with the width, height passed from glimasesink to the draw callback*/
            // XXX: yes, I think we should always check the size of the images. (especially when we will read them from the disk)

            if (thisimage->is_ready() and need_refresh)
            {
                if (Application::get_instance().get_configuration().get_images_in_ram())
                {   
                    std::cerr << "Loading in RAM is disabled for now." << std::endl; 
                    //if ( need_refresh )
                    //    buf = thisimage->get_rawdata();
                    //pixels_are_loaded = true;
                } else {
                    std::string image_full_path = Application::get_instance().get_pipeline().get_image_full_path(thisimage);
                    GError *error = NULL;
                    gboolean success;
                    success = clutter_texture_set_from_file(CLUTTER_TEXTURE(gui.playback_texture_), image_full_path.c_str(), &error);
                    // TODO: validate this path
                    
                    //pixbuf = gdk_pixbuf_new_from_file(image_full_path.c_str(), &error);
                    if (!success)
                    {
                        std::cerr << "Failed to load pixbuf file: " << image_full_path << " " << error->message << std::endl;
                        g_error_free(error);
                    } else {
                        // TODO:2010-08-06:aalex:Do not load an image twice in a row
                        //std::cout << "Loaded image " <<  image_full_path << std::endl;
                        //buf = (char*) gdk_pixbuf_get_pixels(pixbuf);
                        //pixels_are_loaded = true;
                        //loaded_pixbuf = true;
                    }
                }
            }
        }
        prev_image_number = image_number;
    } 
}

/** 
 * Timeline handler.
 * Called on every frame. 
 */
void on_new_frame(ClutterTimeline *timeline, gint msecs, gpointer data)
{
    //std::cout << "on_new_frame" << std::endl; 
//     Gui *gui = (Gui*)data;
    iterate_playhead();
}

/**
 * Called when the stage size has changed.
 */
void on_stage_allocation_changed(ClutterActor *stage, ClutterActorBox *box, ClutterAllocationFlags *flags, gpointer user_data) {
    //g_print("on_stage_allocation_changed\n");
    Gui *gui = static_cast<Gui*>(user_data);
    gui->resize_actors();
}
/**
 * Called when it's time to resize the textures in the stage.
 *
 * Either the window has been resized, or the input image size has changed.
 */
void Gui::resize_actors() {
    // We could override the paint method of the stage
    // Or put everything in a container which has an apply_transform()
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
    // Now that we know the ratio of stuff, 
    // Set the live texture size and position:
    gfloat live_tex_width = set_width / 2;
    gfloat live_tex_height = set_height / 2;
    gfloat live_tex_x = set_x;
    gfloat live_tex_y = (stage_height / 4);
    clutter_actor_set_position(CLUTTER_ACTOR(live_input_texture_), live_tex_x, live_tex_y);
    clutter_actor_set_size(CLUTTER_ACTOR(live_input_texture_), live_tex_width, live_tex_height);

    // Set the playback texture size and position:
    gfloat playback_tex_width = set_width / 2;
    gfloat playback_tex_height = set_height / 2;
    gfloat playback_tex_x = (stage_width / 2);
    gfloat playback_tex_y = (stage_height / 4);
    clutter_actor_set_position(CLUTTER_ACTOR(playback_texture_), playback_tex_x, playback_tex_y);
    clutter_actor_set_size(CLUTTER_ACTOR(playback_texture_), playback_tex_width, playback_tex_height);
}
/**
 * Called when the size of the input image has changed.
 */
void on_live_input_texture_size_changed(ClutterTexture *texture, gfloat width, gfloat height, gpointer user_data) {
    //g_print("on_live_input_texture_size_changed\n");
    Gui *gui = static_cast<Gui*>(user_data);
    gui->video_input_width_ = (float) width;
    gui->video_input_height_ = (float) height;

    ClutterActor *stage;
  
    stage = clutter_actor_get_stage(CLUTTER_ACTOR(texture));
    if (stage == NULL)
        return;
    gui->resize_actors();
}

void on_playback_texture_size_changed(ClutterTexture *texture, gfloat width, gfloat height, gpointer user_data) {
    //g_print("on_playback_texture_size_changed\n");
    // TODO:2010-08-06:aalex:Take into account size and ratio of the playback texture
    Gui *gui = static_cast<Gui*>(user_data);
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
    vbox_ = gtk_vbox_new(FALSE, 0); // args: homogeneous, spacing
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
    live_input_texture_ = (ClutterActor *) g_object_new(CLUTTER_TYPE_TEXTURE, 
        "sync-size", FALSE, 
        "disable-slicing", TRUE, 
        NULL);
    g_signal_connect(CLUTTER_TEXTURE(live_input_texture_), "size-change", G_CALLBACK(on_live_input_texture_size_changed), this);

    playback_texture_ = (ClutterActor *) g_object_new(CLUTTER_TYPE_TEXTURE, 
        "sync-size", FALSE, 
        "disable-slicing", TRUE, 
        NULL);
    g_signal_connect(CLUTTER_TEXTURE(playback_texture_), "size-change", G_CALLBACK(on_playback_texture_size_changed), this);

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
    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(playback_texture_));
  
    gtk_widget_show_all(window_);

    /* Only show the actors after parent show otherwise it will just be
     * unrealized when the clutter foreign window is set. widget_show
     * will call show on the stage.
     */
    clutter_actor_show_all(CLUTTER_ACTOR(live_input_texture_));
    //clutter_actor_show_all(CLUTTER_ACTOR(playback_texture_));
}

ClutterActor* Gui::get_live_input_texture()
{
    return live_input_texture_;
}

