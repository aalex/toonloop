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
#include "application.h"
#include "clip.h"
#include "gui.h"
#include "infowindow.h"
#include "pipeline.h"
#include "unused.h"

/**
 * Window to display some information. 
 */
InfoWindow::InfoWindow(Application *app) : 
    app_(app),
    stage_(NULL),
    text_(NULL)
{
}

void InfoWindow::on_window_destroyed(ClutterActor &stage, gpointer data)
{
    UNUSED(stage);
    InfoWindow *self = static_cast<InfoWindow *>(data);
    std::cout << "Info window has been deleted" << std::endl;
    self->app_->quit();
}

// static gboolean idle_cb(gpointer data)
// {
//     clutter_actor_queue_redraw(CLUTTER_ACTOR(data));
//     return TRUE;
// }

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
        ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
        ClutterColor light_gray = { 0xcc, 0xcc, 0xcc, 0xff };
        ClutterColor background_color = { 0x00, 0x00, 0x00, 0xff };

        clutter_stage_set_color(CLUTTER_STAGE(stage_), &black);
        clutter_stage_set_title(CLUTTER_STAGE(stage_), "Toonloop Information");
        // TODO:make stage's size configurable
        gfloat width = 320.0f;
        gfloat height = 240.0f;
        clutter_actor_set_size(stage_, width, height);

        // stroke
        ClutterActor *rect1 = clutter_rectangle_new_with_color(&light_gray);
        clutter_actor_set_size(rect1, width, height);
        clutter_container_add_actor(CLUTTER_CONTAINER(stage_), rect1);

        // background color
        ClutterActor *rect2 = clutter_rectangle_new_with_color(&background_color);
        clutter_actor_set_size(rect2, width - 4, height - 4);
        clutter_actor_set_position(rect2, 2, 2);
        clutter_container_add_actor(CLUTTER_CONTAINER(stage_), rect2);
        
        text_ = clutter_text_new_full("Sans semibold 12px", "", &white);
        clutter_actor_set_position(rect2, 4, 4);
        clutter_container_add_actor(CLUTTER_CONTAINER(stage_), text_);

        g_signal_connect(CLUTTER_STAGE(stage_), "delete-event", G_CALLBACK(InfoWindow::on_window_destroyed), this);
        //g_idle_add(idle_cb, stage_);

        clutter_actor_show(stage_);
    }
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
