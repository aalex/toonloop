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

/**
 * Graphical user interface made with Clutter 
 */
#include <boost/filesystem.hpp>
#include <clutter-gst/clutter-gst.h>
#include <clutter-gtk/clutter-gtk.h>
#include <clutter/clutter.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <iostream>

#include "application.h"
#include "clip.h"
#include "config.h"
#include "controller.h"
#include "gui.h"
#include "pipeline.h"
#include "timer.h"

namespace fs = boost::filesystem;
using std::tr1::shared_ptr;
typedef std::vector<ClutterActor*>::iterator ActorIterator;

gboolean Gui::on_mouse_button_event(GtkWidget* /* widget */, GdkEventButton *event, gpointer user_data)
{
    Gui *context = static_cast<Gui *>(user_data);
    if (event->type == GDK_BUTTON_PRESS)
    {
        context->owner_->get_controller()->add_frame();
        //context->owner_->get_controller()->enable_video_grabbing(true);
        // if (event->button == 1) // left click
        // {
        //     if (context->owner_->get_configuration()->get_mouse_controls_enabled())
        //         context->owner_->get_controller()->add_frame();
        // }
        // else if (event->button == 2) // right click
        // {
        //     if (context->owner_->get_configuration()->get_mouse_controls_enabled())
        //         context->owner_->get_controller()->add_frame();
        // }
    }
    else if (event->type == GDK_BUTTON_RELEASE)
    {
        //context->owner_->get_controller()->enable_video_grabbing(false);
        //if (event->button == 1)
        //{
        //    //std::cout << "Left mouse button clicked" << std::endl;
        //    if (context->owner_->get_configuration()->get_mouse_controls_enabled())
        //        context->owner_->get_controller()->add_frame();
        //}
        //else if (event->button == 2)
        //{
        //    //std::cout << "Right mouse button clicked" << std::endl;
        //    //std::cout << "Left mouse button clicked" << std::endl;
        //    if (context->owner_->get_configuration()->get_mouse_controls_enabled())
        //        context->owner_->get_controller()->add_frame();
        //}
    }
    return TRUE;
}

void Gui::set_overlay_opacity(int value)
{
    overlay_opacity_ = value;
    if (owner_->get_configuration()->get_verbose())
        std::cout << "overlay opacity: " << overlay_opacity_ << std::endl;
    if (current_layout_ == LAYOUT_OVERLAY)
        clutter_actor_set_opacity(CLUTTER_ACTOR(live_input_texture_), overlay_opacity_);
}

void Gui::enable_onionskin(bool value)
{
    onionskin_enabled_ = value;
    // Now does the stuff. Only need to call either one of these two methods
    set_onionskin_opacity(onionskin_opacity_);
}

void Gui::set_onionskin_opacity(int value)
{
    onionskin_opacity_ = value;
    if (owner_->get_configuration()->get_verbose())
    {
        std::cout << "onionskin enabled: " << onionskin_enabled_ << std::endl;
        std::cout << "onionskin opacity: " << onionskin_opacity_ << std::endl;
    }
    if (onionskin_enabled_)
        clutter_actor_set_opacity(CLUTTER_ACTOR(onionskin_textures_.at(0)), onionskin_opacity_);
    else
        clutter_actor_set_opacity(CLUTTER_ACTOR(onionskin_textures_.at(0)), 0);
}
/**
 * In fullscreen mode, hides the cursor. In windowed mode, shows the cursor.
 */
gboolean Gui::on_window_state_event(GtkWidget* /*widget*/, GdkEventWindowState *event, gpointer user_data)
{
    Gui *context = static_cast<Gui*>(user_data);
    context->isFullscreen_ = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    if (context->isFullscreen_)
        context->hideCursor();
    else
        context->showCursor();
    return TRUE;
}
/**
 * In fullscreen mode, hides the cursor.
 */
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

/**
 * In windowed mode, shows the cursor.
 */
