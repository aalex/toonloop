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

#include "clip.h"
#include "configuration.h"
#include "playheaditerator.h"
#include "image.h"
#include "timing.h"
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include <glib.h>
#include <iostream>
#include <string>
#include <tr1/memory>

//typedef std::vector< std::tr1::shared_ptr<Image> >::iterator ImageIterator;
namespace fs = boost::filesystem;
typedef std::map<std::string, std::tr1::shared_ptr<PlayheadIterator> >::iterator PlayheadIteratorIterator;

const std::string Clip::DEFAULT_FILE_EXTENSION = ".jpg";

Clip::Clip(unsigned int id) :
    verbose_(false)
{
    id_ = id;
    writehead_ = 0;
    playhead_ = 0;
    playhead_fps_ = 12; // some default for now
    has_recorded_a_frame_ = false;
    last_time_grabbed_image_ = timing::get_timestamp_now();
    intervalometer_rate_ = 10.0f; // 10 seconds is a reasonable default for a timelapse
    lower_bound_ = 0.0;
    upper_bound_ = 1.0;
    
    current_playhead_direction_ = std::string("invalid");
    init_playhead_iterators();
}

void Clip::init_playhead_iterators()
{
    PlayheadIterator *tmp = new ForwardIterator();
    current_playhead_direction_ = tmp->get_name();
    playhead_iterators_[tmp->get_name()] = std::tr1::shared_ptr<PlayheadIterator>(tmp);
    
    tmp = new BackwardIterator();
    playhead_iterators_[tmp->get_name()] = std::tr1::shared_ptr<PlayheadIterator>(tmp);
    
    tmp = new YoyoIterator();
    playhead_iterators_[tmp->get_name()] = std::tr1::shared_ptr<PlayheadIterator>(tmp);
    
    tmp = new RandomIterator();
    playhead_iterators_[tmp->get_name()] = std::tr1::shared_ptr<PlayheadIterator>(tmp);
    
    tmp = new DrunkIterator();
    playhead_iterators_[tmp->get_name()] = std::tr1::shared_ptr<PlayheadIterator>(tmp);
}

void Clip::set_intervalometer_rate(float rate)
{
    intervalometer_rate_ = rate;
}

void Clip::set_last_time_grabbed_image(long timestamp)
{
    last_time_grabbed_image_ = timestamp;
}

void Clip::set_directory_path(const std::string &directory_path)
{
    directory_path_ = directory_path;
}

// TODO: make sure the number of consecutive slashes in the path is ok
std::string Clip::get_image_full_path(Image* image) const
{
    std::string project_path = get_directory_path();
    std::string image_name = image->get_name();
    // TODO: use boost:file_system to append paths
    return project_path + "/" + IMAGES_DIRECTORY + "/" + image_name; 
}

unsigned int Clip::get_playhead_fps() const
{
    return playhead_fps_;
}

void Clip::set_playhead_fps(unsigned int fps)
{
    if (fps > MAX_FPS)
        fps = MAX_FPS;
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
// TODO:2010-09-02:aalex:Maybe Clip::frame_add should take the image file name as argument.
unsigned int Clip::frame_add()
{
    std::string name = timing::get_iso_datetime_for_now() + get_image_file_extension(); // .jpg
    return add_image(name);
}

unsigned int Clip::add_image(const std::string &name)
{
    using std::tr1::shared_ptr;
    if (writehead_ > size())
    {
        // Should not occur
        std::cout << "ERROR: The writehead points to a " <<
            "non-existing frame index " << writehead_ << " while the clip has only " << 
            images_.size() << " images." << std::endl;
        writehead_ = size();
        std::cout << "Set the writehead position to " << writehead_ << std::endl;
    }
    unsigned int assigned = writehead_;
    images_.insert(images_.begin() + writehead_, shared_ptr<Image>(new Image(name)));
    writehead_++;
    return assigned;
}

void Clip::set_writehead(unsigned int new_value)
{
    if (new_value > size())
        writehead_ = size();
    else
        writehead_ = new_value;
}

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
    else if (writehead_ == 0)
    {
        std::cout << "Cannot delete a frame since writehead is at 0" << std::endl;
    }
    else 
    {
        if (writehead_ > images_.size()) 
        {
            std::cout << "ERROR: The writehead points to a " <<
                "non-existing frame index " << writehead_ << " while the clip has only " << 
                images_.size() << " images." << std::endl;
            //Move the writehead to the end of clip and erase a frame anyways.
            writehead_ = size();
            std::cout << "Set the writehead position to " << writehead_ << std::endl;
        } 
        std::cout << "Deleting image at position " << (writehead_ - 1) << "/" << (images_.size()  - 1) << std::endl;
        unsigned int pos = writehead_ - 1;
        if (remove_deleted_images_)
        {
            remove_image_file(pos);
        }
        images_.erase(images_.begin() + (pos));
        how_many_deleted = 1;
        // let's decrement the writehead and playhead
        if (writehead_ > 0)
            --writehead_;
        make_sure_playhead_and_writehead_are_valid();
    }
    return how_many_deleted;
}

