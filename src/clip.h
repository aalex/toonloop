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

// forward declaration
class Image;

const unsigned int MAX_FPS = 60;

enum direction 
{
    FORWARD, 
    BACKWARD,
    BACK_AND_FORTH
};

class Clip 
{
    public:
        Clip(unsigned int id);
        unsigned int get_id() const;
        //TODO: list<int>* get_all_images();
        unsigned int frame_add();
        unsigned int frame_remove();
        unsigned int iterate_playhead();
        unsigned int size() const;
        Image* get_image(unsigned int index) const;
        unsigned int get_playhead() const;
        unsigned int get_writehead() const;
        void set_width(unsigned int width);
        void set_height(unsigned int height);
        unsigned int get_width() const;
        unsigned int get_height() const;
        unsigned int get_playhead_fps() const;
        void set_playhead_fps(unsigned int fps);
        void increase_playhead_fps();
        void decrease_playhead_fps();
        void lock_mutex();
        void unlock_mutex();
        void set_has_recorded_frame();
        bool get_has_recorded_frame() const;
        void set_directory_path(const std::string &directory_path);
        std::string get_directory_path() const { return directory_path_; } 
        std::string get_image_file_extension() const { return ".jpg"; };
        std::string get_image_full_path(Image* image) const;
    private:
        unsigned int id_;
        unsigned int playhead_;
        unsigned int writehead_;
        unsigned int width_;
        unsigned int height_;
        unsigned int nchannels_;
        direction direction_;
        //std::vector<int> intervalometer_rate_;
        /**
         * This is a list of images
         * Their order can change.
         * Some of them may disappear.
         */
        std::vector< std::tr1::shared_ptr<Image> > images_;
        unsigned int playhead_fps_;
        bool has_recorded_a_frame_;
        boost::mutex mutex_;
        std::string directory_path_;
        std::string file_extension_;
};

#endif // __CLIP_H__