void Gui::showCursor()
{
    /// sets to default
    gdk_window_set_cursor(GDK_WINDOW(clutter_widget_->window), NULL);
}

/**
 * Handles key pressed event.
 *
 * - Up: increase playhead FPS
 * - Down: decrease playhead FPS
 * - Backscape: remove a frame
 * - Escape or f: toggle full screen mode
 * - Space: add a frame
 * - Page Up: choose next clip
 * - Page Down: choose previous clip
 * - 0, 1, 2, 3, 4, 5, 6, 7, 8, 9: choose a clip
 * - Ctrl-q: quit
 * - s: save the current clip
 * - period: toggle the layout
 * - Tab: changes the playback direction
 * - Caps_Lock: Toggle video grabbing on/off
 * - a: Toggles on/off the intervalometer
 * - k: increase intervalometer interval by 1 second
 * - j: decrease intervalometer interval by 1 second
 * - right: move writehead to the next image
 * - left: move writehead to the previous image
 * - return: move writehead to the last image
 * - semicolon: move writehead to the first image
 * - (): increase/decrease fading between images
 * - o: toggles onion skinning
 * - []: increase/decrease opacity of the live input image in the overlay layout.
 * - TODO: Ctrl-s: save the whole project
 */

gboolean Gui::key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    Gui *context = static_cast<Gui*>(user_data);

    switch (event->keyval)
    {
        case GDK_Caps_Lock:
        {
            if ((event->state & GDK_LOCK_MASK) != 0)
            {
                std::cout << "Caps_Lock off" << std::endl;
                context->owner_->get_controller()->enable_video_grabbing(false);
            } else {
                std::cout << "Caps_Lock on" << std::endl;
                context->owner_->get_controller()->enable_video_grabbing(true);
            }
            break;
        }
        case GDK_Up:
            context->owner_->get_controller()->increase_playhead_fps();
            break;
        case GDK_Down:
            context->owner_->get_controller()->decrease_playhead_fps();
            break;
        //case GDK_Left:
        //    context->owner_->get_controller()->set_current_clip_direction(DIRECTION_BACKWARD);
        //    break;
        //case GDK_Right:
        //    context->owner_->get_controller()->set_current_clip_direction(DIRECTION_FORWARD);
        //    break;
        case GDK_Tab:
            context->owner_->get_controller()->change_current_clip_direction();
            break;
        case GDK_period:
            //TODO:2010-08-27:aalex:Create Controller:toggle_layout
            context->toggle_layout();
            break;
        case GDK_r:
            context->owner_->get_controller()->clear_current_clip();
            break;
        case GDK_BackSpace:
            context->owner_->get_controller()->remove_frame();
            break;
        case GDK_f:
        case GDK_Escape:
            context->toggleFullscreen(widget);
            break;
        case GDK_space:
            context->owner_->get_controller()->add_frame();
            break;
        case GDK_Page_Up:
            context->owner_->get_controller()->choose_previous_clip();
            break;
        case GDK_Page_Down:
            context->owner_->get_controller()->choose_next_clip();
            break;
        case GDK_0:
        case GDK_1:
        case GDK_2:
        case GDK_3:
        case GDK_4:
        case GDK_5:
        case GDK_6:
        case GDK_7:
        case GDK_8:
        case GDK_9:
        {   // need to use brackets when declaring variable inside case
            //* Switch the current clip according to a gdk key value from 0 to 9
            //* keyval should be one of :
            //* GDK_0 GDK_1 GDK_2 GDK_3 GDK_4 GDK_5 GDK_6 GDK_7 GDK_8 GDK_9
            //* Of course, any other value might lead to a crash.
            // FIXME:2010-08-17:aalex:Doing arithmetics with a gdk keyval is a hack
            unsigned int index = (event->keyval & 0x0F);
            context->owner_->get_controller()->choose_clip(index);
            break;
        }
        case GDK_q:
            // Quit application on ctrl-q, this quits the main loop
            // (if there is one)
            if (event->state & GDK_CONTROL_MASK)
            {
                g_print("Ctrl-Q key pressed, quitting.\n");
                context->owner_->quit();
            }
            break;
        case GDK_s:
            // Ctrl-s: Saves the current clip, this quits the main loop
            // (if there is one)
            if (event->state & GDK_CONTROL_MASK)
            {
                g_print("Ctrl-S key pressed, TODO: save the whole project.\n");
            } else // no Ctrl pressed
                context->owner_->get_controller()->save_current_clip();
            break;
        case GDK_a:
            //std::cout << "Toggle intervalometer." << std::endl; 
            context->owner_->get_controller()->toggle_intervalometer();
            break;
        case GDK_k:
            context->owner_->get_controller()->increase_intervalometer_rate();
            break;
        case GDK_j:
            context->owner_->get_controller()->decrease_intervalometer_rate();
            break;
        case GDK_Left:
            context->owner_->get_controller()->move_writehead_to_previous();
            break;
        case GDK_Right:
            context->owner_->get_controller()->move_writehead_to_next();
            break;
        case GDK_Return:
            context->owner_->get_controller()->move_writehead_to_last();
            break;
        case GDK_semicolon:
            context->owner_->get_controller()->move_writehead_to_first();
            break;
        case GDK_bracketleft:
            if (context->overlay_opacity_ > 0)
                context->set_overlay_opacity(context->overlay_opacity_ - 1);
            break;
        case GDK_bracketright:
            if (context->overlay_opacity_ < 255)
                context->set_overlay_opacity(context->overlay_opacity_ + 1);
            break;
        case GDK_parenleft:
            if (context->duration_ratio_ > 0.0f)
            {
                context->duration_ratio_ = context->duration_ratio_ - 0.1;
                std::cout << "Duration ratio: " << context->duration_ratio_ << std::endl;
            }
            break;
        case GDK_parenright:
            if (context->duration_ratio_ < 10.0f)
            {
                context->duration_ratio_ = context->duration_ratio_ + 0.1f;
                std::cout << "Duration ratio: " << context->duration_ratio_ << std::endl;
            }
            break;
        case GDK_i:
            context->enable_hud_ = ! context->enable_hud_;
            if (context->enable_hud_)
                clutter_actor_show(context->info_text_actor_);
            else
                clutter_actor_hide(context->info_text_actor_);
            break;
        case GDK_o:
            context->enable_onionskin( ! context->onionskin_enabled_);
            break;
        default:
            break;
    }
    return TRUE;
}
/**
 * Called when the window is deleted. Quits the application.
 */
