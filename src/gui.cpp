/*
 * Toonloop
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
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
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <clutter-gst/clutter-gst.h>
#include <clutter-gtk/clutter-gtk.h>
#include <clutter/clutter.h>
#include <cmath>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <iostream>
#include <sstream>

#include "application.h"
#include "clip.h"
#include "config.h"
#include "controller.h"
#include "gui.h"
#include "pipeline.h"
#include "properties.h"
#include "property.h"
#include "brcosaeffect.h"
#include "timer.h"
#include "unused.h"
#include "legacyclutter.h"

namespace fs = boost::filesystem;
using std::tr1::shared_ptr;
typedef std::vector<ClutterActor*>::iterator ActorIterator;

static int clip_int(int value, int from, int to)
{
    return std::max(std::min(value, to), from);
}

gboolean Gui::on_mouse_button_event(ClutterActor* /* actor */, ClutterEvent *event, gpointer user_data)
{
    Gui *context = static_cast<Gui *>(user_data);
    // Before 2010-09-20 we tried to start/stop video recording but it was buggy
    if (event->type == CLUTTER_BUTTON_PRESS)
    {
        if (context->owner_->get_configuration()->get_mouse_controls_enabled())
            context->owner_->get_controller()->add_frame();
    }
    return TRUE;
}

/**
 * [0,255]
 */
void Gui::on_livefeed_opacity_changed(std::string &name, int value)
{
    UNUSED(name);
    if (owner_->get_configuration()->get_verbose())
        std::cout << "Gui::" << __FUNCTION__ << " livefeed opacity: " << value << std::endl;
    if (current_layout_ == LAYOUT_OVERLAY)
    {
        clutter_actor_set_opacity(CLUTTER_ACTOR(live_input_texture_), value);
    }
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
    // TODO:2010-09-18:aalex:Hide the actors instead of making them transparent.
    if (onionskin_enabled_)
        clutter_actor_set_opacity(CLUTTER_ACTOR(onionskin_textures_.at(0)), onionskin_opacity_);
    else
        clutter_actor_set_opacity(CLUTTER_ACTOR(onionskin_textures_.at(0)), 0);
}
/**
 * In fullscreen mode, hides the cursor. In windowed mode, shows the cursor.
 */
