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

#include <clutter/clutter.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "application.h"
#include "clip.h"
#include "gui.h"
#include "infowindow.h"
#include "pipeline.h"
#include "unused.h"
#include "controller.h"

static ClutterColor gray = { 0x99, 0x99, 0x99, 0xff };
static ClutterColor white = { 0xff, 0xff, 0xff, 0xff };

/**
 * Window to display some information. 
 */
InfoWindow::InfoWindow(Application *app) : 
    app_(app),
    stage_(NULL),
    text_(NULL),
    clipping_group_(NULL),
    scrollable_box_(NULL),
    previously_selected_(0)
{
}

void InfoWindow::on_window_destroyed(ClutterActor &stage, gpointer data)
{
    UNUSED(stage);
    InfoWindow *self = static_cast<InfoWindow *>(data);
    std::cout << "Info window has been deleted" << std::endl;
    self->app_->quit();
}
/**
 * Enables the whole info window.
 * 
 * If this is not called, nothing will be drawn by this whole class.
 */
void InfoWindow::create()
{
    stage_ = clutter_stage_new();
    if (! stage_)
    {
        g_critical("Could not get a stage. The Clutter backend might not support multiple stages.");
    }
    else
    {
        ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
        clutter_stage_set_color(CLUTTER_STAGE(stage_), &black);
        clutter_stage_set_title(CLUTTER_STAGE(stage_), "Toonloop Information");
        
        gfloat width = 640.0f;
        gfloat height = 480.0f;
        clutter_actor_set_size(stage_, width, height);

        // TEXT ABOUT EVERYTHING
        ClutterActor *text_group = clutter_group_new();
        text_ = clutter_text_new_full("Sans semibold 12px", "", &white);
        clutter_container_add_actor(CLUTTER_CONTAINER(text_group), text_);
        clutter_container_add_actor(CLUTTER_CONTAINER(stage_), text_group);
        
        // each image will be 80x60.
        // Plus some text under it
        gdouble EACH_PADDING = 4;
        clipping_group_ = clutter_group_new(); // FIXME: memleak?
        clutter_actor_set_size(clipping_group_, 620.0, 220.0);
        clutter_actor_set_position(clipping_group_, 0.0, 180.0);


        static const float EACH_CLIP_ACTOR_WIDTH = 80.0;
        scrollable_box_ = clutter_group_new();
        clutter_container_add_actor(CLUTTER_CONTAINER(clipping_group_), scrollable_box_);
        clutter_container_add_actor(CLUTTER_CONTAINER(stage_), clipping_group_);

        // Add the stuff for the clips:
        // TODO: stop using the MAX_CLIPS constant
        for (unsigned int i = 0; i < MAX_CLIPS; i++)
        {
            using std::tr1::shared_ptr;
            clips_.push_back(shared_ptr<ClipInfoBox>(new ClipInfoBox()));
            ClipInfoBox *clip_info_box = clips_.at(i).get();
            clip_info_box->group_ = clutter_group_new();
            clutter_actor_set_size(clip_info_box->group_, EACH_CLIP_ACTOR_WIDTH, 200);

            clip_info_box->rect_ = clutter_rectangle_new_with_color(&gray);
            clutter_actor_set_position(clip_info_box->rect_, 0, 0);
            clutter_actor_set_size(clip_info_box->rect_, 80, 60);
            clutter_container_add_actor(CLUTTER_CONTAINER(clip_info_box->group_), clip_info_box->rect_);

            clip_info_box->image_ = clutter_texture_new();
            clutter_actor_set_position(clip_info_box->image_, 0, 60);
            clutter_container_add_actor(CLUTTER_CONTAINER(clip_info_box->group_), clip_info_box->image_);

            clip_info_box->label_ = clutter_text_new_full("Sans semibold 10px", "", &white);
            clutter_actor_set_position(clip_info_box->label_, 2.0, 2.0);
            clutter_container_add_actor(CLUTTER_CONTAINER(clip_info_box->group_), clip_info_box->label_);
            clutter_actor_set_position(clip_info_box->label_, 10, 16);
            
            clip_info_box->position_ = EACH_CLIP_ACTOR_WIDTH * i + i * EACH_PADDING * 2;
            clutter_actor_set_position(clip_info_box->group_, clip_info_box->position_, 0);
            std::cout << "position:" << clip_info_box->position_ << std::endl;
            clutter_container_add_actor(CLUTTER_CONTAINER(scrollable_box_), clip_info_box->group_);
            // set the label's text
            update_num_frames(i);
        }

        g_signal_connect(CLUTTER_STAGE(stage_), "delete-event", G_CALLBACK(InfoWindow::on_window_destroyed), this);

        Controller *controller = app_->get_controller();
        controller->choose_clip_signal_.connect(boost::bind( 
            &InfoWindow::on_choose_clip, this, _1));
        controller->add_frame_signal_.connect(boost::bind(
            &InfoWindow::on_add_frame, this, _1, _2));
        controller->remove_frame_signal_.connect(boost::bind(
            &InfoWindow::on_remove_frame, this, _1, _2));
        controller->clip_cleared_signal_.connect(boost::bind(
            &InfoWindow::on_clip_cleared, this, _1));

        on_choose_clip(0);
        clutter_actor_show(stage_);
    }
}

/** Slot for Controller::add_frame_signal_ 
 */
void InfoWindow::on_add_frame(unsigned int clip_number, unsigned int frame_number)
{
    UNUSED(frame_number);
    update_num_frames(clip_number);
    if (frame_number == 0)
    {
        update_thumbnail_for_clip(clip_number);
    }
}

