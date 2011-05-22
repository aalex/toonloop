/*
 * Toonloop
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
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
        void save_image_to_current_clip(GdkPixbuf *pixbuf);
        GstElement* gdkpixbufsink_;
        void set_record_all_frames(bool enable);
        bool get_record_all_frames() const { return record_all_frames_enabled_; }
        void set_intervalometer_is_on(bool enable);
        bool get_intervalometer_is_on() const { return intervalometer_is_on_; }
        bool import_image(const std::string &file_name);
    private:
        Application* owner_;
        GstElement* videosrc_;
        GstElement* videosink_;
        GstPipeline* pipeline_;
        GstState state_;
        bool record_all_frames_enabled_;
        bool intervalometer_is_on_;
        static void end_stream_cb(GstBus* bus, GstMessage* msg, gpointer user_data);
        static void on_new_live_pixbuf(GstBus* bus, GstMessage* message, gpointer user_data);
        std::string guess_source_caps(unsigned int framerateIndex) const;
        static void bus_message_cb(GstBus *bus, GstMessage *msg,  gpointer user_data);
        void link_or_die(GstElement *from, GstElement *to);
        static void cb_new_dvdemux_src_pad(GstElement *srcElement, GstPad *srcPad, gpointer data);
};

#endif // __PIPELINE_H__
