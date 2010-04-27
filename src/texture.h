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
#ifndef __TEXTURE_H__
#define __TEXTURE_H__
#include <GL/gl.h>

class Texture 
{
    private:
        char *rawdata_;
    public: 
        GLuint number_;
        Texture();
        void set_number(GLuint number);
        GLuint get_number();
        bool has_new_data_;
        bool has_some_data_;
        char* get_rawdata();
        int allocate_image(int bufsize);
        void free_image();
};

#endif // __TEXTURE_H__