void InfoWindow::update_thumbnail_for_clip(unsigned int clip_number)
{
    if (clip_number >= clips_.size())
    {
        g_critical("%s: Clip number bigger than size of known clips.", __FUNCTION__);
        return;
    }
    Clip *clip = app_->get_clip(clip_number);
    ClipInfoBox *clip_info = clips_.at(clip_number).get();
    unsigned int frames = clip->size();
    if (frames == 0)
        clutter_actor_hide(clip_info->image_);
    else
    {
        clutter_actor_show(clip_info->image_);
        Image *image = clip->get_image(0);
        if (image == 0)
            std::cout << __FUNCTION__ << ": Could not get a handle to any image!" << std::endl;
        else
        {
            std::string file_name = clip->get_image_full_path(image);
            load_thumbnail_from_file(CLUTTER_TEXTURE(clip_info->image_), file_name, 80, 60);
        }
    }
}

/** Slot for Controller::remove_frame_signal_ 
 */
void InfoWindow::on_remove_frame(unsigned int clip_number, unsigned int frame_number)
{
    UNUSED(frame_number);
    update_num_frames(clip_number);
    if (frame_number == 0) // maybe the last left
        update_thumbnail_for_clip(clip_number);
}

/** Slot for Controller::clip_cleared_signal_ 
 */
void InfoWindow::on_clip_cleared(unsigned int clip_number)
{
    update_num_frames(clip_number);
    update_thumbnail_for_clip(clip_number);
}

void InfoWindow::update_num_frames(unsigned int clip_number)
{
    if (clip_number >= clips_.size())
    {
        g_critical("%s: Clip number bigger than size of known clips.", __FUNCTION__);
        return;
    }
    Clip *clip = app_->get_clip(clip_number);
    ClipInfoBox *clip_info = clips_.at(clip_number).get();
    unsigned int frames = clip->size();
    std::ostringstream os;
    os << "  # <b>" << clip_number << "</b>" << std::endl;
    os << "images: " << frames;
    clutter_text_set_markup(CLUTTER_TEXT(clip_info->label_), os.str().c_str());
}

/** Slot for Controller::choose_clip_signal_
 * */
void InfoWindow::on_choose_clip(unsigned int clip_number)
{
    static ClutterColor red = { 255, 0, 0, 255 };
    if (clip_number >= clips_.size())
    {
        g_critical("%s: Clip number bigger than size of known clips.", __FUNCTION__);
        return;
    }
    ClipInfoBox *current = clips_.at(clip_number).get();
    //std::cout << __FUNCTION__ << " " << clip_number << " goto x=" << current->position_ << std::endl;
    //clutter_actor_animate(scrollable_box_, CLUTTER_EASE_IN_OUT_SINE, 200,
    //    "x", current->position_, 
    //    NULL);
    gdouble goto_pos = 300 - current->position_;
    clutter_actor_set_x(scrollable_box_, goto_pos);
    std::cout << "GOTO position:" << goto_pos << std::endl;
    if (previously_selected_ >= clips_.size())
    {
        g_critical("%s: Clip number bigger than size of known clips.", __FUNCTION__);
        return;
    }
    ClipInfoBox *previous = clips_.at(previously_selected_).get();
    clutter_rectangle_set_color(CLUTTER_RECTANGLE(previous->rect_), &gray);
    clutter_rectangle_set_color(CLUTTER_RECTANGLE(current->rect_), &red);
    
    previously_selected_ = clip_number;
}

void InfoWindow::load_thumbnail_from_file(ClutterTexture *texture, const std::string &file_name, int width, int height)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file_at_size(
        file_name.c_str(),
        width,
        height,
        &error);
    if (!pixbuf)
    {
        g_warning ("Error loading pixbuf: %s", error->message);
        g_error_free (error);
        error = NULL;
        return;
    }
    clutter_texture_set_from_rgb_data(
        texture,
        gdk_pixbuf_get_pixels (pixbuf),
        gdk_pixbuf_get_has_alpha (pixbuf),
        gdk_pixbuf_get_width (pixbuf),
        gdk_pixbuf_get_height (pixbuf),
        gdk_pixbuf_get_rowstride (pixbuf),
        gdk_pixbuf_get_n_channels (pixbuf),
        (ClutterTextureFlags) 0,
        &error);
    if (error)
    {
        g_warning ("Error loading the pixbuf's pixels into texture: %s", error->message);
        g_error_free (error);
        error = NULL;
    }
    g_object_unref(pixbuf);
}

void InfoWindow::update_info_window()
{
    if (! text_)
        return;
    Gui *gui = app_->get_gui();
    Clip* current_clip = app_->get_current_clip();
    Gui::layout_number current_layout = gui->get_layout();
    Gui::BlendingMode blending_mode = gui->get_blending_mode();
    std::ostringstream os;

    os << "Layout: " << current_layout << " (" << gui->get_layout_name(current_layout) << ")" << std::endl;
    os << "Blending mode: " << blending_mode << " (" << gui->get_blending_mode_name(blending_mode) << ")" << std::endl;
    os << std::endl;
    os << "CLIP: " << current_clip->get_id() << std::endl;
    os << "  FPS: " << current_clip->get_playhead_fps() << std::endl;
    os << "  Playhead: " << current_clip->get_playhead() << std::endl;
    os << "  Writehead: " << current_clip->get_writehead() << "/" << current_clip->size() << std::endl;
    os << "  Direction: " << Clip::get_direction_name(current_clip->get_direction()) << std::endl;
    os << std::endl;
    os << "  Intervalometer rate: " << current_clip->get_intervalometer_rate() << std::endl;
    os << "  Intervalometer enabled: " << (app_->get_pipeline()->get_intervalometer_is_on() ? "yes" : "no") << std::endl;
    clutter_text_set_text(CLUTTER_TEXT(text_), os.str().c_str());
}
