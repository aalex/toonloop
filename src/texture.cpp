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

#include <GL/gl.h>
#include "texture.h"

/**
 * Textures are images that are displayed in the OpenGL rendering context. 
 * 
 * We load the texture data using a GdkPixBuf. 
 */
Texture::Texture()
{
    has_new_data_ = false;
    has_some_data_ = false;
}

int Texture::allocate_image(int bufsize)
{
    rawdata_ = new char[bufsize];
}

void Texture::free_image()
{
    delete rawdata_;
}

GLuint Texture::get_number()
{
    return number_;
}

void Texture::set_number(GLuint number)
{
    number_ = number;
}

char* Texture::get_rawdata()
{
    return rawdata_;
}