void Gui::on_delete_event(GtkWidget* /*widget*/, GdkEvent* /*event*/, gpointer user_data)
{
    Gui *context = static_cast<Gui*>(user_data);
    g_print("Window has been deleted.\n");
    context->owner_->quit();
}
/**
 * Toggles fullscreen mode on/off.
 */
void Gui::toggleFullscreen(GtkWidget *widget)
{
    // toggle fullscreen state
    isFullscreen_ ? makeUnfullscreen(widget) : makeFullscreen(widget);
}
/** 
 * Makes the window fullscreen.
 */
void Gui::makeFullscreen(GtkWidget *widget)
{
    gtk_window_stick(GTK_WINDOW(widget)); // window is visible on all workspaces
    gtk_window_fullscreen(GTK_WINDOW(widget));
}
/**
 * Makes the window not fullscreen.
 */
void Gui::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unstick(GTK_WINDOW(widget)); // window is not visible on all workspaces
    gtk_window_unfullscreen(GTK_WINDOW(widget));
}
/**
 * Slot for the Controller's next_image_to_play_signal_ signal.
 *
 * Called when it's time to update the image to play back.
 * 
 * The next image to play is raised on top of the input image textures.
 * If the duration_ratio_ (TO RENAME) is set, tweens its opacity from 0 to 255 in 
 * as much ms there are between two frames currently. 
 */
