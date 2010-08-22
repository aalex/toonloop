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

#include <boost/thread/mutex.hpp>
#include <iostream>
#include <map>
#include <string>
#include <tr1/memory>
#include <tr1/unordered_map>
#include <vector>

#include "image.h"

using namespace std::tr1; // shared_ptr

const int MAX_FPS = 30;

enum direction 
{
    FORWARD, 
    BACKWARD,
    BACK_AND_FORTH
};

class Clip 
{
    public:
        Clip(int id);
        int get_id();
        //TODO: list<int>* get_all_images();
        int frame_add();
        int frame_remove();
        int iterate_playhead();
        int size();
        Image& get_image(int index);
        int get_playhead();
        void set_width(int width);
        void set_height(int height);
        int get_width();
        int get_height();
        int get_playhead_fps();
        void set_playhead_fps(int fps);
        void increase_playhead_fps();
        void decrease_playhead_fps();
        void lock_mutex();
        void unlock_mutex();
        void set_has_recorded_frame();
        bool get_has_recorded_frame();
        void set_directory_path(std::string directory_path);
        std::string get_directory_path() { return directory_path_; } 
        std::string get_image_file_extension() { return ".jpg"; };
        std::string get_image_full_path(Image* image);
    private:
        unsigned int id_;
        unsigned int playhead_;
        unsigned int writehead_;
        unsigned int width_;
        unsigned int height_;
        unsigned int nchannels_;
        direction direction_;
        //std::vector<int> intervalometer_rate_;
        //std::vector<int> fps_;
        // This is a list of images
        // I think we should use a std::list<std::tr1::shared_ptr<Image*>>
        // Their order can change.
        // Some of them may disappear.
        // the app is multithread!
        std::vector< shared_ptr<Image> > images_; // FIXME: use a list of shared_ptr to *Image
        int playhead_fps_;
        bool has_recorded_a_frame_;
        boost::mutex mutex_;
        std::string directory_path_;
        std::string file_extension_;
};

#endif // __CLIP_H__

