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
#ifndef __CLIP_H__
#define __CLIP_H__

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <tr1/unordered_map>

enum direction 
{
    FORWARD, 
    BACKWARD,
    BACK_AND_FORTH
};

class Image 
{
    private:
        int number_;
    public: 
        Image(int number);
        int get_number();
};

class Clip 
{
    private:
        int id_;
        int playhead_;
        int writehead_;
        direction direction_;
        std::vector<int> intervalometer_rate_;
        std::vector<int> fps_;
        std::tr1::unordered_map<int, Image*> images_;
        int number_allocator_; // forever growing unique number TODO: use long int?
    public:
        Clip(int id);
        int get_id();
        int frame_add();
        int frame_remove();
        int iterate_playhead();
        int size();
        Image* get_image(int index);
};

#endif // __CLIP_H__

