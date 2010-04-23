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
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <gst/gst.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <iostream>
#include <cstring>
#include "gstgtk.h"
#include "pipeline.h"
#include "draw.h"
#include "application.h"
#include "gui.h"

/**
 * GST bus signal watch callback 
 */
void Pipeline::end_stream_cb(GstBus* bus, GstMessage* message, GstElement* pipeline)
{
    bool stop_it = true;
    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR)
    {
        gchar *debug = NULL;
        GError *err = NULL;
        gst_message_parse_error(message, &err, &debug);
        g_print("Error: %s\n", err->message);
        g_error_free(err);
        if (debug) 
        {
            g_print("Debug details: %s\n", debug);
            g_free(debug);
        }
    } else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
        std::cout << "EOS: End of stream" << std::endl;
    } else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_WARNING) {
        gchar *debug = NULL;
        GError *err = NULL;
        gst_message_parse_warning(message, &err, &debug);
        g_print("Warning: %s\n", err->message);
        g_error_free(err);
        if (debug) 
        {
            g_print("Debug details: %s\n", debug);
            g_free(debug);
        }
        stop_it = false;
    }
    if (stop_it)
    {
        g_print("Stopping the stream and quitting.\n");
        //gst_element_set_state(pipeline, GST_STATE_NULL);
        //gst_object_unref(pipeline);
        // TODO: stop()
        Application::get_instance().quit();
        //gtk_main_quit();
    }
}

void Pipeline::grab_frame()
{
    static int frameid = 0;
    GdkPixbuf* pixbuf;
    // TODO: replace constants by const attributes
    // TODO: use C++ strings, not C-style. :)
    char filename[TOON_MAX_FILENAME_LENGTH];
    char temp[8]; 
    g_object_get(G_OBJECT(gdkpixbufsink_), "last-pixbuf", &pixbuf, NULL);
    int w = gdk_pixbuf_get_width(pixbuf);
    int h = gdk_pixbuf_get_height(pixbuf);

    strcpy(filename, pixfileprefix);
    sprintf(temp, "%d.jpg", frameid);
    strcat(filename, temp);
    if (!gdk_pixbuf_save(pixbuf, filename , "jpeg", NULL, "quality", "100", NULL))
    {
        g_print("Image %s could not be saved. Error\n", filename);
        // TODO : print error message.
    }
    else
        g_print("Image %s saved\n", filename);
    frameid++;
    g_object_unref(pixbuf);
}

