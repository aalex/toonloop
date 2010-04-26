#include <string>
#include <boost/program_options.hpp>
#include "videoconfig.h"

VideoConfig::VideoConfig(const boost::program_options::variables_map &options)
{
    frame_rate_ = options["fps"].as<int>();
    video_source_ = options["video-source"].as<std::string>();
}

