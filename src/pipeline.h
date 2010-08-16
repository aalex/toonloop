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
#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <string>

#include "configuration.h"
#include "image.h"
#include "texture.h"

class Pipeline
{
    public:
        void stop();
        Pipeline();
        ~Pipeline();
        void grab_frame();
        void remove_frame();
        //Texture playback_texture_;
        //Texture onionskin_texture_;
        std::string get_image_full_path(Image* image);
    private:
        GstElement* videosrc_;
        GstElement* videosink_;
        GstElement* gdkpixbufsink_;
        GstPipeline* pipeline_;
        GstState state_;
        static void end_stream_cb(GstBus* bus, GstMessage* msg, GstElement* pipeline);
        std::string guess_source_caps(unsigned int framerateIndex) const;
};

#endif // __PIPELINE_H__
