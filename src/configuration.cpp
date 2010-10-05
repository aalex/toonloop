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

/**
 * Contains the runtime configuration options for Toonloop.
 *
 * Some of these values might change while the program is running.
 */
Configuration::Configuration(const boost::program_options::variables_map &options) :
    playhead_fps_(0),
    video_source_(""),
    display_(options["display"].as<std::string>()),
    fullscreen_(options["fullscreen"].as<bool>()),
    enable_effects_(false),
    verbose_(options["verbose"].as<bool>()),
    mouse_controls_enabled_(options["enable-mouse-controls"].as<bool>()),
    midi_input_number_(options.count("midi-input") ? options["midi-input"].as<int>() : MIDI_INPUT_NONE),
    osc_recv_port_(options.count("osc-receive-port") ? options["osc-receive-port"].as<std::string>() : OSC_PORT_NONE),
    osc_send_port_(options.count("osc-send-port") ? options["osc-send-port"].as<std::string>() : OSC_PORT_NONE),
    osc_send_addr_(options["osc-send-addr"].as<std::string>())
{
    //enable_effects_ = options["enable-effects"].as<bool>();
    //capture_frame_rate_ = options["capture-fps"].as<int>();
    //rendering_frame_rate_ = capture_frame_rate_; //options["rendering-fps"].as<int>();
    //playhead_fps_ = options["playhead-fps"].as<int>();
    // video_source_ = options["video-source"].as<std::string>();
    if (midi_input_number_ != MIDI_INPUT_NONE) // Means disabled
        std::cout << "Using MIDI input " << midi_input_number_ << std::endl;
    capture_width_ = options["width"].as<int>();
    capture_height_ = options["height"].as<int>();
    max_images_per_clip_ = options["max-images-per-clip"].as<int>();
}

void Configuration::set_project_home(const std::string &project_home)
{
    project_home_ = project_home;
}

void Configuration::set_video_source(const std::string &video_source)
{
    video_source_ = video_source;
}

