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

#include "image.h"
#include <string>

/** 
 * This Image class is just a container for a single integer value.
 * We use this number to determine the file name for the image saved as JPEG.
 */
Image::Image(std::string name)
{
    name_ = name;
    ready_ = false;
    rawdata_ = new char[0];
}

int Image::allocate_image(int bufsize)
{
    delete rawdata_;
    rawdata_ = new char[bufsize];
}

Image::~Image()
{
    delete rawdata_;
}

std::string Image::get_name()
{
    return name_;
}

char* Image::get_rawdata()
{
    return rawdata_;
}

/**
 * Whether the image data has been fully loaded/saved or not.
 */
bool Image::is_ready()
{
    return ready_;
}

void Image::set_ready(bool ready)
{
    ready_ = ready;
}

