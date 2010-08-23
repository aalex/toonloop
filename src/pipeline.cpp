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
#include <boost/filesystem.hpp>
#include <clutter-gst/clutter-gst.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <iostream>
#include <stdlib.h> // for itoa()

#include "application.h"
#include "configuration.h"
#include "gui.h"
#include "log.h" // TODO: make it async and implement THROW_ERROR
#include "pipeline.h"
#include "timing.h"

//const bool USE_SHADER = false;
namespace fs = boost::filesystem;

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
        g_print("The stream ended. Let's quit.\n");
        //gst_element_set_state(pipeline, GST_STATE_NULL);
        //gst_object_unref(pipeline);
        //FIXME: causes a segfault
        //Application::get_instance().quit();
    }
}

/** 
 * Adds an image to the current clip.
 * TODO: should be moved out of here, to application. 
 */
void Pipeline::remove_frame()
{
    Clip *thisclip = Application::get_instance().get_current_clip();

    thisclip->lock_mutex();
    //int num_deleted;
    //num_deleted = 
    thisclip->frame_remove();
    thisclip->unlock_mutex();
    //if (num_deleted > 0)
    //    std::cout << "Deleted " << num_deleted << " frames in clip " << thisclip->get_id() << std::endl; 
}
/** 
 * Adds an image to the current clip.
 * TODO: should be moved out of here, to application. 
 */
void Pipeline::grab_frame()
{
    GdkPixbuf* pixbuf;
    Clip *thisclip = Application::get_instance().get_current_clip();
    bool is_verbose = Application::get_instance().get_configuration().get_verbose();
    thisclip->lock_mutex();
    int current_clip_id = thisclip->get_id();
    g_object_get(G_OBJECT(gdkpixbufsink_), "last-pixbuf", &pixbuf, NULL);
    if (! GDK_IS_PIXBUF(pixbuf))
    {
        std::cout << "No picture yet to grab!" << std::endl;
        thisclip->unlock_mutex();
        return;
    }

    int w = gdk_pixbuf_get_width(pixbuf);
    int h = gdk_pixbuf_get_height(pixbuf);
    int nchannels = gdk_pixbuf_get_n_channels(pixbuf);

    /* if this is the first frame grabbed, set frame properties in clip */
    if (! thisclip->get_has_recorded_frame()) 
    {
        // TODO: each image should be a different size, if that's what it is.
        thisclip->set_width(w);
        thisclip->set_height(h);
        thisclip->set_has_recorded_frame();
    }

    int image_number = thisclip->frame_add();
    Image *thisimage = &thisclip->get_image(image_number);
    if (is_verbose)
        std::cout << "Current clip: " << current_clip_id << ". Image number: " << image_number << std::endl;
    std::string file_name = thisclip->get_image_full_path(thisimage);
    // We can store the pixel data in RAM or not.
    // We need 3 textures: 
    //  * the onionskin of the last frame grabbed. (or at the writehead position)
    //  * the frame at the playhead position
    //  * the current live input. (GST gives us this one)

    if (!gdk_pixbuf_save(pixbuf, file_name.c_str(), "jpeg", NULL, "quality", "100", NULL))
    {
        g_print("Image %s could not be saved. Error\n", file_name.c_str());
    } else {
        if (is_verbose)
            g_print("Image %s saved\n", file_name.c_str());
    }
    if (Application::get_instance().get_configuration().get_images_in_ram())
    {
        size_t buf_size = w * h * nchannels;
        thisimage->allocate_image(w * h * nchannels);
        char *buf = thisimage->get_rawdata();
        /* copy gdkpixbuf raw data to Image's buffer. Will be used for the texture of the grabbed frames */
        memcpy(buf, gdk_pixbuf_get_pixels(pixbuf), w * h * nchannels);
    }
    // TODO:2010-08-06:aalex:Deprecate Image::set_ready() and Image::is_ready()
    thisimage->set_ready(true);
    g_object_unref(pixbuf);
    thisclip->unlock_mutex();
}

/**
 * Stops the pipeline.
 * Called from Application::quit()
 */
