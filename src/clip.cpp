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

Image::Image(int number)
{
    number_ = number;
}

int Image::allocate_image(int bufsize)
{
    rawdata_ = new char[bufsize];
}

int Image::get_number()
{
    return number_;
}

char* Image::get_rawdata()
{
    return rawdata_;
}

Clip::Clip(int id)
{
    // FIXME: How to use a 2-int vector?
    //intervalometer_rate_(1, 1); // default: 1 FPS
    //fps_(12, 1); // default: 12 FPS
    id_ = id;
    writehead_ = 0;
    playhead_ = 0;
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

int Clip::frame_add()
{
    int assigned = writehead_;
    images_[writehead_] = new Image(number_allocator_);
    number_allocator_ ++;
    writehead_ ++;
    return assigned;
}

/**
 * Delete an image for the clip.
 * Returns how many images it has deleted. (0 or 1)
 */
int Clip::frame_remove()
{
    int how_many_deleted = 0;
    if (writehead_ > 0)
    {
        delete images_[writehead_]; // FIXME: fine tune the image deletion algorithm
        writehead_ --;
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
    int len = (int) images_.size();
    // TODO: implement BACK_AND_FORTH and BACKWARD directions
    if (playhead_ >= len)
    {
        playhead_ = 0;
    } else {
        playhead_ ++;
    }
    return playhead_;
}

int Clip::size()
{
    return (int) images_.size();
}

Image* Clip::get_image(int index)
{
    // FIXME: will crash if no image at that index
    return images_[index];
}
