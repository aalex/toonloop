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

/**
 * Configuration for Toonloop.
 */

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <string>
#include <boost/program_options.hpp>

/**
 * Integer value that means that no MIDI input has been selected.
 */
const int MIDI_INPUT_NONE = 99;
/**
 * Name of the directory to store images in the project directory.
 */
const std::string IMAGES_DIRECTORY = "images";
/**
 * Name of the directory to store movies in the project directory.
 */
const std::string MOVIES_DIRECTORY = "movies";
// /**
//  * Name of the directory to store data in the project directory.
//  */
// const std::string DATA_DIRECTORY = "data";
/**
 * Default project directory.
 */
const std::string DEFAULT_PROJECT_HOME = "~/Documents/toonloop/default";

const std::string OSC_PORT_NONE = "";
/** Contains the configuration options for Toonloop.
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
        bool get_effects_enabled() const { return enable_effects_; }
        void set_effects_enabled(bool enabled) { enable_effects_ = enabled; }
        void set_project_home(const std::string &project_home);
        void set_video_source(const std::string &video_source);
        bool get_verbose() const { return verbose_; } ;
        bool get_fullscreen() const { return fullscreen_; } ;
        int get_midi_input_number() const { return midi_input_number_; }
        std::string get_osc_recv_port() const { return osc_recv_port_; }
        std::string get_osc_send_port() const { return osc_send_port_; }
        std::string get_osc_send_addr() const { return osc_send_addr_; }
        bool get_mouse_controls_enabled() const { return mouse_controls_enabled_; }
        int get_capture_width() const { return capture_width_; }
        int get_capture_height() const { return capture_height_; }
        int get_max_images_per_clip() const { return max_images_per_clip_; }
        int get_default_intervalometer_rate() const { return default_intervalometer_rate_; }
    private:
        //int capture_frame_rate_;
        //int rendering_frame_rate_;
        int playhead_fps_;
        std::string video_source_;
        std::string display_;
        std::string project_home_;
        bool fullscreen_;
        bool enable_effects_;
        bool verbose_;
        bool mouse_controls_enabled_;
        int midi_input_number_;
        std::string osc_recv_port_;
        std::string osc_send_port_;
        std::string osc_send_addr_;
        int capture_width_;
        int capture_height_;
        int max_images_per_clip_;
        int default_intervalometer_rate_;
};
#endif // __CONFIGURATION_H__