gboolean Gui::on_window_state_event(GtkWidget* /*widget*/, GdkEvent * event, gpointer user_data)
{
    Gui *context = static_cast<Gui*>(user_data);
    GdkEventWindowState *tmp = (GdkEventWindowState *) event;
    context->isFullscreen_ = (tmp->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    if (context->isFullscreen_)
        context->hideCursor();
    else
        context->showCursor();
    return TRUE;
}

bool is_amd64()
{
    return sizeof(void*) == 8;
}
/**
 * In fullscreen mode, hides the cursor.
 */
void Gui::hideCursor()
{
    if (! is_amd64())
    	gdk_window_set_cursor(GDK_WINDOW(clutter_widget_->window), (GdkCursor *) GDK_BLANK_CURSOR);
}

/**
 * In windowed mode, shows the cursor.
 */
void Gui::showCursor()
{
    /// sets to default
    gdk_window_set_cursor(GDK_WINDOW(clutter_widget_->window), (GdkCursor *) NULL);
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
 * - r: clears the contents of the current clip
 * - Ctrl-e: export the current clip
 * - Ctrl-s: save the whole project
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
 */
gboolean Gui::key_press_event(ClutterActor *stage, ClutterEvent *event, gpointer user_data)
{
    // TODO:2010-09-18:aalex:Use the accelerators to allow the user to configure the controls
    // TODO:2010-09-18:aalex:Use Clutter for mouse and keyboard controls (ClutterBindingPool)

    UNUSED(stage);
    guint keyval = clutter_event_get_key_symbol(event);
    ClutterModifierType state = clutter_event_get_state(event);
    //bool shift_pressed = (state & CLUTTER_SHIFT_MASK ? true : false);
    bool ctrl_pressed = (state & CLUTTER_CONTROL_MASK ? true : false);
    bool caps_lock_on = (state & CLUTTER_LOCK_MASK ? false : true); // FIXME: caps lock seems on when its off and vice versa

    Gui *context = static_cast<Gui*>(user_data);
    Controller *controller = context->owner_->get_controller();
    bool verbose = context->owner_->get_configuration()->get_verbose();

    switch (keyval)
    {
        case CLUTTER_KEY_Caps_Lock:
        {
            if (caps_lock_on)
            {
                std::cout << "Caps_Lock on. Recording video." << std::endl;
                controller->enable_video_grabbing(true);
            } else {
                std::cout << "Caps_Lock off." << std::endl;
                controller->enable_video_grabbing(false);
            }
            break;
        }
        case CLUTTER_KEY_Up:
            controller->increase_playhead_fps();
            break;
        case CLUTTER_KEY_Down:
            controller->decrease_playhead_fps();
            break;
        //case CLUTTER_KEY_XXX:
        //    controller->set_current_clip_direction(DIRECTION_BACKWARD);
        //    break;
        //case CLUTTER_KEY_XXX:
        //    controller->set_current_clip_direction(DIRECTION_FORWARD);
        //    break;
        case CLUTTER_KEY_Tab:
            controller->change_current_clip_direction();
            break;
        case CLUTTER_KEY_period:
            //TODO:2010-08-27:aalex:Create Controller:toggle_layout
            context->toggle_layout();
            break;
        case CLUTTER_KEY_r:
            // Ctrl-r or just r?
            //if (event->state & CLUTTER_CONTROL_MASK)
            controller->clear_current_clip();
            break;
        case CLUTTER_KEY_BackSpace:
            controller->remove_frame();
            break;
        case CLUTTER_KEY_f:
        case CLUTTER_KEY_Escape:
            context->toggleFullscreen(context->window_);
            break;
        case CLUTTER_KEY_space:
            controller->add_frame();
            break;
        case CLUTTER_KEY_Page_Up:
            controller->choose_previous_clip();
            break;
        case CLUTTER_KEY_Page_Down:
            controller->choose_next_clip();
            break;
        case CLUTTER_KEY_0:
        case CLUTTER_KEY_1:
        case CLUTTER_KEY_2:
        case CLUTTER_KEY_3:
        case CLUTTER_KEY_4:
        case CLUTTER_KEY_5:
        case CLUTTER_KEY_6:
        case CLUTTER_KEY_7:
        case CLUTTER_KEY_8:
        case CLUTTER_KEY_9:
        {   // need to use brackets when declaring variable inside case
            //* Switch the current clip according to a gdk key value from 0 to 9
            //* keyval should be one of :
            //* GDK_0 GDK_1 GDK_2 GDK_3 GDK_4 GDK_5 GDK_6 GDK_7 GDK_8 GDK_9
            //* Of course, any other value might lead to a crash.
            // FIXME:2010-08-17:aalex:Doing arithmetics with a gdk keyval is a hack
            unsigned int number_pressed = (keyval & 0x0F);
            if (ctrl_pressed)
            {
                if (number_pressed == 0)
                    context->set_layout(LAYOUT_SPLITSCREEN);
                else if (number_pressed == 1)
                    context->set_layout(LAYOUT_PLAYBACK_ONLY);
                else if (number_pressed == 2)
                    context->set_layout(LAYOUT_OVERLAY);
                else if (number_pressed == 3)
                    context->set_layout(LAYOUT_PORTRAIT);
                else if (number_pressed == 4)
                    context->set_layout(LAYOUT_LIVEFEED_ONLY);
                else
                    std::cout << "No layout with number " << number_pressed << std::endl;
            }
            else
                controller->choose_clip(number_pressed);
            break;
        }
        case CLUTTER_KEY_q:
            // Quit application on ctrl-q, this quits the main loop
            // (if there is one)
            if (ctrl_pressed)
            {
                if (verbose)
                    g_print("Ctrl-Q key pressed, quitting.\n");
                context->owner_->quit();
            }
            break;
        case CLUTTER_KEY_s:
            // Ctrl-s: Save the whole project
            if (ctrl_pressed)
            {
                //if (verbose)
                controller->save_project();
                g_print("Saved the whole project.\n");
            } else // no Ctrl pressed
                g_print("Warning: Use Ctrl-E to export the current clip as a movie file, or Ctrl-s to save the whole project.\n");
            break;
        case CLUTTER_KEY_e:
            // Ctrl-e: Exports the current clip
            // (if there is one)
            if (ctrl_pressed)
            {
                if (verbose)
                    g_print("Exporting the current clip.");
                controller->save_current_clip();
            }
            break;
        case CLUTTER_KEY_F2: // TODO: change this key for save
            controller->save_project();
            controller->save_current_clip();
            break;
        case CLUTTER_KEY_a:
            //std::cout << "Toggle intervalometer." << std::endl; 
            controller->toggle_intervalometer();
            break;
        case CLUTTER_KEY_k:
            controller->increase_intervalometer_rate();
            break;
        case CLUTTER_KEY_j:
            controller->decrease_intervalometer_rate();
            break;
        case CLUTTER_KEY_Left:
            controller->move_writehead_to_previous();
            break;
        case CLUTTER_KEY_Right:
            controller->move_writehead_to_next();
            break;
        case CLUTTER_KEY_Return:
            controller->move_writehead_to_last();
            break;
        case CLUTTER_KEY_semicolon:
            controller->move_writehead_to_first();
            break;
        case CLUTTER_KEY_bracketleft:
            controller->set_int_value("livefeed_opacity", clip_int(controller->get_int_value("livefeed_opacity") - 1, 0, 255));
            break;
        case CLUTTER_KEY_bracketright:
            controller->set_int_value("livefeed_opacity", clip_int(controller->get_int_value("livefeed_opacity") + 1, 0, 255));
            break;
        case CLUTTER_KEY_parenleft:
            context->crossfade_increment(-0.1f);
            break;
        case CLUTTER_KEY_parenright:
            context->crossfade_increment(0.1f);
            break;
        case CLUTTER_KEY_i:
            context->toggle_info();
            break;
        case CLUTTER_KEY_F1:
            context->toggle_help();
            break;
        case CLUTTER_KEY_o:
            context->enable_onionskin( ! context->onionskin_enabled_);
            break;
        case CLUTTER_KEY_x:
            controller->set_int_value("black_out", 1 - controller->get_int_value("black_out")); // toggle [0,1]
            break;
        case CLUTTER_KEY_b:
            {
                Property<int> *blending_mode = controller->int_properties_.get_property("blending_mode");
                if (blending_mode->get_value() == 1) //context->blending_mode_ == BLENDING_MODE_ADDITIVE)
                    blending_mode->set_value(0);
                    //context->set_blending_mode(BLENDING_MODE_NORMAL);
                else
                    blending_mode->set_value(1);
                    //context->set_blending_mode(BLENDING_MODE_ADDITIVE);
                std::cout << "Blending mode:" << context->blending_mode_ << std::endl;
            }
            break;
        default:
            break;
    }
    return TRUE;
}

void Gui::on_crossfade_ratio_changed(std::string &name, float value)
{
    UNUSED(name);
    if (crossfade_ratio_ != value)
    {
        crossfade_ratio_ = value;
        if (owner_->get_configuration()->get_verbose())
            std::cout << "Crossfade ratio: " << value << std::endl;
    }
}

/**
 * Adds a certain amount of crossfading duration to the crossfade between images.
 * The given value can be negative.
 */
void Gui::crossfade_increment(float value)
{
    const float maximum = 10.0f;
    float current = crossfade_ratio_;
    float new_value = current + value;

    if (new_value < 0.0f)
        new_value = 0.0f;
    else if (new_value > maximum)
        new_value = maximum;
    if (crossfade_ratio_ != new_value)
    {
        crossfade_ratio_ = new_value;
        if (owner_->get_configuration()->get_verbose())
            std::cout << "Duration ratio: " << new_value << std::endl;
    }
}

/**
 * Toggles the visibility of the info text.
 */
void Gui::toggle_info()
{
    enable_info_ = ! enable_info_;
    if (enable_info_)
        clutter_actor_show(info_text_actor_);
    else
        clutter_actor_hide(info_text_actor_);
}
/**
 * Toggles the visibility of the info text.
 */
void Gui::toggle_help()
{
    enable_help_ = ! enable_help_;
    if (enable_help_)
        clutter_actor_show(help_text_actor_);
    else
        clutter_actor_hide(help_text_actor_);
}

/**
 * Called when the window is deleted. Quits the application.
 */
void Gui::on_delete_event(GtkWidget* /*widget*/, GdkEvent* /*event*/, gpointer user_data)
{
    Gui *context = static_cast<Gui*>(user_data);
    if (context->owner_->get_configuration()->get_verbose())
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
 * The next image to play is lowered to bottom of the input image textures.
 * If the crossfade_ratio_ is set, tweens its opacity from 255 to 0 in 
 * as much ms there are between two frames currently. 
 */
void Gui::on_next_image_to_play(unsigned int clip_number, unsigned int/*image_number*/, std::string file_name)
{
    GError *error = NULL;
    gboolean success;
    // Rotate the textures
    ClutterActor* _tmp = playback_textures_.back();
    playback_textures_.insert(playback_textures_.begin() + 0, _tmp);
    playback_textures_.pop_back();
    // Load file
    success = clutter_texture_set_from_file(CLUTTER_TEXTURE(playback_textures_.at(0)), file_name.c_str(), &error);
    clutter_container_lower_child(CLUTTER_CONTAINER(playback_group_), CLUTTER_ACTOR(playback_textures_.at(0)), NULL);
    clutter_actor_set_opacity(CLUTTER_ACTOR(playback_textures_.at(0)), 255);
   
    if (owner_->get_clip(clip_number)->size() == 0)
    {
        if (owner_->get_configuration()->get_verbose())
            g_print("No image to play in current clip.\n");
    }
    // TODO: Handle the ClutterAnimation* 
    // Attach a callback to when it's done
    if (crossfade_ratio_ > 0.0f && owner_->get_clip(clip_number)->size() > 1) // do not fade if only one image in clip
    {
        unsigned int fps = owner_->get_clip(clip_number)->get_playhead_fps();
        unsigned int duration = (unsigned int) (((1.0f / fps) * crossfade_ratio_) * 1000);
        //if (owner_->get_configuration()->get_verbose())
        //    std::cout << "animate texture for " << duration << " ms" << std::endl;
        // TODO:2010-11-10:aalex:If there is only one image in the clip, do not fade out.
        clutter_actor_animate(CLUTTER_ACTOR(playback_textures_.at(1)), CLUTTER_EASE_IN_OUT_CUBIC, duration, "opacity", 0, NULL);  
    }
    else
        clutter_actor_set_opacity(CLUTTER_ACTOR(playback_textures_.at(1)), 0);
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
void Gui::on_frame_added(unsigned int clip_number, unsigned int image_number)
{
    if (owner_->get_configuration()->get_verbose())
        std::cout << "Gui::on_frame_added" << std::endl;
    // Rotate the textures
    ClutterActor* _tmp = onionskin_textures_.back();
    onionskin_textures_.insert(onionskin_textures_.begin() + 0, _tmp);
    onionskin_textures_.pop_back();
    // Get image file name
    Clip* clip = owner_->get_clip(clip_number);
    Image* image = clip->get_image(image_number);
    if (image == 0)
        std::cout << __FUNCTION__ << ": Could not get a handle to any image!" << std::endl;
    else
    {
        GError *error = NULL;
        gboolean success;
        std::string file_name = clip->get_image_full_path(image);
        // Load file
        //std::cout << "Loading " << file_name << " for onion skin" << std::endl;
        success = clutter_texture_set_from_file(CLUTTER_TEXTURE(onionskin_textures_.at(0)), file_name.c_str(), &error);
        // If ever we have many onion skins, we might raise the latest one:
        // //clutter_container_raise_child(CLUTTER_CONTAINER(onionskin_group_), CLUTTER_ACTOR(onionskin_textures_.at(0)), NULL);
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
 * We could draw some stuff using OpenGL in this callback.
 */
void Gui::on_render_frame(ClutterTimeline * /*timeline*/, gint /*msecs*/, gpointer user_data)
{
    // Prints rendering FPS information

    Gui *context = static_cast<Gui*>(user_data);

    context->owner_->check_for_messages();
    bool verbose = context->owner_->get_configuration()->get_verbose();
    Clip *thisclip = context->owner_->get_current_clip();

    // calculate rendering FPS
    context->fps_calculation_timer_.tick();
    ++context->number_of_frames_in_last_second_;
    if (context->fps_calculation_timer_.get_elapsed() >= 1.0f)
    {
        if (verbose)
            std::cout << "Rendering FPS: " << context->number_of_frames_in_last_second_ << std::endl;
        context->rendering_fps_ = context->number_of_frames_in_last_second_;
        context->number_of_frames_in_last_second_ = 0;
        context->fps_calculation_timer_.reset();
    }
    // Display info:
    if (context->enable_info_)
        context->update_info_text();
    context->info_window_.update_info_window();

    context->owner_->get_controller()->update_playback_image();

    //TODO:2010-08-26:aalex:connect to Controller's on_no_image_to_play
    if(thisclip->size() > 0) 
    {     
        if (context->current_layout_ != LAYOUT_LIVEFEED_ONLY)
            clutter_actor_show_all(CLUTTER_ACTOR(context->playback_group_));
        else
            clutter_actor_hide_all(CLUTTER_ACTOR(context->playback_group_));
    } else {
        clutter_actor_hide_all(CLUTTER_ACTOR(context->playback_group_));
    }
    // // This is just a test
    // static float rot = 0.0f;
    // rot += 1.0f;
    // clutter_actor_set_rotation(CLUTTER_ACTOR(context->live_input_texture_), CLUTTER_Z_AXIS, rot, 160.0f, 120.0f, 0.0f);
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
void Gui::resize_actors() 
{
    Controller *controller = owner_->get_controller();
    // Hide or show the live input:
    if (current_layout_ == LAYOUT_PLAYBACK_ONLY)
    {
        clutter_actor_hide_all(CLUTTER_ACTOR(live_input_texture_));
    }
    else if (current_layout_ == LAYOUT_OVERLAY)
    {    
        clutter_actor_show_all(CLUTTER_ACTOR(live_input_texture_));
        Property<int> *livefeed_opacity = controller->int_properties_.get_property("livefeed_opacity"); // TODO: use int_properties_.get_value() or it might crash if the property is not found!
        clutter_actor_set_opacity(CLUTTER_ACTOR(live_input_texture_), livefeed_opacity->get_value());
    } 
    else  // LAYOUT_SPLITSCREEN or LAYOUT_PORTRAIT or LAYOUT_LIVEFEED_ONLY
    {
        clutter_actor_show_all(CLUTTER_ACTOR(live_input_texture_));
        clutter_actor_set_opacity(CLUTTER_ACTOR(live_input_texture_), 255);
    }

    if (current_layout_ == LAYOUT_LIVEFEED_ONLY)
        clutter_actor_hide(CLUTTER_ACTOR(playback_group_));
    else
        clutter_actor_show(CLUTTER_ACTOR(playback_group_));
    // Figure out the size of the area on which we draw things
    gfloat area_x, area_y, area_width, area_height;
    gfloat stage_width, stage_height;

    clutter_actor_get_size(stage_, &stage_width, &stage_height);

    area_height = (video_input_height_ * stage_width) / video_input_width_;
    if (area_height <= stage_height) 
    {
        area_width = stage_width;
        area_x = 0;
        area_y = (stage_height - area_height) / 2;
    } 
    else 
    {
        area_width  = (video_input_width_ * stage_height) / video_input_height_;
        area_height = stage_height;
        area_x = (stage_width - area_width) / 2;
        area_y = 0;
    }
    // Checks if we are using the whole drawing area, or parts of it for each actor:
    gfloat live_tex_width = area_width;
    gfloat live_tex_height = area_height;
    gfloat live_tex_x = area_x;
    gfloat live_tex_y = area_y;
    gfloat playback_tex_width = area_width;
    gfloat playback_tex_height = area_height;
    gfloat playback_tex_x = area_x;
    gfloat playback_tex_y = area_y;
    gdouble rotation = 0;
    // Position actors in the stage:
    if (current_layout_ == LAYOUT_SPLITSCREEN)
    {
        // live texture size and position:
        live_tex_width = area_width / 2;
        live_tex_height = area_height / 2;
        live_tex_x = area_x;
        live_tex_y = (stage_height / 4);

        // playback texture size and position:
        playback_tex_width = area_width / 2;
        playback_tex_height = area_height / 2;
        playback_tex_x = (stage_width / 2);
        playback_tex_y = (stage_height / 4);
    } 
    else if (current_layout_ == LAYOUT_PLAYBACK_ONLY) 
    {
        // all actors are full screen
    } 
    else if (current_layout_ == LAYOUT_OVERLAY) 
    {
        // all actors are full screen
    } 
    else if (current_layout_ == LAYOUT_LIVEFEED_ONLY) 
    {
        // all actors are full screen
    } 

    else if (current_layout_ == LAYOUT_PORTRAIT)
    {
        // portrait:
        //TODO:2010-09-20:aalex:Those dimensions are not right at all
        // live texture size and position:
        live_tex_width = area_width / 2.0f * (4.0 / 3.0);
        live_tex_height = area_width / 2.0f;
        live_tex_x = area_x - ((live_tex_width - live_tex_height) / 2); // to the left
        live_tex_y = area_y + (area_height - live_tex_height) / 2.0f; // TOP 

        // playback texture size and position: (some are copied from live tex)
        playback_tex_width = live_tex_width;
        playback_tex_height = live_tex_height;
        // FIXME: The following looks OK when the window ration is 4:3 or smaller, but not when bigger than that. 
        playback_tex_x = area_width / 2.0f - ((live_tex_width - live_tex_height) / 2); // in the middle
        playback_tex_y = live_tex_y;

        // rotation: 
        rotation = 90.0f;
    }
    else 
        std::cout << "ERROR: Invalid layout" << std::endl; // should not occur...

    // Now, set actually everything:
    // Live input:
    clutter_actor_set_position(CLUTTER_ACTOR(live_input_texture_), live_tex_x, live_tex_y);
    clutter_actor_set_size(CLUTTER_ACTOR(live_input_texture_), live_tex_width, live_tex_height);
    clutter_actor_set_rotation(CLUTTER_ACTOR(live_input_texture_), CLUTTER_Z_AXIS, rotation, live_tex_width / 2.0f, live_tex_height / 2.0f, 0.0f);

    // TODO: maybe not do the following if layout is LAYOUT_LIVEFEED_ONLY

    // Playback:
    for (ActorIterator iter = playback_textures_.begin(); iter != playback_textures_.end(); ++iter)
    {
        clutter_actor_set_position(CLUTTER_ACTOR(*iter), playback_tex_x, playback_tex_y);
        clutter_actor_set_size(CLUTTER_ACTOR(*iter), playback_tex_width, playback_tex_height);
        clutter_actor_set_rotation(CLUTTER_ACTOR(*iter), CLUTTER_Z_AXIS, rotation, playback_tex_width / 2.0f, playback_tex_height / 2.0f, 0.0f);
    }
    // Onion skin: same as live input
    for (ActorIterator iter = onionskin_textures_.begin(); iter != onionskin_textures_.end(); ++iter)
    {
        clutter_actor_set_position(CLUTTER_ACTOR(*iter), live_tex_x, live_tex_y);
        clutter_actor_set_size(CLUTTER_ACTOR(*iter), live_tex_width, live_tex_height);
        clutter_actor_set_rotation(CLUTTER_ACTOR(*iter), CLUTTER_Z_AXIS, rotation, live_tex_width / 2.0f, live_tex_height / 2.0f, 0.0f);
    }
    clutter_actor_set_size(black_out_rectangle_, stage_width, stage_height);
}
/**
 * Called when the size of the input image has changed.
 *
 * Useful to update the layout. Calls Gui::resize_actors.
 *
 * Makes the live input texture visible. 
 */
void Gui::on_live_input_texture_size_changed(ClutterTexture *texture, gint width, gint height, gpointer user_data) 
{
    if (width < 0 || width > 9999)
        width = 1;
    if (height < 0 || height > 9999)
        height = 1;

    Gui *gui = static_cast<Gui*>(user_data);
    if (gui->owner_->get_configuration()->get_verbose())
        g_print("on_live_input_texture_size_changed\n");
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
        set_layout(LAYOUT_PORTRAIT);
    else if (current_layout_ == LAYOUT_PORTRAIT)
        set_layout(LAYOUT_LIVEFEED_ONLY);
    else if (current_layout_ == LAYOUT_LIVEFEED_ONLY)
        set_layout(LAYOUT_PLAYBACK_ONLY);
}

/**
 * Sets the current layout.
 * Also makes some actors visible or not.
 */
void Gui::set_layout(layout_number layout)
{
    current_layout_ = layout; 
    resize_actors();
}

/**
 * Called when the size of the image to play back has changed.
 *
 * Useful to update the layout. Calls Gui::resize_actors.
 */
void on_playback_texture_size_changed(ClutterTexture *texture, 
        gint /*width*/, gint /*height*/, gpointer user_data) 
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

void Gui::on_blending_mode_int_property_changed(std::string &name, int value)
{
    UNUSED(name);
    if (value == 1)
        set_blending_mode(BLENDING_MODE_ADDITIVE);
    else
        set_blending_mode(BLENDING_MODE_NORMAL);
}

void Gui::on_playback_opacity_changed(std::string &name, int value)
{
    UNUSED(name);
    clutter_actor_set_opacity(playback_group_, value);
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
    onionskin_opacity_(50),
    onionskin_enabled_(false),
    enable_info_(false),
    enable_help_(false),
    crossfade_ratio_(0.0),
    fps_calculation_timer_(),
    number_of_frames_in_last_second_(0),
    rendering_fps_(0),
    info_window_(owner)
{
    Controller *controller = owner_->get_controller();
    controller->next_image_to_play_signal_.connect(boost::bind(&Gui::on_next_image_to_play, this, _1, _2, _3));
    controller->add_frame_signal_.connect(boost::bind(&Gui::on_frame_added, this, _1, _2));
    //TODO: controller->no_image_to_play_signals_.connect(boost::bind(&Gui::on_no_image_to_play, this))
    // Main GTK window
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // TODO:2010-08-06:aalex:make window size configurable
    gtk_widget_set_size_request(window_, WINWIDTH, WINHEIGHT); 
    gtk_window_move(GTK_WINDOW(window_), 300, 10); // TODO: make configurable
    std::string window_title("Toonloop " PACKAGE_VERSION);
    // TODO: version is "snapshot" if minor number is odd, "git" if micro is odd, "release" otherwise
    gtk_window_set_title(GTK_WINDOW(window_), window_title.c_str());
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
    GTK_WIDGET_UNSET_FLAGS (clutter_widget_, GTK_DOUBLE_BUFFERED);
    gtk_container_add(GTK_CONTAINER(vbox_), clutter_widget_);
    stage_ = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(clutter_widget_));

    g_signal_connect(G_OBJECT(stage_), "key-press-event", G_CALLBACK(key_press_event), this);
    g_signal_connect(G_OBJECT(stage_), "button-press-event", G_CALLBACK(on_mouse_button_event), this);

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
        clutter_container_lower_child(CLUTTER_CONTAINER(playback_group_), CLUTTER_ACTOR(playback_textures_.at(0)), NULL);
        clutter_actor_set_opacity(CLUTTER_ACTOR(playback_textures_.at(0)), 0);
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
        clutter_container_lower_child(CLUTTER_CONTAINER(onionskin_group_), CLUTTER_ACTOR(onionskin_textures_.at(0)), NULL);
    }

    // Background color:
    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    clutter_stage_set_color(CLUTTER_STAGE(stage_), &black);

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

    // INFO TEXT

    ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
    info_text_actor_ = clutter_text_new_full("Sans semibold 16px", "", &white);
    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(info_text_actor_));
    // HELP TEXT
    std::string HELP_TEXT(INTERACTIVE_HELP + "(Press F1 to hide)");
    help_text_actor_ = clutter_text_new_full("Sans semibold 12px", HELP_TEXT.c_str(), &white);
    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(help_text_actor_));

    // black out
    black_out_rectangle_ = clutter_rectangle_new_with_color(&black);
    clutter_actor_hide(black_out_rectangle_);
    clutter_container_add_actor(CLUTTER_CONTAINER(stage_), black_out_rectangle_);

    // Sort actors and groups:
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(playback_group_), NULL);
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(live_input_texture_), NULL);
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(onionskin_group_), NULL);
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(black_out_rectangle_), NULL);
    // The image on top, if set:
    if (owner_->get_configuration()->should_show_image_on_top())
    {
        // TODO: move this somewhere else
        std::string image_on_top_path = owner_->get_configuration()->get_image_on_top();
        if (! fs::exists(image_on_top_path))
            std::cout << "No such file: " << image_on_top_path << std::endl;
        else 
        {
            ClutterActor *image_on_top = clutter_texture_new_from_file(image_on_top_path.c_str(), NULL);
            clutter_container_add_actor(CLUTTER_CONTAINER(stage_), image_on_top);
        }
    }
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(info_text_actor_), NULL);
    clutter_container_raise_child(CLUTTER_CONTAINER(stage_), CLUTTER_ACTOR(help_text_actor_), NULL);

    if (owner_->get_configuration()->get_info_window_enabled())
        info_window_.create();

    /* Only show the actors after parent show otherwise it will just be
     * unrealized when the clutter foreign window is set. widget_show
     * will call show on the stage.
     */
    gtk_widget_show_all(window_);

    // Set visibility for other things
    enable_onionskin(false);
    clutter_actor_hide(info_text_actor_);
    clutter_actor_hide(help_text_actor_);
    // shown when we get first live image size, and we play the first image
    clutter_actor_hide(live_input_texture_); 
    clutter_actor_hide_all(CLUTTER_ACTOR(playback_group_));
    // Makes fullscreen if needed
    if (owner_->get_configuration()->get_fullscreen())
        toggleFullscreen(window_);
    // add properties:
    controller->add_int_property("blending_mode", 0)->value_changed_signal_.connect(boost::bind(&Gui::on_blending_mode_int_property_changed, this, _1, _2));
    controller->add_float_property("crossfade_ratio", 0.0)->value_changed_signal_.connect(boost::bind(&Gui::on_crossfade_ratio_changed, this, _1, _2));
    controller->add_int_property("livefeed_opacity", 255)->value_changed_signal_.connect(boost::bind(&Gui::on_livefeed_opacity_changed, this, _1, _2));
    controller->add_int_property("black_out", 0)->value_changed_signal_.connect(boost::bind(&Gui::on_black_out_changed, this, _1, _2));
    controller->add_int_property("black_out_opacity", 255)->value_changed_signal_.connect(boost::bind(&Gui::on_black_out_opacity_changed, this, _1, _2));
    controller->add_int_property("playback_opacity", 255)->value_changed_signal_.connect(boost::bind(&Gui::on_playback_opacity_changed, this, _1, _2));

    // saturation effect:
    clutter_actor_set_name(playback_group_, "playback_group_");
    clutter_actor_set_name(live_input_texture_, "live_input_texture_");
    clutter_actor_set_name(onionskin_group_, "onionskin_group_");
    
    if (owner_->get_configuration()->get_shaders_enabled())
    {
        saturation_effect_.reset(new BrCoSaEffect(controller));
        saturation_effect_->add_actor(playback_group_);
        saturation_effect_->add_actor(live_input_texture_);
        saturation_effect_->add_actor(onionskin_group_);
        saturation_effect_->update_all_actors();
    }
}

