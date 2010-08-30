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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <string>
#include "configuration.h"
#include "image.h"

class Application;
/** GStreamer video pipeline for grabbing images from a camera.
 */
class Pipeline
{
    public:
        void stop();
        Pipeline(Application* owner);
        ~Pipeline();
        void grab_frame();
        void remove_frame();
    private:
        Application* owner_;
        GstElement* videosrc_;
        GstElement* videosink_;
        GstElement* gdkpixbufsink_;
        GstPipeline* pipeline_;
        GstState state_;
        static void end_stream_cb(GstBus* bus, GstMessage* msg, GstElement* pipeline);
        static void on_new_live_pixbuf(GstBus* bus, GstMessage* message, GstElement* pipeline);
        std::string guess_source_caps(unsigned int framerateIndex) const;
        void save_image_to_current_clip(GdkPixbuf *pixbuf);
};

#endif // __PIPELINE_H__