void Gui::on_next_image_to_play(unsigned int /*clip_number*/, unsigned int/*image_number*/, std::string file_name)
{
    GError *error = NULL;
    gboolean success;
    // Rotate the textures
    ClutterActor* _tmp = playback_textures_.back();
    playback_textures_.insert(playback_textures_.begin() + 0, _tmp);
    playback_textures_.pop_back();
    // Load file
    success = clutter_texture_set_from_file(CLUTTER_TEXTURE(playback_textures_.at(0)), file_name.c_str(), &error);
    clutter_container_raise_child(CLUTTER_CONTAINER(playback_group_), CLUTTER_ACTOR(playback_textures_.at(0)), NULL);
   
    // TODO: Handle the ClutterAnimation* 
    // Attach a callback to when it's done
    if (duration_ratio_ >= 0.01f)
    {
        unsigned int fps = owner_->get_current_clip()->get_playhead_fps();
        unsigned int duration = (unsigned int) ((1.0f / fps * duration_ratio_) * 1000);
        clutter_actor_set_opacity(CLUTTER_ACTOR(playback_textures_.at(0)), 0);
        clutter_actor_animate(CLUTTER_ACTOR(playback_textures_.at(0)), CLUTTER_LINEAR, duration, "opacity", 255, NULL);  
    }
    else
        clutter_actor_set_opacity(CLUTTER_ACTOR(playback_textures_.at(0)), 255);
        
    // TODO: validate this path
    if (!success)
    {
        std::cerr << "Failed to load pixbuf file: " << file_name << " " << error->message << std::endl;
        g_error_free(error);
    } else {
        //std::cout << "Loaded image " <<  image_full_path << std::endl;
    }
}

/**
 * Slot for the Controller's add_frame_signal_ signal.
 * 
 * Updates the onionskin texture
 */
void Gui::on_frame_added(unsigned int /*clip_number*/, unsigned int image_number)
{
    GError *error = NULL;
    gboolean success;
    // Rotate the textures
    ClutterActor* _tmp = onionskin_textures_.back();
    onionskin_textures_.insert(onionskin_textures_.begin() + 0, _tmp);
    onionskin_textures_.pop_back();
    // Get image file name
    Clip* clip = owner_->get_current_clip();
    Image* image = clip->get_image(image_number);
    if (image == 0)
        std::cout << "Could not get a handle to any image!" << std::endl;
    else
    {
        std::string file_name = clip->get_image_full_path(image);
        // Load file
        success = clutter_texture_set_from_file(CLUTTER_TEXTURE(onionskin_textures_.at(0)), file_name.c_str(), &error);
        clutter_container_raise_child(CLUTTER_CONTAINER(onionskin_group_), CLUTTER_ACTOR(onionskin_textures_.at(0)), NULL);
        if (!success)
        {
            std::cerr << "Failed to load pixbuf file: " << file_name << " " << error->message << std::endl;
            g_error_free(error);
        } else {
            //std::cout << "Loaded image " <<  image_full_path << std::endl;
        }
    }
}

/** 
 * Called on every frame. 
 *
 * (Clutter Timeline handler)
 * Times the playback frames and display it if it's time to do so.
 *
 * Prints the rendering FPS information.
 * Calls Controller::update_playback_image
 */
