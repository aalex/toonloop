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


#ifndef __VIDEO_CONFIG_H__
#define __VIDEO_CONFIG_H__

#include <string>
#include <boost/program_options.hpp>
/**
 * Contains the configuration options for the whole application.
 */
class Configuration
{
    public:
        /**
         * A lot of configuration options are set in the constructor of the Configuration class.
         */
        Configuration(const boost::program_options::variables_map &options);
        //int get_capture_fps() const { return capture_frame_rate_; }
        //int get_rendering_fps() const { return rendering_frame_rate_; }
        int playheadFps() const { return playhead_fps_; }
        std::string videoSource() const { return video_source_; }
        std::string display() const { return display_; }
        std::string get_project_home() const { return project_home_; }
        bool fullscreen() const { return fullscreen_; }
        bool get_images_in_ram() const { return images_in_ram_; }
        bool get_effects_enabled() const { return enable_effects_; }
        void set_effects_enabled(bool enabled) { enable_effects_ = enabled; }
        void set_project_home(std::string project_home);
        void set_video_source(std::string video_source);
        bool get_verbose() const { return verbose_; } ;
        bool get_fullscreen() const { return fullscreen_; } ;
        int get_midi_input_number() const { return midi_input_number_; }
    private:
        //int capture_frame_rate_;
        //int rendering_frame_rate_;
        int playhead_fps_;
        std::string video_source_;
        std::string display_;
        std::string project_home_;
        bool fullscreen_;
        bool enable_effects_;
        bool images_in_ram_;
        bool verbose_;
        int midi_input_number_;
};
#endif // __VIDEO_CONFIG_H__

