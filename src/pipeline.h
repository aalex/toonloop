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

#include <GL/glew.h> // Must include it before GL/gl.h
#include <GL/glx.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <string>

#include "configuration.h"
#include "image.h"
#include "shader.h"
#include "texture.h"

class Pipeline
{
    public:
        void stop();
        //void set_drawing_area(GtkWidget *drawing_area);
        Pipeline(); // const VideoConfig &config);
        ~Pipeline();
        void grab_frame();
        void remove_frame();
        Texture playback_texture_;
        Texture onionskin_texture_;
        Shader* get_shader();
        std::string get_image_full_path(Image* image);
        void set_shader(Shader* shader);
    private:
        GstElement* videosrc_;
        GstElement* videosink_;
        GstElement* gdkpixbufsink_;
        GstPipeline* pipeline_;
        GstState state_;
        static void end_stream_cb(GstBus* bus, GstMessage* msg, GstElement* pipeline);
        std::string guess_source_caps(unsigned int framerateIndex) const;
        Shader* shader_;
};

//static GstBusSyncReply create_window(GstBus* bus, GstMessage* message, GtkWidget* widget);
//static gboolean on_expose_event(GtkWidget* widget, GdkEventExpose* event, GstElement* videosink);
//void reshapeCallback(GLuint width, GLuint height, gpointer data);
//gboolean drawCallback(GLuint texture, GLuint width, GLuint height, gpointer data);
//bool check_if_shaders_are_supported();

#endif // __PIPELINE_H__
