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

/** The Clip class contains a list of image paths */
class Clip 
{
    public:
        /** 
         * Each clip needs a unique ID.
         */
        Clip(unsigned int id);
        static const unsigned int MAX_FPS = 60;
        unsigned int get_id() const;
        //TODO: list<int>* get_all_images();
        /**
         * Adds an image to the clip.
         * Once its added, the one who called this should fill that Image object with
         * its file name.
         * @return The index of the new image
         */
        unsigned int frame_add();
        /**
         * Delete an image for the clip.
         * @return How many images it has deleted. (0 or 1)
         */
        unsigned int frame_remove();
        unsigned int iterate_playhead();
        unsigned int size() const;
        /**
         * Returns the given image.
         * @return A valid Image pointer, or a null pointer if there is no image at the given index.
         */
        Image* get_image(unsigned int index) const;
        unsigned int get_playhead() const;
        unsigned int get_writehead() const;
        /**
         * Sets the write head position
         */
        void set_writehead(unsigned int new_value);
        /**
         * Only the Pipeline should call this.
         */
        void set_width(unsigned int width);
        /**
         * Only the Pipeline should call this.
         */
        void set_height(unsigned int height);
        bool set_direction(const std::string &direction);
        const std::string &get_direction() { return current_playhead_direction_; }
        /**
         * Useful to know the size of the movie clip to convert.
         */
        unsigned int get_width() const;
        /**
         * Useful to know the size of the movie clip to convert.
         */
        unsigned int get_height() const;
        unsigned int get_playhead_fps() const;
        void set_playhead_fps(unsigned int fps);
        void increase_playhead_fps();
        void decrease_playhead_fps();
        void set_has_recorded_frame();
        bool get_has_recorded_frame() const;
        void set_directory_path(const std::string &directory_path);
        std::string get_directory_path() const { return directory_path_; } 
        std::string get_image_full_path(Image* image) const;
        void clear_all_images();
        long get_last_time_grabbed_image() const { return last_time_grabbed_image_; }
        void set_last_time_grabbed_image(long timestamp);
        /** The intervalometer speed is in seconds */
        float get_intervalometer_rate() const { return intervalometer_rate_; }
        void set_intervalometer_rate(float rate);
        bool remove_last_image();
        bool remove_first_image();
        void set_remove_deleted_images(bool enabled);
        /**
         * Chooses the next playhead direction.
         */
        void change_direction();
        /**
         * Used internally by frame_add, but also when loading a project.
         * Return Its index.
         */
        unsigned int add_image(const std::string &name);
        void set_verbose(bool verbose) { verbose_ = verbose; }

        /**
         * Removes the first image if the maximum number of frames has been reached.
         * Does not remove any if max_num is 0
         */
        bool remove_first_if_more_than(int max_num);
        void goto_beginning();
        /**
         * Set relative loop bounds.
         * @param lower: From 0.0 to 1.0.
         * @param upper: From 0.0 to 1.0.
         */
        void set_loop_bounds(double lower=0.0, double upper=1.0);
        void get_loop_bounds(double &lower, double &upper) const
        {
            lower = lower_bound_;
            upper = upper_bound_;
        }
    private:
        bool verbose_;
        unsigned int id_;
        unsigned int playhead_;
        unsigned int writehead_;
        unsigned int width_;
        unsigned int height_;
        unsigned int nchannels_;
        float intervalometer_rate_;
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
        /**
         * Populates the map of playhead iterator objects.
         */
        void init_playhead_iterators();
        static const std::string DEFAULT_FILE_EXTENSION;
        const std::string &get_image_file_extension() const { return DEFAULT_FILE_EXTENSION; };
        double lower_bound_;
        double upper_bound_;
        void get_effective_bounds(unsigned int &lower, unsigned int &upper);
};

#endif // __CLIP_H__