void Gui::on_black_out_opacity_changed(std::string &name, int value)
{
    if (owner_->get_configuration()->get_verbose())
        g_print("make black_out opacity %d\n", value);
    UNUSED(name);
    clutter_actor_set_opacity(black_out_rectangle_, value);
}

void Gui::on_black_out_changed(std::string &name, int value)
{
    if (owner_->get_configuration()->get_verbose())
        g_print("toggle black_out %d\n", value);
    UNUSED(name);
    if (value == 0)
        clutter_actor_hide(black_out_rectangle_);
    else
        clutter_actor_show(black_out_rectangle_);
}

std::string Gui::get_layout_name(layout_number layout)
{
    switch (layout)
    {
        case LAYOUT_SPLITSCREEN:
            return std::string("splitscreen");
            break;
        case LAYOUT_OVERLAY:
            return std::string("overlay");
            break;
        case LAYOUT_PLAYBACK_ONLY:
            return std::string("playback_only");
            break;
        case LAYOUT_PORTRAIT:
            return std::string("portrait");
            break;
        case LAYOUT_LIVEFEED_ONLY:
            return std::string("live_feed");
            break;
        default:
            return std::string("unknown");
            break;
    }
}

void Gui::update_info_text()
{
    static const bool isUnstable = PACKAGE_VERSION_MINOR % 2 == 1;
    static const bool isGit = PACKAGE_VERSION_MICRO != 0;
    std::ostringstream os;
    Clip* current_clip = owner_->get_current_clip();
    os << "Toonloop " << PACKAGE_VERSION << 
        (isUnstable ? " (UNSTABLE) " : "") << 
        (isGit ? "(Git)" : "") << std::endl << std::endl <<
        "Press i to hide info" << std::endl;
    os << "OpenGL rendering rate: " << rendering_fps_ << " FPS" << std::endl;
    os << "Layout: " << get_layout_name(current_layout_);
    os << std::endl;
    os << std::endl;
    os << "Current clip: " << current_clip->get_id() << std::endl;
    os << "Direction: " << current_clip->get_direction() << std::endl;
    os << "FPS: " << current_clip->get_playhead_fps() << std::endl;
    os << "Playhead: " << current_clip->get_playhead() << std::endl;
    os << "Writehead: " << current_clip->get_writehead() << "/" << current_clip->size() << std::endl;
    os << "Intervalometer rate: " << current_clip->get_intervalometer_rate() << std::endl;
    os << "Intervalometer enabled:" << owner_->get_pipeline()->get_intervalometer_is_on() << std::endl;
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

static void set_blending_mode_for_texture(ClutterTexture *texture, const gchar *blend)
{
    CoglHandle material = clutter_texture_get_cogl_material(texture);
    GError *error = NULL;
#if CLUTTER_CHECK_VERSION(1, 4, 0)
    gboolean success = cogl_material_set_blend(COGL_MATERIAL(material), blend, &error);
#else
    gboolean success = cogl_material_set_blend(material, blend, &error);
#endif

    if (error)
    {
        g_critical("Error setting blend: %s\n", error->message);
        g_error_free(error);
    }
    if (! success)
    {
        g_critical("Could not set blend mode");
    }
}

void Gui::set_blending_mode(BlendingMode mode)
{
    static const gchar *NORMAL_BLEND_MODE = "RGBA = ADD (SRC_COLOR * (SRC_COLOR[A]), DST_COLOR * (1-SRC_COLOR[A]))";
    static const gchar *ADD_BLEND_MODE = "RGBA = ADD (SRC_COLOR * (SRC_COLOR[A]), DST_COLOR)";
    if (mode == blending_mode_)
        return;
    const gchar *blend;
    blending_mode_ = mode;
    switch (blending_mode_)
    { 
        case BLENDING_MODE_NORMAL:
            blend = NORMAL_BLEND_MODE;
            break;
        case BLENDING_MODE_ADDITIVE:
            blend = ADD_BLEND_MODE;
            break;
    } 
    // set mode for all textures:
    set_blending_mode_for_texture(CLUTTER_TEXTURE(live_input_texture_), blend);
    for (ActorIterator iter = playback_textures_.begin(); iter != playback_textures_.end(); ++iter)
        set_blending_mode_for_texture(CLUTTER_TEXTURE(*iter), blend);
    for (ActorIterator iter = onionskin_textures_.begin(); iter != onionskin_textures_.end(); ++iter)
        set_blending_mode_for_texture(CLUTTER_TEXTURE(*iter), blend);
}

Gui::~Gui()
{
    //std::cout << "~Gui" << std::endl;
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

std::string Gui::get_blending_mode_name(BlendingMode mode)
{
    switch (mode)
    {
        case BLENDING_MODE_ADDITIVE:
            return std::string("additive");
            break;
        case BLENDING_MODE_NORMAL:
            return std::string("normal");
            break;
        default: 
            return std::string("unknown");
            break;
    }
}
