#ifndef __VIDEO_CONFIG_H__
#define __VIDEO_CONFIG_H__

#include <string>
#include <boost/program_options.hpp>

class VideoConfig
{
    public:
        VideoConfig(const boost::program_options::variables_map &options);
        int get_capture_fps() const { return capture_frame_rate_; }
        int get_rendering_fps() const { return rendering_frame_rate_; }
        int playheadFps() const { return playhead_fps_; }
        std::string videoSource() const { return video_source_; }
        std::string display() const { return display_; }
        bool fullscreen() const { return fullscreen_; }
    private:
        int capture_frame_rate_;
        int rendering_frame_rate_;
        int playhead_fps_;
        std::string video_source_;
        std::string display_;
        bool fullscreen_;
};
#endif // __VIDEO_CONFIG_H__