void Clip::make_sure_playhead_and_writehead_are_valid()
{
    if (playhead_ >= size())
    {
        if (size() == 0)
            playhead_ = 0;
        else
            playhead_ = size() - 1;
    }
    if (writehead_ > images_.size()) 
        writehead_ = size();
}

bool Clip::remove_last_image()
{
    if (size() > 0)
    {
        images_.erase(images_.end());
        // let's decrement the writehead and playhead
        if (writehead_ > 0 && writehead_ >= size())
            --writehead_;
        make_sure_playhead_and_writehead_are_valid();
        return true;
    } 
    else
        return false;
}

bool Clip::remove_first_if_more_than(int max_num)
{
    if (max_num != 0)
    {
        if (size() > (unsigned int) max_num)
        {
            remove_first_image();
            if (verbose_)
                std::cout << "Removing the first image! Max of " << max_num << " has been reached." << std::endl;
            return true;
        }
    }
    return false;
}

bool Clip::remove_first_image()
{
    if (size() > 0)
    {
        images_.erase(images_.begin());
        // let's decrement the writehead and playhead
        if (writehead_ > 0)
            --writehead_;
        make_sure_playhead_and_writehead_are_valid();
        return true;
    }
    else
        return false;
}

unsigned int Clip::get_playhead() const
{
    return playhead_;
}

unsigned int Clip::get_writehead() const
{
    return writehead_;
}

void Clip::get_effective_bounds(unsigned int &lower, unsigned int &upper)
{
    if (lower_bound_ == 0.0 && upper_bound_ == 1.0)
    {
        lower = 0;
        upper = size();
        return;
    }
    lower = (unsigned int) (size() * lower_bound_);
    upper = (unsigned int) (size() * upper_bound_);
    if (upper > size())
        upper = size();
}

unsigned int Clip::iterate_playhead()
{
    unsigned int len = size();
    if (len <= 1)
        playhead_ = 0;
    else 
    {
        unsigned int lower;
        unsigned int upper;
        get_effective_bounds(lower, upper);
        unsigned int max_num = upper - lower;
        playhead_ = playhead_iterators_[current_playhead_direction_].get()->iterate(playhead_, max_num) + lower;
    }
    return playhead_;
}

bool Clip::set_direction(const std::string &direction)
{
    PlayheadIteratorIterator iter;
    iter = playhead_iterators_.find(direction);
    if (iter != playhead_iterators_.end())
    {
        current_playhead_direction_ = direction;
        return true;
    }
    else
        return false;
}

unsigned int Clip::size() const
{
    return images_.size();
}

Image* Clip::get_image(unsigned int index) const
{
    try 
    {
        Image *img = images_.at(index).get();
        return img;
    }
    catch (const std::out_of_range &e)
    {
        // Aug 25 2010:tmatth:FIXME we should actually prevent callers' 
        // logic from trying to get invalid framenumbers
        std::cerr << "Clip::get_image(" << index << "): Got exception " << e.what() << std::endl;
        return 0;
    }
}

void Clip::increase_playhead_fps()
{
    if (playhead_fps_ < MAX_FPS)
    {
        ++playhead_fps_;
        if (verbose_)
            std::cout << "Playback FPS: " << playhead_fps_ << std::endl;
    }
}

void Clip::decrease_playhead_fps()
{
    if (playhead_fps_ > 1)
    {
        -- playhead_fps_;
        if (verbose_)
            std::cout << "Playback FPS: " << playhead_fps_ << std::endl;
    }
}

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
    if (remove_deleted_images_)
    {
        for (unsigned int i = 0; i < images_.size(); i++)
            remove_image_file(i);
    }
    images_.clear();
    playhead_ = 0;
    writehead_ = 0;
}

void Clip::set_remove_deleted_images(bool enabled)
{
    // TODO:2010-10-06:aalex:If that option is changed globally, change it for each clip.
    remove_deleted_images_ = enabled;
}

void Clip::remove_image_file(unsigned int index)
{
    Image* image = get_image(index);
    if (image == 0)
        std::cout << "Could not get a handle to any image!" << std::endl;
    else
    {
        std::string file_name = get_image_full_path(image);
        fs::path file_path = fs::path(file_name);
        if (fs::exists(file_path))
        {
            fs::remove(file_path);
            if (verbose_)
                std::cout << "Removed file " << file_name << std::endl;
        }
        else
            std::cout << "File " << file_name << " does not exist!" << std::endl;
    }
}

void Clip::change_direction()
{
    std::string current = get_direction();
    
    PlayheadIteratorIterator iter = playhead_iterators_.find(current);
    iter++;
    if (iter == playhead_iterators_.end())
        iter = playhead_iterators_.begin();
    set_direction((*iter).first);
}

void Clip::goto_beginning()
{
    playhead_ = 0;
}

void Clip::set_loop_bounds(double lower, double upper)
{
    if (lower <= 0.0)
        lower = 0.0;
    if (upper >= 1.0)
        upper = 1.0;
    if (lower > upper)
    {
        double tmp = lower;
        lower = upper;
        upper = tmp;
    }
    lower_bound_ = lower;
    upper_bound_ = upper;
}

