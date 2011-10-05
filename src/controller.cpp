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
#include <boost/filesystem.hpp>
#include "application.h"
#include "clip.h"
#include "moviesaver.h"
#include "pipeline.h"
#include "controller.h"
#include "statesaving.h"
#include "timer.h"
#include "log.h"
#include "image_importer.h"

Controller::Controller(Application* owner) : 
    owner_(owner),
    playback_enabled_(true)
{
    
}

Property<int> *Controller::add_int_property(const std::string &name, int value)
{
    return int_properties_.add_property(name, value);
}

Property<float> *Controller::add_float_property(const std::string &name, float value)
{
    return float_properties_.add_property(name, value);
}

bool Controller::set_int_value(const std::string &name, int value)
{
    return int_properties_.set_property_value(name, value);
}

bool Controller::set_float_value(const std::string &name, float value)
{
    return float_properties_.set_property_value(name, value);
}

int Controller::get_int_value(const std::string &name)
{
    return int_properties_.get_property_value(name);
}

float Controller::get_float_value(const std::string &name)
{
    return float_properties_.get_property_value(name);
}

void Controller::import_image(const std::string &file_name)
{
    owner_->get_pipeline()->import_image(file_name);
}

void Controller::add_frame()
{
    //Clip* clip = owner_->get_current_clip();
    //LOG_DEBUG("add_frame to clip #" << clip->get_id());
    //unsigned int new_frame_number = clip->get_writehead();
    owner_->get_pipeline()->grab_frame();
    //Moved the call to the add_frame_signal_ to pipeline::save_image_to_current_clip
    //add_frame_signal_(clip->get_id(), new_frame_number);
}
void Controller::remove_frame()
{
    Clip* clip = owner_->get_current_clip();
    //LOG_DEBUG("remove frame from clip #" << clip->get_id());
    int deleted_frame_number = clip->get_writehead() - 1;
    if (deleted_frame_number < 0)
        deleted_frame_number = 0;
    // TODO:2010-08-25:aalex:Check for deletion success
    owner_->get_pipeline()->remove_frame();
    remove_frame_signal_(clip->get_id(), (unsigned int) deleted_frame_number);
}
void Controller::choose_clip(unsigned int clip_number)
{
    unsigned int current_clip = owner_->get_current_clip_number();
    if (clip_number > MAX_CLIPS)
        LOG_ERROR("Invalid clip #" << clip_number);
    if (current_clip == clip_number)
        LOG_DEBUG("Already chosen clip #" << clip_number);
    else 
    {
        owner_->set_current_clip_number(clip_number);
        if (owner_->get_configuration()->get_home_when_choose())
        {
            owner_->get_current_clip()->goto_beginning();
        }
        //LOG_DEBUG("choose_clip #" << clip_number);
        choose_clip_signal_(clip_number);
    }
}

void Controller::choose_clip_and_add_frame(unsigned int clip_number)
{
    choose_clip(clip_number);
    add_frame();
}

void Controller::choose_previous_clip()
{
    unsigned int current_clip = owner_->get_current_clip_number();
    if (current_clip > 0)
        choose_clip(current_clip - 1);
}
void Controller::choose_next_clip()
{
    unsigned int current_clip = owner_->get_current_clip_number();
    if (current_clip < MAX_CLIPS - 1)
        choose_clip(current_clip + 1);
}
// void Controller::save_clip(unsigned int clip_number)
// {
//     LOG_DEBUG("save_clip : TODO" << clip_number);
//     // TODO:2010-08-25:aalex:Allow to save any clip.
//     // Not only the current one
// }

/**
 * Saves the current clip
 */
void Controller::save_current_clip()
{
    unsigned int current_clip_number = owner_->get_current_clip_number();
    //save_clip(current_clip);
    std::cout << "Saving clip #" << current_clip_number << std::endl;
    Clip* clip = owner_->get_current_clip();
    if (clip->size() == 0)
    {
        std::cout << "Clip is empty: #" << current_clip_number << std::endl;
        // return false;
    } else {
        std::string file_name = "";
        owner_->get_movie_saver()->add_saving_task(*clip, file_name);
        //TODO: save_clip_signal_ should only be called when done saving.
        save_clip_signal_(current_clip_number, file_name);
    }
}