void Pipeline::stop()
{
    std::cout << "Stopping the Gstreamer pipeline." << std::endl;
    // FIXME: a segfault occurs here!
    gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_NULL);
    //std::cout << "Deleting the Gstreamer pipeline." << std::endl;
    //gst_object_unref(pipeline_);
    // gtk main quit?
}

/**
 * Constructor which create the Gstreamer pipeline.
 * This pipeline grabs the video and render the OpenGL.
 */
Pipeline::Pipeline()
{
    Configuration config = Application::get_instance().get_configuration();

    //onionskin_texture_ = Texture();
    //playback_texture_ = Texture();
    
    pipeline_ = NULL;
    
    pipeline_ = GST_PIPELINE(gst_pipeline_new("pipeline"));
    
    // Video source element
    if (config.videoSource() == std::string("test")) 
    {
        std::cout << "Video source: videotestsrc" << std::endl;
        videosrc_  = gst_element_factory_make("videotestsrc", "videosrc0");
    } 
    else if (config.videoSource() == std::string("x")) 
    {
        std::cout << "Video source: ximagesrc" << std::endl;
        videosrc_  = gst_element_factory_make("ximagesrc", "videosrc0");
    } 
    else 
    {
        std::cout << "Video source: v4l2src" << std::endl;
        videosrc_  = gst_element_factory_make("v4l2src", "videosrc0"); 
        // TODO: add more input types like in Ekiga
    }
    g_assert(videosrc_); 
    // TODO: use something else than g_assert to see if we could create the elements.
    
    // capsfilter element #0
        // Possible values for the capture FPS:
        // 25/1 
        // 30000/1001
        // 30/1
    GstElement* capsfilter0 = gst_element_factory_make ("capsfilter", NULL);
    // capsfilter0, for the capture FPS and size
    
    // TODO: we should use the capture_frame_rate, not the rendering frame rate!
    // There are 3 FPS values to consider.
    //GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv", // TODO: rgb ?
    //                                    "width", G_TYPE_INT, 640, // TODO: make configurable!
    //                                    "height", G_TYPE_INT, 480,
    //                                    "framerate", GST_TYPE_FRACTION, config.get_capture_fps() * 1000, 1001,
    //                                    NULL); 
    //std::cout << "Video capture caps: " << 640 << "x" << 480 << " @ " << config.get_capture_fps() << std::endl;
    //g_object_set(capsfilter0, "caps", caps, NULL);
    //gst_caps_unref(caps);

    // ffmpegcolorspace0 element
    GstElement* ffmpegcolorspace0 = gst_element_factory_make("ffmpegcolorspace", "ffmpegcolorspace0");
    g_assert(ffmpegcolorspace0);
    GstElement* tee0 = gst_element_factory_make("tee", "tee0");
    g_assert(tee0);
    GstElement* queue0 = gst_element_factory_make("queue", "queue0");
    g_assert(queue0);

    videosink_ = clutter_gst_video_sink_new(CLUTTER_TEXTURE(Application::get_instance().get_gui().get_live_input_texture()));
    // TODO: Make sure the rendering FPS is constant, and not subordinate to
    // the FPS of the camera.

    // GdkPixbuf sink:
    GstElement* queue1 = gst_element_factory_make("queue", "queue1");
    g_assert(queue1);
    gdkpixbufsink_ = gst_element_factory_make("gdkpixbufsink", "gdkpixbufsink0");
    g_assert(gdkpixbufsink_);
    //gst_caps_unref(outcaps);

    // add elements
    gst_bin_add(GST_BIN(pipeline_), videosrc_); // capture
    gst_bin_add(GST_BIN(pipeline_), capsfilter0);
    gst_bin_add(GST_BIN(pipeline_), ffmpegcolorspace0);
    gst_bin_add(GST_BIN(pipeline_), tee0);
    gst_bin_add(GST_BIN(pipeline_), queue0); // branch #0: videosink
    //gst_bin_add(GST_BIN(pipeline_), capsfilter1);
    gst_bin_add(GST_BIN(pipeline_), videosink_);
    gst_bin_add(GST_BIN(pipeline_), queue1); // branch #1: gdkpixbufsink
    gst_bin_add(GST_BIN(pipeline_), gdkpixbufsink_);

    // link pads:
    gboolean is_linked = NULL;
    bool source_is_linked = false;
    int frame_rate_index = 0;
    if (config.videoSource() == std::string("test") || config.videoSource() == std::string("x")) 
    {
        if (config.videoSource() == std::string("x")) 
        {
            g_object_set(G_OBJECT(videosrc_), "endx", 640, NULL);
            g_object_set(G_OBJECT(videosrc_), "endy", 480, NULL);
            //std::cout << "Calling gst_caps_from_string" << std::endl;
            GstCaps *the_caps = gst_caps_from_string("video/x-raw-rgb, framerate=30/1");
            g_object_set(capsfilter0, "caps", the_caps, NULL);
            gst_caps_unref(the_caps);
            
        } else {
            //std::cout << "Using 640x480 @ 30 FPS for the videotestsrc." << std::endl;
            //g_object_set(capsfilter0, "caps", gst_caps_from_string("video/x-raw-yuv, width=640, height=480, framerate=30/1"));
            //std::cout << "Calling gst_caps_from_string" << std::endl;
            GstCaps *the_caps = gst_caps_from_string("video/x-raw-yuv, width=640, height=480, framerate=30/1");
            //std::cout << "Calling g_object_set on the caps." << std::endl;
            g_object_set(capsfilter0, "caps", the_caps, NULL);
            gst_caps_unref(the_caps);
            //std::cout << "set the caps for the testsrc" << std::endl;
        }
        is_linked = gst_element_link_pads(videosrc_, "src", capsfilter0, "sink");
        if (!is_linked) {
            g_print("Could not link %s to %s.\n", "videosrc_", "capfilter0"); 
            exit(1); 
        }
        //std::cout << "videosrc is linked" << std::endl;
        source_is_linked = true;
    } else {
        // Guess the right FPS to use with the video capture device
        while (not source_is_linked)
        {
            g_object_set(capsfilter0, "caps", gst_caps_from_string(guess_source_caps(frame_rate_index).c_str()), NULL);
            is_linked = gst_element_link_pads(videosrc_, "src", capsfilter0, "sink");
            if (!is_linked) 
            { 
                std::cout << "Failed to link video source. Trying an other framerate." << std::endl;
                ++frame_rate_index;
                if (frame_rate_index >= 10) 
                {
                    std::cout << "Giving up after 10 tries." << std::endl;
                }
            } else {
                std::cout << "Success." << std::endl;
                source_is_linked = true;
            }
        }
    }
    //std::cout << "Figured out the caps of the video source." << std::endl;
    //std::cout << "Will now link capfilter0--ffmpegcolorspace0--tee." << std::endl;
    is_linked = gst_element_link_pads(capsfilter0, "src", ffmpegcolorspace0, "sink");
    if (!is_linked) {
        g_print("Could not link %s to %s.\n", "capsfilter0", "ffmpegcolorspace0"); 
        exit(1);
    }
    is_linked = gst_element_link_pads(ffmpegcolorspace0, "src", tee0, "sink");
    if (!is_linked) {
        g_print("Could not link %s to %s.\n", "ffmpegcolorspace0", "tee0"); 
        exit(1);
    }
    //std::cout << "Will now link tee--queue--videosink." << std::endl;
    is_linked = gst_element_link_pads(tee0, "src0", queue0, "sink");
    if (!is_linked) {
        g_print("Could not link %s to %s.\n", "tee0", "sink"); 
        exit(1);
    }
    // output 0: the OpenGL uploader.
    is_linked = gst_element_link_pads(queue0, "src", videosink_, "sink");
    if (!is_linked) {
        // FIXME:2010-08-06:aalex:We could get the name of the GST element for the clutter sink using gst_element_get_name
        g_print("Could not link %s to %s.\n", "queue0", "cluttervideosink0");
        exit(1); 
    }

    // output 1: the GdkPixbuf sink
    //std::cout << "Will now link tee--queue--pixbufsink." << std::endl;
    is_linked = gst_element_link_pads(tee0, "src1", queue1, "sink");
    if (!is_linked) { 
        g_print("Could not link %s to %s.\n", "tee0", "queue1"); 
        exit(1); 
    }
    is_linked = gst_element_link_pads(queue1, "src", gdkpixbufsink_, "sink");
    if (!is_linked) { 
        g_print("Could not link %s to %s.\n", "queue1", "gdkpixbufsink0"); 
        exit(1); 
    }

    std::cout << "Will now setup the pipeline bus." << std::endl;
    /* setup bus */
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), this);
    gst_object_unref(bus);

    // TODO:2010-08-06:aalex:We could rely on gstremer-properties to configure the video source.
    if (config.videoSource() != std::string("test") && config.videoSource() != std::string("x"))
    {
        std::string device_name = config.videoSource(); // "/dev/video0";
        g_print("Using camera %s.\n", device_name.c_str());
        g_object_set(videosrc_, "device", device_name.c_str(), NULL); 
    }
    // make it play !!

    /* run */
    GstStateChangeReturn ret;
    ret = gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_print("Failed to start the video pipeline!\n");
        g_print("-----------------------------------\n");
        /* check if there is an error message with details on the bus */
        GstMessage* msg = gst_bus_poll(bus, GST_MESSAGE_ERROR, 0);
        if (msg)
        {
          GError *err = NULL;
          gst_message_parse_error(msg, &err, NULL);
          g_print("ERROR: %s\n", err->message);
          g_error_free(err);
          gst_message_unref(msg);
        }
        g_print("-----------------------------------\n");
        exit(1);
        //FIXME: causes a segfault: Application::get_instance().quit();
    }

}