void Gui::on_render_frame(ClutterTimeline * /*timeline*/, gint /*msecs*/, gpointer user_data)
{
    // Prints rendering FPS information
    static int number_of_frames_in_last_second = 0; // counting FPS
    static Timer fps_calculation_timer = Timer();
    
    Gui *context = static_cast<Gui*>(user_data);
    bool verbose = context->owner_->get_configuration()->get_verbose();
    Clip *thisclip = context->owner_->get_current_clip();
    
    context->update_info_text();

    fps_calculation_timer.tick();
    ++number_of_frames_in_last_second;
    // calculate rendering FPS
    if (fps_calculation_timer.get_elapsed() >= 1.0f)
    {
        if (verbose)
            std::cout << "Rendering FPS: " << number_of_frames_in_last_second << std::endl;
        number_of_frames_in_last_second = 0;
        fps_calculation_timer.reset();
    }

    context->owner_->get_controller()->update_playback_image();

    //TODO:2010-08-26:aalex:connect to Controller's on_no_image_to_play
    if(thisclip->size() > 0) 
    {     
        if (! CLUTTER_ACTOR_IS_VISIBLE(context->playback_group_))
            clutter_actor_show_all(CLUTTER_ACTOR(context->playback_group_));
    } else {
        if (CLUTTER_ACTOR_IS_VISIBLE(context->playback_group_))
            clutter_actor_hide_all(CLUTTER_ACTOR(context->playback_group_));
    }
}

/**
 * Called when the stage size has changed.
 *
 * Calls Gui::resize_actors
 */
void on_stage_allocation_changed(ClutterActor * /*stage*/, 
        ClutterActorBox * /*box*/, 
        ClutterAllocationFlags * /*flags*/, 
        gpointer user_data) 
{
    //g_print("on_stage_allocation_changed\n");
    Gui *gui = static_cast<Gui*>(user_data);
    gui->resize_actors();
}
/**
 * Called when it's time to resize the textures in the stage.
 *
 * Either the window has been resized, or the input image size has changed.
 * 
 * This is where the actors are resized according to the current layout.
 * Also, the actor transparency and visibility vary in each layout.
 */
void Gui::resize_actors() {
    // We could override the paint method of the stage
    // Or put everything in a container which has an apply_transform()
    gfloat set_x, set_y, set_width, set_height;
    gfloat stage_width, stage_height;

    clutter_actor_get_size(stage_, &stage_width, &stage_height);
    
    set_height = (video_input_height_ * stage_width) / video_input_width_;
    if (set_height <= stage_height) 
    {
        set_width = stage_width;
        set_x = 0;
        set_y = (stage_height - set_height) / 2;
    } 
    else 
    {
        set_width  = (video_input_width_ * stage_height) / video_input_height_;
        set_height = stage_height;
        set_x = (stage_width - set_width) / 2;
        set_y = 0;
    }
    if (current_layout_ == LAYOUT_SPLITSCREEN)
    {
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
        for (ActorIterator iter = playback_textures_.begin(); iter != playback_textures_.end(); ++iter)
        {
            clutter_actor_set_position(CLUTTER_ACTOR(*iter), playback_tex_x, playback_tex_y);
            clutter_actor_set_size(CLUTTER_ACTOR(*iter), playback_tex_width, playback_tex_height);
            clutter_actor_set_opacity(CLUTTER_ACTOR(*iter), 255);
        }
        for (ActorIterator iter = onionskin_textures_.begin(); iter != onionskin_textures_.end(); ++iter)
        {
            clutter_actor_set_position(CLUTTER_ACTOR(*iter), playback_tex_x, playback_tex_y);
            clutter_actor_set_size(CLUTTER_ACTOR(*iter), playback_tex_width, playback_tex_height);
        }
    } 
    else if (current_layout_ == LAYOUT_PLAYBACK_ONLY) 
    {
        for (ActorIterator iter = playback_textures_.begin(); iter != playback_textures_.end(); ++iter)
        {
            clutter_actor_set_position(CLUTTER_ACTOR(*iter), set_x, set_y);
            clutter_actor_set_size(CLUTTER_ACTOR(*iter), set_width, set_height);
            clutter_actor_set_opacity(CLUTTER_ACTOR(*iter), 255);
        }
        for (ActorIterator iter = onionskin_textures_.begin(); iter != onionskin_textures_.end(); ++iter)
        {
            clutter_actor_set_position(CLUTTER_ACTOR(*iter), set_x, set_y);
            clutter_actor_set_size(CLUTTER_ACTOR(*iter), set_width, set_height);
        }
    } 
    else if (current_layout_ == LAYOUT_OVERLAY) 
    {
        for (ActorIterator iter = playback_textures_.begin(); iter != playback_textures_.end(); ++iter)
        {
            clutter_actor_set_position(CLUTTER_ACTOR(*iter), set_x, set_y);
            clutter_actor_set_size(CLUTTER_ACTOR(*iter), set_width, set_height);
        }
        for (ActorIterator iter = onionskin_textures_.begin(); iter != onionskin_textures_.end(); ++iter)
        {
            clutter_actor_set_position(CLUTTER_ACTOR(*iter), set_x, set_y);
            clutter_actor_set_size(CLUTTER_ACTOR(*iter), set_width, set_height);
        }
        clutter_actor_set_position(CLUTTER_ACTOR(live_input_texture_), set_x, set_y);
        clutter_actor_set_size(CLUTTER_ACTOR(live_input_texture_), set_width, set_height);
        clutter_actor_set_opacity(CLUTTER_ACTOR(live_input_texture_), overlay_opacity_);
    } 
    else 
        std::cout << "ERROR: Invalid layout" << std::endl; // should not occur...
}
/**
 * Called when the size of the input image has changed.
 *
 * Useful to update the layout. Calls Gui::resize_actors.
 *
 * Makes the live input texture visible. 
 */