void Controller::update_playback_image()
{
    namespace fs = boost::filesystem;
    Clip *thisclip = owner_->get_current_clip();
    bool move_playhead = false;

    playback_timer_.tick();
    // check if it is time to move the playhead
    if ((playback_timer_.get_elapsed()) >=  (1.0f / thisclip->get_playhead_fps() * 1.0) || playback_timer_.get_elapsed() < 0.0f)
    {
        move_playhead = true;
        playback_timer_.reset();
    }
    if (! playback_enabled_)
        move_playhead = false;

    if (move_playhead) // if it's time to move the playhead
        thisclip->iterate_playhead(); // updates the clip's playhead number
    advertise_current_image();
}

void Controller::advertise_current_image()
{
    namespace fs = boost::filesystem;
    Clip *thisclip = owner_->get_current_clip();
    if (thisclip->size() == 0)
        no_image_to_play_signal_();
    else
    {     
        int image_number = thisclip->get_playhead();
        Image* thisimage = thisclip->get_image(image_number);
        if (thisimage == 0)
            std::cerr << "Controller::" << __FUNCTION__ << ": The image is NULL!" << std::endl;
        else
        {
            bool need_refresh = false;
            if ((prev_clip_id_ != thisclip->get_id()) || (prev_image_name_ != thisimage->get_name()))
                  need_refresh = true;
            prev_clip_id_ = thisclip->get_id();
            if (need_refresh)
            {
                std::string image_full_path = thisclip->get_image_full_path(thisimage);
                if (fs::exists(image_full_path))
                    next_image_to_play_signal_(thisclip->get_id(), image_number, image_full_path);
                else
                    std::cerr << "Controller::" << __FUNCTION__ << ": The image does not exist: " << image_full_path << std::endl;
                prev_image_name_ = thisimage->get_name();
            }
        }
    }
}

void Controller::toggle_video_grabbing()
{
    Pipeline *pipeline = owner_->get_pipeline();
    if (pipeline->get_record_all_frames())
    {
        enable_video_grabbing(false);
    } else {
        enable_video_grabbing(true);
    }
}

void Controller::enable_video_grabbing(bool enable)
{
    Pipeline *pipeline = owner_->get_pipeline();
    Clip *current_clip = owner_->get_current_clip();
    if (pipeline->get_record_all_frames())
    {
        if (!enable)
        {
            pipeline->set_record_all_frames(false);
            //FIXME: This should be a property of Clip, not of Pipeline
            //TODO: rename pipeline::get_record_all_frames to a better name
            clip_videograb_changed_signal_(current_clip->get_id(), false);
        } else
            std::cout << "Video grabbing was already disabled." << std::endl;

    } else {
        if (enable)
        {
            pipeline->set_record_all_frames(true);
            clip_videograb_changed_signal_(current_clip->get_id(), true);
        } else
            std::cout << "Video grabbing was already enabled." << std::endl;
    }
}

void Controller::increase_playhead_fps()
{

    Clip *current_clip = owner_->get_current_clip();
    current_clip->increase_playhead_fps();
    clip_fps_changed_signal_(current_clip->get_id(), current_clip->get_playhead_fps());
}

void Controller::decrease_playhead_fps()
{
    Clip *current_clip = owner_->get_current_clip();
    current_clip->decrease_playhead_fps();
    clip_fps_changed_signal_(current_clip->get_id(), current_clip->get_playhead_fps());
}

void Controller::set_playhead_fps(unsigned int fps)
{
    Clip *current_clip = owner_->get_current_clip();
    current_clip->set_playhead_fps(fps);
    clip_fps_changed_signal_(current_clip->get_id(), current_clip->get_playhead_fps());
}

void Controller::change_current_clip_direction()
{
    Clip *current_clip = owner_->get_current_clip();
    current_clip->change_direction();
    clip_direction_changed_signal_(current_clip->get_id(), current_clip->get_direction());
}

