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
#include <string>
#include <iostream>
#include <boost/program_options.hpp>
#include "configuration.h"

Configuration::Configuration(const boost::program_options::variables_map &options)
{
    //capture_frame_rate_ = options["capture-fps"].as<int>();
    //rendering_frame_rate_ = capture_frame_rate_; //options["rendering-fps"].as<int>();
    //playhead_fps_ = options["playhead-fps"].as<int>();
    // video_source_ = options["video-source"].as<std::string>();
    display_ = options["display"].as<std::string>();
    fullscreen_ = options["fullscreen"].as<bool>();
    verbose_ = options["verbose"].as<bool>();
    // TODO:2010-08-16:aalex:Either discard keep-image-in-ram option or make it possible again.
    //images_in_ram_ = options["keep-images-in-ram"].as<bool>();
    images_in_ram_ = false;
    //enable_effects_ = options["enable-effects"].as<bool>();
    enable_effects_ = false;
    if (images_in_ram_)
        std::cout << "Images will be kept into RAM and not loaded from the disk on every frame." << std::endl;
    if (options.count("midi-input"))
    {
        midi_input_number_ = options["midi-input"].as<int>();
        std::cout << "Using MIDI input " << midi_input_number_ << std::endl;
    } else {
        midi_input_number_ = MIDI_INPUT_NONE; // Means disabled;
    }
}

void Configuration::set_project_home(std::string project_home)
{
    project_home_ = project_home;
}

void Configuration::set_video_source(std::string video_source)
{
    video_source_ = video_source;
}