void Gui::on_live_input_texture_size_changed(ClutterTexture *texture, gfloat width, gfloat height, gpointer user_data) 
{
    //g_print("on_live_input_texture_size_changed\n");
    Gui *gui = static_cast<Gui*>(user_data);
    gui->video_input_width_ = (float) width;
    gui->video_input_height_ = (float) height;

    ClutterActor *stage;
  
    stage = clutter_actor_get_stage(CLUTTER_ACTOR(texture));
    if (stage == NULL)
        return;

    clutter_actor_show_all(CLUTTER_ACTOR(gui->live_input_texture_));
    gui->resize_actors();
}

/**
 * Toggles the layout.
 */
void Gui::toggle_layout()
{
    if (current_layout_ == LAYOUT_PLAYBACK_ONLY)
        set_layout(LAYOUT_SPLITSCREEN);
    else if (current_layout_ == LAYOUT_SPLITSCREEN)
        set_layout(LAYOUT_OVERLAY);
    else if (current_layout_ == LAYOUT_OVERLAY)
        set_layout(LAYOUT_PLAYBACK_ONLY);
}
/**
 * Sets the current layout.
 * Also makes some actors visible or not.
 */
void Gui::set_layout(layout_number layout)
{
    current_layout_ = layout; 
    if (current_layout_ == LAYOUT_PLAYBACK_ONLY)
    {
        clutter_actor_hide_all(CLUTTER_ACTOR(live_input_texture_));
    } else if (current_layout_ == LAYOUT_SPLITSCREEN) {
        clutter_actor_show_all(CLUTTER_ACTOR(live_input_texture_));
    }
    resize_actors();
}

/**
 * Called when the size of the image to play back has changed.
 *
 * Useful to update the layout. Calls Gui::resize_actors.
 */
