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

#ifndef __CLIP_H__
#define __CLIP_H__

#include <iostream>
#include <map>
#include <string>
#include <tr1/memory>
#include <tr1/unordered_map>
#include <vector>
#include "playheaditerator.h"

// forward declarations
class Image;

enum clip_direction 
{
    DIRECTION_FORWARD, 
    DIRECTION_BACKWARD,
    DIRECTION_YOYO,
    DIRECTION_RANDOM,
    DIRECTION_DRUNK
};

/** The Clip class contains a list of image paths */
class Clip 
{
    public:
        Clip(unsigned int id);
        static const unsigned int MAX_FPS = 60;
        unsigned int get_id() const;
        //TODO: list<int>* get_all_images();
        unsigned int frame_add();
        unsigned int frame_remove();
        unsigned int iterate_playhead();
        unsigned int size() const;
        Image* get_image(unsigned int index) const;
        unsigned int get_playhead() const;
        unsigned int get_writehead() const;
        void set_writehead(unsigned int new_value);
        void set_width(unsigned int width);
        void set_height(unsigned int height);
        bool set_direction(const std::string direction);
        const std::string &get_direction() { return current_playhead_direction_; }
        unsigned int get_width() const;
        unsigned int get_height() const;
        unsigned int get_playhead_fps() const;
        void set_playhead_fps(unsigned int fps);
        void increase_playhead_fps();
        void decrease_playhead_fps();
        void set_has_recorded_frame();
        bool get_has_recorded_frame() const;
        void set_directory_path(const std::string &directory_path);
        std::string get_directory_path() const { return directory_path_; } 
        std::string get_image_file_extension() const { return ".jpg"; };
        std::string get_image_full_path(Image* image) const;
        void clear_all_images();
        long get_last_time_grabbed_image() const { return last_time_grabbed_image_; }
        void set_last_time_grabbed_image(const long timestamp);
        /** The intervalometer speed is in seconds */
        float get_intervalometer_rate() const { return intervalometer_rate_; }
        void set_intervalometer_rate(const float rate);
        bool remove_last_image();
        bool remove_first_image();
        void set_remove_deleted_images(bool enabled);
        void change_direction();
    private:
        unsigned int id_;
        unsigned int playhead_;
        unsigned int writehead_;
        unsigned int width_;
        unsigned int height_;
        unsigned int nchannels_;
        float intervalometer_rate_;
        clip_direction direction_;
        clip_direction yoyo_sub_direction_;
        //std::vector<int> intervalometer_rate_;
        /**
         * This is a list of images
         * Their order can change.
         * Some of them may disappear.
         */
        std::vector< std::tr1::shared_ptr<Image> > images_;
        unsigned int playhead_fps_;
        bool has_recorded_a_frame_;
        std::string directory_path_;
        std::string file_extension_;
        /** Used to store time stamp of when grabbed last image.
         *
         * Useful for either the intervalometer, or the video grabbing.
         */
        long last_time_grabbed_image_;
        void make_sure_playhead_and_writehead_are_valid();
        bool remove_deleted_images_;
        void remove_image_file(unsigned int index);
        std::map< std::string, std::tr1::shared_ptr<PlayheadIterator> > playhead_iterators_;
        std::string current_playhead_direction_;
        void init_playhead_iterators();
};

#endif // __CLIP_H__