void Controller::set_current_clip_direction(const std::string &direction)
{
    Clip *current_clip = owner_->get_current_clip();
    bool success = current_clip->set_direction(direction);
    if (success)
        clip_direction_changed_signal_(current_clip->get_id(), direction);
    else
        std::cout << "Invalid playhead direction: " << direction << std::endl;
}

void Controller::clear_current_clip()
{
    Clip *current_clip = owner_->get_current_clip();
    current_clip->clear_all_images();
    clip_cleared_signal_(current_clip->get_id());
}


void Controller::set_intervalometer_rate(float rate)
{
    Clip *current_clip = owner_->get_current_clip();
    if (rate < 0.0f)
        rate = 0.0f;
    current_clip->set_intervalometer_rate(rate);
    intervalometer_rate_changed_signal_(current_clip->get_id(), current_clip->get_intervalometer_rate());
}

void Controller::increase_intervalometer_rate()
{
    Clip *current_clip = owner_->get_current_clip();
    float rate = current_clip->get_intervalometer_rate();
    set_intervalometer_rate(rate + 1.0f);
}

void Controller::decrease_intervalometer_rate()
{
    Clip *current_clip = owner_->get_current_clip();
    float rate = current_clip->get_intervalometer_rate();
    if (rate > 1.0f)
        set_intervalometer_rate(rate - 1.0f);
    // else Already at its minimum!
}

void Controller::toggle_intervalometer()
{
    Pipeline *pipeline = owner_->get_pipeline();
    if (pipeline->get_intervalometer_is_on())
    {
        enable_intervalometer(false);
    } else {
        enable_intervalometer(true);
    }
}

void Controller::enable_intervalometer(bool enable)
{
    Pipeline *pipeline = owner_->get_pipeline();
    Clip *current_clip = owner_->get_current_clip();
    if (pipeline->get_intervalometer_is_on())
    {
        if (!enable)
        {
            pipeline->set_intervalometer_is_on(false);
            //FIXME: This should be a property of Clip, not of Pipeline
            intervalometer_toggled_signal_(current_clip->get_id(), false);
        } else
            std::cout << "Video grabbing was already disabled." << std::endl;

    } else {
        if (enable)
        {
            pipeline->set_intervalometer_is_on(true);
            intervalometer_toggled_signal_(current_clip->get_id(), true);
        } else
            std::cout << "Video grabbing was already enabled." << std::endl;
    }
}

void Controller::move_writehead_to_next()
{
    Clip *current_clip = owner_->get_current_clip();
    move_writehead_to(current_clip->get_writehead() + 1);
}

void Controller::move_writehead_to_previous()
{
    Clip *current_clip = owner_->get_current_clip();
    unsigned int current_position = current_clip->get_writehead();
    if (current_position != 0)
        move_writehead_to(current_position - 1);
}

void Controller::move_writehead_to_last()
{
    Clip *current_clip = owner_->get_current_clip();
    move_writehead_to(current_clip->size());
}


void Controller::move_writehead_to_first()
{
    move_writehead_to(0);
}

void Controller::move_writehead_to(unsigned int position)
{
    Clip *current_clip = owner_->get_current_clip();
    if (current_clip->get_writehead() != position)
    {
        current_clip->set_writehead(position);
        writehead_moved_signal_(current_clip->get_id(), current_clip->get_writehead());
    }
}

void Controller::quit()
{
    owner_->quit();
}

void Controller::print_properties()
{
    std::cout << "Toonloop properties:" << std::endl;
    int_properties_.print_properties();
    //std::cout << "Toonloop float properties:" << std::endl;
    float_properties_.print_properties();
}

void Controller::save_project()
{
    namespace ss = statesaving;
    std::string file_name = owner_->get_project_file_name();
    owner_->save_project(file_name);
    save_project_signal_(file_name);
}

void Controller::move_playhead_to(unsigned int position)
{
    Clip *current_clip = owner_->get_current_clip();
    current_clip->set_playhead(position);
    advertise_current_image();
}

void Controller::playback_toggle(bool enabled)
{
    playback_enabled_ = enabled;
    //std::cout << "toggle playback " << enabled << std::endl;
    playback_toggled_signal_(playback_enabled_);
}