void on_playback_texture_size_changed(ClutterTexture *texture, 
        gfloat /*width*/, gfloat /*height*/, gpointer user_data) 
{
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
 * Graphical user interface for Toonloop. 
 *
 * Here we create the window and a Clutter stage with actors.
 *
 * Exits the application if OpenGL needs are not met.
 */
Gui::Gui(Application* owner) :
    video_input_width_(1),
    video_input_height_(1),
    owner_(owner),
    isFullscreen_(false),
    current_layout_(LAYOUT_SPLITSCREEN),
    overlay_opacity_(175),
    onionskin_opacity_(100),
    onionskin_enabled_(false),
    enable_hud_(false),
    duration_ratio_(1.0)
{
    //video_xwindow_id_ = 0;
    owner_->get_controller()->next_image_to_play_signal_.connect(boost::bind(&Gui::on_next_image_to_play, this, _1, _2, _3));
    owner_->get_controller()->add_frame_signal_.connect(boost::bind(&Gui::on_frame_added, this, _1, _2));
    //TODO: owner_->get_controller()->no_image_to_play_signals_.connect(boost::bind(&Gui::on_no_image_to_play, this))
    // Main GTK window
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // TODO:2010-08-06:aalex:make window size configurable
    gtk_widget_set_size_request(window_, WINWIDTH, WINHEIGHT); 
    gtk_window_move(GTK_WINDOW(window_), 300, 10); // TODO: make configurable
    gtk_window_set_title(GTK_WINDOW(window_), "Toonloop " PACKAGE_VERSION);
    // Set window icon
    fs::path iconPath(std::string(PIXMAPS_DIR) + "/toonloop.png");
    if (fs::exists(iconPath))
        gtk_window_set_icon_from_file(GTK_WINDOW(window_), iconPath.string().c_str(), NULL);

    GdkGeometry geometry;
    geometry.min_width = 1;
    geometry.min_height = 1;
    geometry.max_width = -1;
    geometry.max_height = -1;
    gtk_window_set_geometry_hints(GTK_WINDOW(window_), window_, &geometry, GDK_HINT_MIN_SIZE);
    // connect window signals:
    g_signal_connect(G_OBJECT(window_), "delete-event", G_CALLBACK(on_delete_event), this);

    g_signal_connect(G_OBJECT(window_), "key-press-event", G_CALLBACK(key_press_event), this);
    g_signal_connect(G_OBJECT(window_), "button-press-event", G_CALLBACK(on_mouse_button_event), this);
    
    // add listener for window-state-event to detect fullscreenness
    g_signal_connect(G_OBJECT(window_), "window-state-event", G_CALLBACK(on_window_state_event), this);

    // vbox:
    vbox_ = gtk_vbox_new(FALSE, 0); // args: homogeneous, spacing
    gtk_container_add(GTK_CONTAINER(window_), vbox_);

    //some buttons:
    //TODO:2010:08-17:aalex:Add a HBox with some buttons, plus a menu

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

    // Playback textures:
    playback_group_ = clutter_group_new();
    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(playback_group_));
    
    const unsigned int NUM_PLAYBACK_IMAGES = 30;
    for(unsigned int i = 0 ; i < NUM_PLAYBACK_IMAGES; i++)
    {
        playback_textures_.insert(playback_textures_.begin(), 
            (ClutterActor*)
            g_object_new(CLUTTER_TYPE_TEXTURE, 
                "sync-size", FALSE, 
                "disable-slicing", TRUE, 
                NULL));
        clutter_container_add_actor(CLUTTER_CONTAINER(playback_group_), CLUTTER_ACTOR(playback_textures_.at(0)));
        // FIXME:2010-09-14:aalex:It's OK to detect playback image change but it should be done at the last minute before drawing each.
        g_signal_connect(CLUTTER_TEXTURE(playback_textures_.at(0)), "size-change", G_CALLBACK(on_playback_texture_size_changed), this);
        clutter_container_raise_child(CLUTTER_CONTAINER(playback_group_), CLUTTER_ACTOR(playback_textures_.at(0)), NULL);
    }

    // Onionskin textures:
    onionskin_group_ = clutter_group_new();
    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(onionskin_group_));
    const unsigned int NUM_ONIONSKIN_IMAGES = 1;
    for(unsigned int i = 0 ; i < NUM_ONIONSKIN_IMAGES; i++)
    {
        onionskin_textures_.insert(onionskin_textures_.begin(), 
            (ClutterActor*)
            g_object_new(CLUTTER_TYPE_TEXTURE, 
                "sync-size", FALSE, 
                "disable-slicing", TRUE, 
                NULL));
        clutter_container_add_actor(CLUTTER_CONTAINER(onionskin_group_), CLUTTER_ACTOR(onionskin_textures_.at(0)));
        clutter_container_raise_child(CLUTTER_CONTAINER(onionskin_group_), CLUTTER_ACTOR(onionskin_textures_.at(0)), NULL);
    }
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(playback_group_), NULL);
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(onionskin_group_), NULL);

    // Background color:
    ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };
    clutter_stage_set_color(CLUTTER_STAGE(stage_), &stage_color);

    /* Create a timeline to manage animation */
    timeline_ = clutter_timeline_new(6000); // time here doesn't matter
    g_object_set(timeline_, "loop", TRUE, NULL);   /* have it loop */
    /* fire a callback for frame change */
    g_signal_connect(timeline_, "new-frame", G_CALLBACK(on_render_frame), this);
    /* and start it */
    clutter_timeline_start(timeline_);

    /* of course, we need to show the texture in the stage. */

    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(live_input_texture_));
    clutter_actor_hide_all(CLUTTER_ACTOR(live_input_texture_));
    
    // TEXT
    info_text_actor_ = clutter_text_new_full("", "Sans 16px", clutter_color_new(255, 255, 255, 255));
    update_info_text();
    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(info_text_actor_));
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(info_text_actor_), NULL);
  
    gtk_widget_show_all(window_);

    /* Only show the actors after parent show otherwise it will just be
     * unrealized when the clutter foreign window is set. widget_show
     * will call show on the stage.
     */
    clutter_actor_show_all(CLUTTER_ACTOR(live_input_texture_));
    clutter_actor_show_all(CLUTTER_ACTOR(playback_group_));
    clutter_actor_show_all(CLUTTER_ACTOR(onionskin_group_));
    enable_onionskin(false); // hides it
    //NO: clutter_actor_show_all(CLUTTER_ACTOR(info_text_actor_));
    
    for (ActorIterator iter = playback_textures_.begin(); iter != playback_textures_.end(); ++iter)
        clutter_actor_show_all(CLUTTER_ACTOR(*iter));
    for (ActorIterator iter = onionskin_textures_.begin(); iter != onionskin_textures_.end(); ++iter)
        clutter_actor_show_all(CLUTTER_ACTOR(*iter));
    clutter_actor_hide(info_text_actor_);
    if (owner_->get_configuration()->get_fullscreen())
        toggleFullscreen(window_);
}

