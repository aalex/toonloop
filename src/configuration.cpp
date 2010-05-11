#include <string>
#include <boost/program_options.hpp>
#include "configuration.h"

Configuration::Configuration(const boost::program_options::variables_map &options)
{
    //capture_frame_rate_ = options["capture-fps"].as<int>();
    //rendering_frame_rate_ = capture_frame_rate_; //options["rendering-fps"].as<int>();
    //playhead_fps_ = options["playhead-fps"].as<int>();
    video_source_ = options["video-source"].as<std::string>();
    display_ = options["display"].as<std::string>();
    fullscreen_ = options["fullscreen"].as<bool>();
}

void Configuration::set_project_home(std::string project_home)
{
    project_home_ = project_home;
}

