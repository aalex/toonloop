#ifndef __VIDEO_CONFIG_H__
#define __VIDEO_CONFIG_H__

#include <string>
#include <boost/program_options.hpp>

class VideoConfig
{
    public:
        VideoConfig(const boost::program_options::variables_map &options);
        int frameRate() const { return frame_rate_; }
        std::string videoSource() const { return video_source_; }
        std::string display() const { return display_; }
        bool fullscreen() const { return fullscreen_; }
    private:
        int frame_rate_;
        std::string video_source_;
        std::string display_;
        bool fullscreen_;
};
#endif // __VIDEO_CONFIG_H__