void Gui::update_info_text()
{
    std::ostringstream os;
    Clip* current_clip = owner_->get_current_clip();
    os << "Toonloop " << PACKAGE_VERSION << std::endl;
    os << "Current clip: " << current_clip->get_id() << std::endl;
    os << "FPS: " << current_clip->get_playhead_fps() << std::endl;
    os << "Playhead: " << current_clip->get_playhead() << std::endl;
    os << "Writehead: " << current_clip->get_writehead() << "/" << current_clip->size() << std::endl;
    os << "Intervalometer rate: " << current_clip->get_intervalometer_rate() << std::endl;
    clutter_text_set_text(CLUTTER_TEXT(info_text_actor_), os.str().c_str());
}

/**
 * Returns the live input image texture. 
 *
 * Called from the Pipeline in order to get the video sink element that allows
 * use to view some GStreamer video on a Clutter actor.
 */
ClutterActor* Gui::get_live_input_texture() const
{
    return live_input_texture_;
}

Gui::~Gui()
{
    std::cout << "~Gui" << std::endl;
    //TODO:
    //for (ActorIterator iter = playback_textures_.begin(); iter != playback_textures_.end(); ++iter)
    //    //iter->
    for (unsigned int i = 0; i < playback_textures_.size(); i++)
    {
        //ClutterActor* tmp = playback_textures_.back();
        playback_textures_.pop_back();
        //g_print("TODO: clutter destroy texture\n");
    }
}