void Pipeline::stop()
{
    std::cout << "Stopping the Gstreamer pipeline." << std::endl;
    gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_NULL);
    gst_object_unref(pipeline_);
    // gtk main quit?
}
Pipeline::Pipeline()
{
    pipeline_ = NULL;
    
    pipeline_ = GST_PIPELINE(gst_pipeline_new("pipeline"));
    //state_ = 0;
    GstElement* glupload0;
    
    // Video source element
    //videosrc_  = gst_element_factory_make("videotestsrc", "videosrc0");
    videosrc_  = gst_element_factory_make("v4l2src", "videosrc0"); // TODO: add more like in Ekiga
    g_assert(videosrc_); // TODO: use something else than g_assert to see if we could create the elements.
    // capsfilter element #0
        // Possible values for the capture FPS:
        // 25/1 
        // 30000/1001
        // 30/1
    GstElement* capsfilter0 = gst_element_factory_make ("capsfilter", NULL);
    // capsfilter0, for the capture FPS and size
    GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb",
                                        "width", G_TYPE_INT, 640, // TODO: make configurable!
                                        "height", G_TYPE_INT, 480,
                                        //"framerate", GST_TYPE_FRACTION, 30000, 1001,
                                        NULL); 
    g_object_set(capsfilter0, "caps", caps, NULL);
    gst_caps_unref(caps);
    // ffmpegcolorspace0 element
    GstElement* ffmpegcolorspace0 = gst_element_factory_make("ffmpegcolorspace", "ffmpegcolorspace0");
    g_assert(ffmpegcolorspace0);
    GstElement* tee0 = gst_element_factory_make("tee", "tee0");
    g_assert(tee0);
    GstElement* queue0 = gst_element_factory_make("queue", "queue0");
    g_assert(queue0);

    // glupload element
    glupload0  = gst_element_factory_make ("glupload", "glupload0");
    // glimagesink
    videosink_ = gst_element_factory_make("glimagesink", "glimagesink0");
    g_object_set(videosink_, "sync", FALSE, NULL);
    g_object_set(G_OBJECT(videosink_), "client-reshape-callback", reshapeCallback, NULL);
    g_object_set(G_OBJECT(videosink_), "client-draw-callback", drawCallback, NULL);
    // capsfilter element #1, for the OpenGL FPS and size
    GstElement* capsfilter1 = gst_element_factory_make ("capsfilter", NULL);
    GstCaps *outcaps = gst_caps_new_simple("video/x-raw-gl",
                                        "width", G_TYPE_INT, 800,
                                        "height", G_TYPE_INT, 600,
                                        NULL) ;
    g_object_set(capsfilter1, "caps", outcaps, NULL);

    // GdkPixbuf sink:
    GstElement* queue1 = gst_element_factory_make("queue", "queue1");
    g_assert(queue1);
    //FIXME: GstElement* 
    gdkpixbufsink_ = gst_element_factory_make("gdkpixbufsink", "gdkpixbufsink0");
    g_assert(gdkpixbufsink_);
    pixfileprefix = TOON_DEFAULT_FILENAME;
    gst_caps_unref(outcaps);

    // add elements

    // add elements
    gst_bin_add(GST_BIN(pipeline_), videosrc_); // capture
    gst_bin_add(GST_BIN(pipeline_), capsfilter0);
    gst_bin_add(GST_BIN(pipeline_), ffmpegcolorspace0);
    gst_bin_add(GST_BIN(pipeline_), tee0);
    gst_bin_add(GST_BIN(pipeline_), queue0); // branch #0: videosink
    gst_bin_add(GST_BIN(pipeline_), glupload0);
    gst_bin_add(GST_BIN(pipeline_), capsfilter1);
    gst_bin_add(GST_BIN(pipeline_), videosink_);
    gst_bin_add(GST_BIN(pipeline_), queue1); // branch #1: gdkpixbufsink
    gst_bin_add(GST_BIN(pipeline_), gdkpixbufsink_);

    // link pads:
    gboolean is_linked = NULL;
    is_linked = gst_element_link_pads(videosrc_, "src", capsfilter0, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "videosrc0", "capsfilter0"); exit(1); }
    is_linked = gst_element_link_pads(capsfilter0, "src", ffmpegcolorspace0, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "capsfilter0", "ffmpegcolorspace0"); exit(1); }
    is_linked = gst_element_link_pads(ffmpegcolorspace0, "src", tee0, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "ffmpegcolorspace0", "tee0"); exit(1); }
    is_linked = gst_element_link_pads(tee0, "src0", queue0, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "tee0", "queue0"); exit(1); }
    // output 0: the OpenGL uploader.
    is_linked = gst_element_link_pads(queue0, "src", glupload0, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "queue0", "glupload0"); exit(1); }
    is_linked = gst_element_link_pads(glupload0, "src", capsfilter1, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "glupload0", "capsfilter1"); exit(1); }
    is_linked = gst_element_link_pads(capsfilter1, "src", videosink_, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "capsfilter1", "fakesink0"); exit(1); }

    // output 1: the GdkPixbuf sink
    is_linked = gst_element_link_pads(tee0, "src1", queue1, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "tee0", "queue1"); exit(1); }
    is_linked = gst_element_link_pads(queue1, "src", gdkpixbufsink_, "sink");
    if (!is_linked) { g_print("Could not link %s to %s.\n", "queue1", "gdkpixbufsink0"); exit(1); }

    /* setup bus */
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), this);
    gst_object_unref(bus);

    //TODO: 
    char* device_name = "/dev/video0";
    g_print("Using camera %s.\n", device_name);
    g_object_set(videosrc_, "device", device_name, NULL); 

    // make it play !!

    /* run */
    GstStateChangeReturn ret;
    ret = gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_print ("Failed to start up pipeline!\n");
        /* check if there is an error message with details on the bus */
        GstMessage* msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
        if (msg)
        {
          GError *err = NULL;
          gst_message_parse_error (msg, &err, NULL);
          g_print ("ERROR: %s\n", err->message);
          g_error_free (err);
          gst_message_unref (msg);
        }
        exit(1);
    }
}
//client reshape callback
void reshapeCallback (GLuint width, GLuint height, gpointer data)
{
    glViewport(0, 0, width, height);
    
    float w = float(width) / float(height);
    float h = 1.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w, w, -h, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glEnable (GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //glEnable (GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

//client draw callback
gboolean drawCallback (GLuint texture, GLuint width, GLuint height, gpointer data)
{
    static GLfloat  zrot = 0;
    static GTimeVal current_time;
    static glong last_sec = current_time.tv_sec;
    static gint nbFrames = 0;

    g_get_current_time (&current_time);
    nbFrames++ ;

    if ((current_time.tv_sec - last_sec) >= 1)
    {
        std::cout << "GRAPHIC FPS = " << nbFrames << std::endl;
        nbFrames = 0;
        last_sec = current_time.tv_sec;
    }

    glEnable (GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
    // TODO: simplify those parameters
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glTranslatef(0.0f,0.0f,0.0f);

    glRotatef(zrot,0.0f,0.0f,1.0f);

    glScalef(0.666f, 0.5f, 1.0f);
    draw::draw_vertically_flipped_textured_square(width, height);
    glPopMatrix();

    zrot+=0.001f;

    // DRAW LINES
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glColor4f(0.2, 0.2, 0.2, 0.2);
    int num = 64;
    float x;
    
    for (int i = 0; i < num; i++)
    {
        x = (i / float(num)) * 4 - 2;
        draw::draw_line(float(x), -2.0, float(x), 2.0);
        draw::draw_line(-2.0, float(x), 2.0, float(x));
    }
    
    //return TRUE causes a postRedisplay
    return FALSE;
}

// sets the x-window-id 
// Important !
static GstBusSyncReply create_window(GstBus* bus, GstMessage* message, GtkWidget* widget)
{
    GstXOverlay *xoverlay;
    gulong xwindow_id;
    // ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;
    if (!gst_structure_has_name(message->structure, "prepare-xwindow-id"))
        return GST_BUS_PASS;
    g_print("setting xwindow id\n");
    xoverlay = GST_X_OVERLAY(GST_MESSAGE_SRC(message));
    xwindow_id = Application::get_instance().get_gui().video_xwindow_id_;
    if (xwindow_id != 0)
    {
        gst_x_overlay_set_xwindow_id(xoverlay, xwindow_id);
    } else {
        g_warning ("Should have obtained video_xwindow id by now! X-related crash may occur");
        gst_x_overlay_set_xwindow_id (xoverlay, 0);
    }
    gst_message_unref(message);
    return GST_BUS_DROP;
}

void Pipeline::set_drawing_area(GtkWidget* drawing_area)
{
    GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline_));
    gst_bus_set_sync_handler(bus, (GstBusSyncHandler)create_window, drawing_area);
    gst_object_unref (bus);
}

Pipeline::~Pipeline() {}

