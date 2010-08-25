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
#include <boost/signals2.hpp>
#include "application.h"
#include "clip.h"
#include "controller.h"
#include "log.h"

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
        LOG_ERROR("Already chosen clip number " << clip_number);
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

void Controller::save_current_clip()
{
    unsigned int current_clip = owner_->get_current_clip_number();
    //save_clip(current_clip);
    owner_->save_current_clip();
    save_clip_signal_(current_clip);
}

