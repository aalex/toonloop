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
    LOG_DEBUG("add_frame to clip " << clip->get_id());
    unsigned int new_frame_number = clip->get_writehead();
    owner_->get_pipeline()->grab_frame();
    add_frame_signal_(clip->get_id(), new_frame_number);
}
void Controller::remove_frame()
{
    Clip* clip = owner_->get_current_clip();
    LOG_DEBUG("remove frame from clip " << clip->get_id());
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
        LOG_ERROR("Invalid clip number " << clip_number);
    if (current_clip == clip_number)
        LOG_DEBUG("Already chosen clip number " << clip_number);
    else 
    {
        owner_->set_current_clip_number(clip_number);
        LOG_DEBUG("choose_clip " << clip_number);
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
    unsigned int current_clip = owner_->get_current_clip_number();
    //save_clip(current_clip);
    std::cout << "Saving clip #" << current_clip << std::endl;
    Clip* clip = owner_->get_current_clip();
    if (clip->size() == 0)
    {
        std::cout << "Clip is empty: " << current_clip << std::endl;
        // return false;
    } else {
        owner_->get_movie_saver()->add_saving_task(*clip);
        //TODO: save_clip_signal_ should only be called when done saving.
        save_clip_signal_(current_clip, "TODO: add file name");
    }
}
/**
 * Times the playhead and iterate it if it's time to.
 */
void Controller::update_playback_image()
{
    static int prev_image_number = -1;
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

        if ((prevclip != thisclip) or (prev_image_number != image_number))
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
            }
        }
        prev_image_number = image_number;
    } 

}
