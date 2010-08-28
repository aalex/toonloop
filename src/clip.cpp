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

#include "clip.h"
#include "configuration.h"
#include "image.h"
#include "timing.h"
#include <string>
#include <iostream>
#include <tr1/memory>
#include <boost/thread/mutex.hpp>

// FIXME: vector is not thread safe. You need to protect it with a mutex or such.
/** Each clip needs a unique ID.
 */
Clip::Clip(unsigned int id)
{
    // FIXME: How to use a 2-int vector?
    //intervalometer_rate_(1, 1); // default: 1 FPS
    //fps_(12, 1); // default: 12 FPS
    id_ = id;
    writehead_ = 0;
    playhead_ = 0;
    playhead_fps_ = 12; // some default for now
    has_recorded_a_frame_ = false;
    direction_ = DIRECTION_FORWARD;
    yoyo_sub_direction_ = DIRECTION_FORWARD;
    //mutex_;
}

void Clip::set_directory_path(const std::string &directory_path)
{
    directory_path_ = directory_path;
}

// TODO: make sure the number of consecutive slashes in the path is ok
std::string Clip::get_image_full_path(Image* image) const
{
    std::string project_path = get_directory_path();
    std::string image_name = image->get_name() + get_image_file_extension(); //".jpg";
    // TODO: use boost:file_system to append paths
    return project_path + "/" + IMAGES_DIRECTORY + "/" + image_name; 
}

unsigned int Clip::get_playhead_fps() const
{
    return playhead_fps_;
}

void Clip::set_playhead_fps(unsigned int fps)
{
    playhead_fps_ = fps;
}

unsigned int Clip::get_id() const
{
    return id_;
}

unsigned int Clip::get_width() const
{
    return width_;
}

unsigned int Clip::get_height() const
{
    return height_;
}

void Clip::set_width(unsigned int width)
{
    width_ = width;
}

void Clip::set_height(unsigned int height)
{
    height_ = height;
}
/**
 * Adds an image to the clip.
 * Returns the its index.
 */
unsigned int Clip::frame_add()
{
    using namespace std::tr1; // shared_ptr

    unsigned int assigned = writehead_;
    std::string name = timing::get_iso_datetime_for_now();
    //images_.push_back(shared_ptr<Image>(new Image(name)));
    images_.insert(images_.begin() + writehead_, shared_ptr<Image>(new Image(name)));
    //images_[writehead_] = new Image(name);
    writehead_++;
    return assigned;
}

/**
 * Delete an image for the clip.
 * Returns how many images it has deleted. (0 or 1)
 */
// FIXME: this is not thread-safe, isn't it? (we must used shared_ptr)
unsigned int Clip::frame_remove()
{
    unsigned int how_many_deleted = 0;
    if (images_.empty()) 
    {
        std::cout << "Cannot delete a frame since the clip is empty." << std::endl;
    } 
    else if (size() == 1 && writehead_ == 1)
    {
        clear_all_images(); // takes care of writehead_ and playhead_
    }
    else if (writehead_ > images_.size()) 
    {
        std::cout << "Cannot delete a frame since the writehead points to a " <<
            "non-existing frame index " << writehead_ << " while the clip has only " << 
        images_.size() << " images." << std::endl;
        //TODO:2010-08-27:aalex:Move the writehead to the end of clip and erase a frame anyways.
    } 
    else if (writehead_ == 0)
    {
        std::cout << "Cannot delete a frame since writehead is at 0" << std::endl;
    }
    else 
    {
        std::cout << "Deleting image at position " << (writehead_ - 1) << "/" << (images_.size()  - 1) << std::endl;
        images_.erase(images_.begin() + (writehead_ - 1));
        how_many_deleted = 1;
        // let's decrement the writehead and playhead
        --writehead_;
        if (playhead_ >= size())
        {
            if (size() == 0)
                playhead_ = 0;
            else
                playhead_ = size() - 1;
        }
    }
    return how_many_deleted;
}


unsigned int Clip::get_playhead() const
{
    return playhead_;
}

unsigned int Clip::get_writehead() const
{
    return writehead_;
}

unsigned int Clip::iterate_playhead()
{
    unsigned int len = size();
    if (len == 0)
        playhead_ = 0;
    else 
    {
        // clip has at leat 1 of length
        switch (direction_)
        {
        case DIRECTION_FORWARD:
            if (playhead_ >= len - 1)
                playhead_ = 0;
            else 
                ++playhead_;
            break;
        case DIRECTION_BACKWARD:
            if (playhead_ == 0)
                playhead_ = len - 1;
            else 
                --playhead_;
            break;
        // a slightly more complex type is the yoyo:
        case DIRECTION_YOYO:
            if (yoyo_sub_direction_ == DIRECTION_BACKWARD)
            {
                if (playhead_ == 0)
                {
                    playhead_ = 1;
                    yoyo_sub_direction_ = DIRECTION_FORWARD;
                } else 
                    --playhead_;
            } else {
                if (playhead_ >= len - 1)
                {
                    if (len == 1)
                        playhead_ = 0;
                    else
                        playhead_ = len - 2;
                    yoyo_sub_direction_ = DIRECTION_BACKWARD;
                } else 
                    ++playhead_;
            }
            break;
        } // switch
    } // else
    return playhead_;
}

unsigned int Clip::size() const
{
    int ret = static_cast<unsigned int>(images_.size());
    return ret;
}

/**
 * Returns NULL if there is no image at the given index.
 */
Image* Clip::get_image(unsigned int index) const
{
    // FIXME: will crash if no image at that index
    //if (images_.empty())
    //{
    //    std::cout << "ERROR: There is no image in the clip!"<< std::endl;
    //    return NULL;
    //}
    //else if (index > images_.size())
    //{
    //    std::cout << "ERROR: There is no image at index " << index << " in the clip! Total is " << images_.size() << "." << std::endl;
    //    return NULL;
    //}
    try 
    {
        Image *img = images_.at(index).get();
        return img;
    }
    catch (const std::out_of_range &e)
    {
        // Aug 25 2010:tmatth:FIXME we should actually prevent callers' 
        // logic from trying to get invalid framenumbers
        std::cerr << "Clip::get_image: Got exception " << e.what() << std::endl;
        return 0;
    }
}

void Clip::increase_playhead_fps()
{
    if (playhead_fps_ < MAX_FPS)
    {
        ++playhead_fps_;
        //TODO:2010-08-26:aalex:Do not print if not verbose
        std::cout << "Playback FPS: " << playhead_fps_ << std::endl;
    }
}

void Clip::decrease_playhead_fps()
{
    if (playhead_fps_ > 1)
    {
        -- playhead_fps_;
        //TODO:2010-08-26:aalex:Do not print if not verbose
        std::cout << "Playback FPS: " << playhead_fps_ << std::endl;
    }
}

// void Clip::lock_mutex()
// {
//     mutex_.lock();
// }
// 
// void Clip::unlock_mutex()
// {
//     mutex_.unlock();
// }

bool Clip::get_has_recorded_frame() const
{
    return has_recorded_a_frame_;
}

void Clip::set_has_recorded_frame()
{
    has_recorded_a_frame_ = true;
}

void Clip::clear_all_images()
{
    images_.clear();
    playhead_ = 0;
    writehead_ = 0;
}

