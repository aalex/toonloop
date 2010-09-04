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
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>
#include "application.h"
#include "clip.h"
#include "moviesaver.h"
#include "pipeline.h"
#include "controller.h"
#include "timer.h"
#include "log.h"

namespace fs = boost::filesystem;

Controller::Controller(Application* owner) : 
    owner_(owner)
{
    
}
void Controller::add_frame()
{
    Clip* clip = owner_->get_current_clip();
    LOG_DEBUG("add_frame to clip #" << clip->get_id());
    unsigned int new_frame_number = clip->get_writehead();
    owner_->get_pipeline()->grab_frame();
    add_frame_signal_(clip->get_id(), new_frame_number);
}
void Controller::remove_frame()
{
    Clip* clip = owner_->get_current_clip();
    LOG_DEBUG("remove frame from clip #" << clip->get_id());
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
        LOG_DEBUG("choose_clip #" << clip_number);
        choose_clip_signal_(clip_number);
    }
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
        owner_->get_movie_saver()->add_saving_task(*clip);
        //TODO: save_clip_signal_ should only be called when done saving.
        save_clip_signal_(current_clip_number, "TODO: add file name");
    }
}

void Controller::update_playback_image()
{
    static std::string prev_image_name = "";
    static Clip *prevclip = NULL;
    static Timer playback_timer = Timer(); // TODO: move to Clip

    Clip *thisclip = owner_->get_current_clip();
    bool move_playhead = false;
    bool need_refresh = false;
    

    playback_timer.tick();

    // check if it is time to move the playhead
    if ((playback_timer.get_elapsed()) >=  (1.0f / thisclip->get_playhead_fps() * 1.0) || playback_timer.get_elapsed() < 0.0f)
    {
        move_playhead = true;
        playback_timer.reset();
    }

    if (thisclip->size() == 0)
        no_image_to_play_signal_();
    
    if(thisclip->size() > 0) 
    {     
        if (move_playhead) // if it's time to move the playhead
            thisclip->iterate_playhead(); // updates the clip's playhead number
        int image_number = thisclip->get_playhead();
        //width = thisclip->get_width(); 
        //height = thisclip->get_height();
        
        // Aug 25 2010:tmatth:FIXME: when deleting frames, image_number can be invalid
        Image* thisimage = thisclip->get_image(image_number);
        if (thisimage == 0)
        {
            std::cerr << "Controller::update_playback_image: The image is NULL!" << std::endl;
        } else {
            if ((prevclip != thisclip) or (prev_image_name != thisimage->get_name()))
                  need_refresh = true;
            if (prevclip != thisclip) 
                prevclip = thisclip;
            
            if (thisimage == NULL)
                std::cout << "No image at index" << image_number << "." << std::endl;
            else 
            {
                if (need_refresh)
                {
                    std::string image_full_path = thisclip->get_image_full_path(thisimage);
                    if (fs::exists(image_full_path))
                        next_image_to_play_signal_(thisclip->get_id(), image_number, image_full_path);
                    prev_image_name = thisimage->get_name();
                }
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
    clip_direction current = current_clip->get_direction();
    clip_direction change_to = DIRECTION_FORWARD; // default
    std::string signal_arg = "FORWARD";
    switch (current)
    {
        case DIRECTION_FORWARD:
            change_to = DIRECTION_BACKWARD;
            signal_arg = "BACKWARD";
            break;
        case DIRECTION_BACKWARD:
            change_to = DIRECTION_YOYO;
            signal_arg = "YOYO";
            break;
        case DIRECTION_YOYO:
            change_to = DIRECTION_FORWARD;
            signal_arg = "FORWARD";
            break;
    }
    current_clip->set_direction(change_to);
    clip_direction_changed_signal_(current_clip->get_id(), signal_arg);
}

void Controller::set_current_clip_direction(clip_direction direction)
{
    Clip *current_clip = owner_->get_current_clip();
    clip_direction current = current_clip->get_direction();
    if (current != direction)
    {
        clip_direction change_to = DIRECTION_FORWARD; // default
        //TODO: the clip_direction_changed_signal should accept a constant, not a string
        // That will be way simpler
        std::string signal_arg = "FORWARD";
        switch (current)
        {
            case DIRECTION_FORWARD:
                change_to = DIRECTION_BACKWARD;
                signal_arg = "BACKWARD";
                break;
            case DIRECTION_BACKWARD:
                change_to = DIRECTION_YOYO;
                signal_arg = "YOYO";
                break;
            case DIRECTION_YOYO:
                change_to = DIRECTION_FORWARD;
                signal_arg = "FORWARD";
                break;
        }
        current_clip->set_direction(change_to);
        clip_direction_changed_signal_(current_clip->get_id(), signal_arg);
    }
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

