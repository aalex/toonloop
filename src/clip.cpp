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
#include "image.h"
#include "timing.h"
#include <string>
#include <iostream>
#include <tr1/memory>

using namespace std::tr1; // shared_ptr

/**
 * A clip is a list of images.
 */
Clip::Clip(int id)
{
    // FIXME: How to use a 2-int vector?
    //intervalometer_rate_(1, 1); // default: 1 FPS
    //fps_(12, 1); // default: 12 FPS
    id_ = id;
    writehead_ = 0;
    playhead_ = 0;
    playhead_fps_ = 12; // some default for now
}

int Clip::get_playhead_fps()
{
    return playhead_fps_;
}
void Clip::set_playhead_fps(int fps)
{
    playhead_fps_ = fps;
}

int Clip::get_id()
{
    return id_;
}

int Clip::get_width()
{
    return width_;
}

int Clip::get_height()
{
    return height_;
}

void Clip::set_width(int width)
{
    width_ = width;
}

void Clip::set_height(int height)
{
    height_ = height;
}
/**
 * Adds an image to the clip.
 * Returns the its index.
 */
int Clip::frame_add()
{
    int assigned = writehead_;
    std::string name = timing::get_iso_datetime_for_now();
    images_.push_back(shared_ptr<Image>(new Image(name)));
    //images_[writehead_] = new Image(name);
    writehead_ ++;
    return assigned;
}

/**
 * Delete an image for the clip.
 * Returns how many images it has deleted. (0 or 1)
 */
// FIXME: this is not thread-safe, isn't it? (we must used shared_ptr)
int Clip::frame_remove()
{
    int how_many_deleted = 0;
    //int len = size();
    int len = writehead_;
    if (len > 0) // TODO: ! images_.empty()
    {
        //Image* image = images_[writehead_];
        images_.erase(images_.begin() + writehead_);
        // delete image;
        //delete images_[writehead_]; // FIXME: fine tune the image deletion algorithm
        writehead_ --; // FIXME
        how_many_deleted = 1;
    }
    return how_many_deleted;
}

int Clip::get_playhead()
{
    return playhead_;
}

int Clip::iterate_playhead()
{
    //int len = size();
    int len = writehead_;
    // TODO: implement BACK_AND_FORTH and BACKWARD directions
    if (len == 0)
    {
        playhead_ = 0;
    } else if (playhead_ >= len - 1) // >= ?
    {
        playhead_ = 0;
    } else {
        playhead_ ++;
    }
    return playhead_;
}

int Clip::size()
{
    int ret = static_cast<int>(images_.size());
    return ret;
}

/**
 * Returns NULL if there is no image at the given index.
 */
Image* Clip::get_image(int index)
{
    // FIXME: will crash if no image at that index
    int len = size();
    if (index > len)
    {
        std::cout << "ERROR: There is no image at index " << index << " in the clip! Total is " << len << "." << std::endl;
        return NULL;
    }
    return &(*images_[index]); // FIXME: is this OK?
}