// Desctructor. TODO: do we need to free anything?
Pipeline::~Pipeline() {}

std::string Pipeline::guess_source_caps(unsigned int framerateIndex) const
{
    LOG_DEBUG("Trying to guess source FPS " << framerateIndex);

    std::ostringstream capsStr;
    GstStateChangeReturn ret = gst_element_set_state(videosrc_, GST_STATE_READY);
    if (ret not_eq GST_STATE_CHANGE_SUCCESS)
        THROW_ERROR("Could not change v4l2src state to READY");
    GstPad *srcPad = gst_element_get_static_pad(videosrc_, "src");
    GstCaps *caps = gst_pad_get_caps(srcPad);
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    const GValue *val = gst_structure_get_value(structure, "framerate");
    LOG_DEBUG("Caps structure from v4l2src srcpad: " << gst_structure_to_string(structure));
    gint framerate_numerator, framerate_denominator; 
    if (GST_VALUE_HOLDS_LIST(val))
    {
        // trying another one
        if (framerateIndex >= gst_value_list_get_size(val))
            THROW_ERROR("Framerate index out of range");
        framerate_numerator = gst_value_get_fraction_numerator((gst_value_list_get_value(val, framerateIndex)));
        framerate_denominator = gst_value_get_fraction_denominator((gst_value_list_get_value(val, framerateIndex)));
    }
    else
    {
        // FIXME: this is really bad, we should be iterating over framerates and resolutions until we find a good one
        if (framerateIndex > 0)
            LOG_ERROR("Caps parameters haven't been changed and have failed before");
        framerate_numerator = gst_value_get_fraction_numerator(val);
        framerate_denominator = gst_value_get_fraction_denominator(val);
    }

    gst_caps_unref(caps);
    gst_object_unref(srcPad);

    // use default from gst
    std::string capsSuffix = boost::lexical_cast<std::string>(framerate_numerator);
    capsSuffix += "/";
    capsSuffix += boost::lexical_cast<std::string>(framerate_denominator);

    // TODO: 
    //if (v4l2util::isInterlaced(deviceStr()))
    //    capsSuffix +=", interlaced=true";

    // TODO:
    //capsSuffix += ", pixel-aspect-ratio=";
    //capsSuffix += config_.pixelAspectRatio();
    //capsSuffix += "4:3";
    
    capsStr << "video/x-raw-yuv, width=" 
        << "640" //<< config_.captureWidth() 
        << ", height="
        << "480" //<< config_.captureHeight()
        << ", framerate="
        << capsSuffix;
    LOG_DEBUG("Video source caps are " << capsStr.str());
    ret = gst_element_set_state(videosrc_, GST_STATE_NULL);
    if (ret not_eq GST_STATE_CHANGE_SUCCESS)
        THROW_ERROR("Could not change v4l2src state to NULL");

    return capsStr.str();
}

