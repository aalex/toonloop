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
#include <boost/filesystem.hpp>
#include <clutter-gst/clutter-gst.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <iostream>
#include <stdlib.h> // for itoa()

#include "application.h"
#include "clip.h"
#include "configuration.h"
#include "controller.h"
#include "gui.h"
#include "log.h" // TODO: make it async and implement THROW_ERROR
#include "pipeline.h"
#include "timing.h"
#include "raw1394util.h"
#include "v4l2util.h"

//const bool USE_SHADER = false;
namespace fs = boost::filesystem;

/**
 * Called every time there is a message on the GStreamer pipeline's bus.
 *
 * We are mostly interested in the new pixbug message.
 * In that case, checks if the video recording or the intervalometer is enabled. 
 * If so, grabs an image if it's time to do so.
 */
void Pipeline::bus_message_cb(GstBus* /*bus*/, GstMessage *msg, gpointer user_data)
{
    Pipeline *context = static_cast<Pipeline*>(user_data);
    bool verbose = context->owner_->get_configuration()->get_verbose();
    switch (GST_MESSAGE_TYPE (msg)) 
    {
    case GST_MESSAGE_ELEMENT:
    {
        const GValue *val;
        GdkPixbuf *pixbuf = NULL;
  
        /* only interested in element messages from our gdkpixbufsink */
        if (msg->src != GST_OBJECT_CAST(context->gdkpixbufsink_))
            break;
  
        /* only interested in these two messages */
        if (!gst_structure_has_name(msg->structure, "preroll-pixbuf") &&
                !gst_structure_has_name(msg->structure, "pixbuf")) 
        {
            break;
        }
  
        //g_print("pixbuf\n");
        val = gst_structure_get_value(msg->structure, "pixbuf");
        g_return_if_fail(val != NULL);
  
        pixbuf = GDK_PIXBUF(g_value_dup_object(val));
        if (context->get_record_all_frames() || context->get_intervalometer_is_on()) // if video grabbing is enabled
        {
            Clip *current_clip = context->owner_->get_current_clip();
            unsigned long last_time_grabbed = current_clip->get_last_time_grabbed_image();
            unsigned long now = timing::get_timestamp_now();
            bool must_grab_now = false;
            // VIDEO RECORDING:
            if (context->get_record_all_frames())
            {
                //std::cout << "Video grabbing is on." << std::endl; 
                unsigned long time_between_frames = (unsigned long)(1.0f / float(current_clip->get_playhead_fps()) * timing::TIMESTAMP_PRECISION);
                if (verbose)
                    std::cout << "now=" << now << " last_time_grabbed=" << last_time_grabbed << " time_between_frames" << time_between_frames << std::endl;
                if ((now - last_time_grabbed) > time_between_frames)
                {
                    must_grab_now = true;
                }
            } // not mutually exclusive - why not have both on?
            // INTERVALOMETER:
            if (context->get_intervalometer_is_on())
            {
                long time_between_intervalometer_ticks = long(current_clip->get_intervalometer_rate() * timing::TIMESTAMP_PRECISION);
                long passed = (now - last_time_grabbed);
                if (verbose)
                    std::cout << "time between intervalometer ticks: " << passed << "/" << time_between_intervalometer_ticks << std::endl;
                if (passed > time_between_intervalometer_ticks)
                {
                    if (verbose)
                        std::cout << "Interval has passed. Time to grab." << std::endl;
                    must_grab_now = true;
                }
            }
            if (must_grab_now)
            {
                if (verbose)
                    std::cout << "Grabbing an image" << std::endl;
                context->save_image_to_current_clip(pixbuf);
                current_clip->set_last_time_grabbed_image(now);
            }
        }
        g_object_unref(pixbuf);
        break;
    }
    case GST_MESSAGE_ERROR:
    {
        GError *err = NULL;
        gchar *dbg = NULL;
        gst_message_parse_error(msg, &err, &dbg);
        g_error("Error: %s\n%s\n", err->message, (dbg) ? dbg : "");
        g_error_free(err);
        g_free(dbg);
        break;
    }
    default:
        break;
    }
}

static void show_unlinked_pads(GstBin *bin)
{
    GstPad *pad = gst_bin_find_unlinked_pad(bin, GST_PAD_SRC);
    if (pad)
    {
        gchar *pad_name = gst_pad_get_name(pad);
        g_print("Pad %s is not linked.\n", pad_name);
        g_free(pad_name);
    }
}

/**
 * GST bus signal watch callback 
 */
void Pipeline::end_stream_cb(GstBus* /*bus*/, GstMessage* message, gpointer user_data)
{
    Pipeline *context = static_cast<Pipeline*>(user_data);
    bool is_verbose = context->owner_->get_configuration()->get_verbose();
    if (is_verbose)
        std::cout << "Pipeline::" << __FUNCTION__ << "(" << message << ")" << std::endl;
    bool stop_it = true;
    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR)
    {
        gchar *debug = NULL;
        GError *err = NULL;
        gst_message_parse_error(message, &err, &debug);
        g_printerr("GStreamer ERROR from element %s: %s\n", GST_OBJECT_NAME(message->src), err->message);
        show_unlinked_pads(GST_BIN(context->pipeline_));
        //if (message->src == GST_OBJECT(context->videosrc_))
        //{
        //    g_print("The error message comes from %s. Not stopping the pipeline.\n", GST_OBJECT_NAME(message->src));
        //    stop_it = false;
        //}
        g_error_free(err);
        if (debug) 
        {
            g_print("Error message Debug details: %s\n", debug);
            g_free(debug);
        }
    } 
    else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) 
    {
        std::cout << "EOS: End of stream" << std::endl;
    } 
    else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_WARNING) 
    {
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
        // context->owner_->quit();
    }
}

void Pipeline::on_new_live_pixbuf(GstBus* /*bus*/, GstMessage* /*message*/, gpointer /*user_data*/)
{
    std::cout << "on_new_live_pixbuf" << std::endl;
}

/** 
 * Adds an image to the current clip.
 * TODO: should be moved out of here, to application. 
 */
void Pipeline::remove_frame()
{
    Clip *thisclip = owner_->get_current_clip();

    //int num_deleted;
    //num_deleted = 
    thisclip->frame_remove();
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
    g_object_get(G_OBJECT(gdkpixbufsink_), "last-pixbuf", &pixbuf, NULL);
    if (! GDK_IS_PIXBUF(pixbuf))
    {
        std::cout << "No picture yet to grab!" << std::endl;
    } else {
        save_image_to_current_clip(pixbuf);
    }
    g_object_unref(pixbuf);
}
/**
 * Saves a GdkPixbuf image to the current clip.
 *
 * Needed, because the image might come from the last grabbed
 * frame, or the recording of every frame might be automatic. (video)
 *
 * The gdkpixbufsink element posts messages containing the pixbuf.
 */
void Pipeline::save_image_to_current_clip(GdkPixbuf *pixbuf)
{
    Clip *thisclip = owner_->get_current_clip();
    bool is_verbose = owner_->get_configuration()->get_verbose();
    int current_clip_id = thisclip->get_id();
    int w = gdk_pixbuf_get_width(pixbuf);
    int h = gdk_pixbuf_get_height(pixbuf);

    /* if this is the first frame grabbed, set frame properties in clip */
    // TODO:2010-08-27:aalex:Use image size, not clip size. 
    // A clip may contain images that are not the same size.
    if (! thisclip->get_has_recorded_frame()) 
    {
        // TODO: each image should be a different size, if that's what it is.
        thisclip->set_width(w);
        thisclip->set_height(h);
        thisclip->set_has_recorded_frame();
    }

    int new_image_number = thisclip->frame_add();
    Image *new_image = thisclip->get_image(new_image_number);
    if (new_image == 0)
    {
        // This is very unlikely to happen
        std::cerr << "No image at " << new_image_number << std::endl;
        gdk_pixbuf_unref(pixbuf);
        return;
    }
    if (is_verbose)
        std::cout << "Grab a frame. Current clip: " << current_clip_id << ". Image number: " << new_image_number << std::endl;
    std::string file_name = thisclip->get_image_full_path(new_image);
    // We need 3 textures: 
    //  * the onionskin of the last frame grabbed. (or at the writehead position)
    //  * the frame at the playhead position
    //  * the current live input. (GST gives us this one)

    if (!gdk_pixbuf_save(pixbuf, file_name.c_str(), "jpeg", NULL, "quality", "100", NULL))
    {
        g_print("Image %s could not be saved. Error\n", file_name.c_str());
    }
    else
    {
        if (is_verbose)
            g_print("Image %s saved\n", file_name.c_str());
        owner_->get_controller()->add_frame_signal_(current_clip_id, new_image_number);
        // Removes the first image if the maximum number of frames has been reached.
        if (owner_->get_configuration()->get_max_images_per_clip() != 0)
        {
            unsigned int max_num = (unsigned int) owner_->get_configuration()->get_max_images_per_clip();
            if (thisclip->size() > max_num)
            {
                thisclip->remove_first_image();
                if (is_verbose)
                    std::cout << "Removing the first image! Max of " << max_num << " has been reached." << std::endl;
            }
        }
    }
}

/**
 * Stops the pipeline.
 * Called from Application::quit()
 */
void Pipeline::stop()
{
    if (owner_->get_configuration()->get_verbose())
        std::cout << "Stopping the GStreamer pipeline." << std::endl;
    // FIXME: a segfault occurs here!
    gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_NULL);
    //std::cout << "Deleting the GStreamer pipeline." << std::endl;
    //gst_object_unref(pipeline_);
    // gtk main quit?
}

void Pipeline::link_or_die(GstElement *from, GstElement *to)
{
    bool is_linked = gst_element_link(from, to);
    gchar *from_name = gst_element_get_name(from);
    gchar *to_name = gst_element_get_name(to);
    if (!is_linked) 
    {
        g_print("Could not link %s to %s.\n", from_name, to_name);
        exit(1); 
    }
    g_free(from_name);
    g_free(to_name);
}

/// Called due to incoming dv stream, either video or audio, links appropriately
void Pipeline::cb_new_dvdemux_src_pad(GstElement * /*srcElement*/, GstPad * srcPad, gpointer data)
{
    GstElement *sinkElement = static_cast<GstElement*>(data);
    if (std::string("video") == gst_pad_get_name(srcPad))
    {
        LOG_DEBUG("Got video stream from DV\n");
    }
    else 
    {
        g_print("Ignoring %s stream from DV\n", gst_pad_get_name(srcPad));
        return;
    }

    GstPad *sinkPad;
    sinkPad = gst_element_get_static_pad(sinkElement, "sink");
    if (GST_PAD_IS_LINKED(sinkPad))
    {
        g_object_unref(sinkPad); // don't link more than once
        return;
    }
    gchar *sink_pad_name = gst_pad_get_name(sinkPad);
    gchar *sink_element_name = gst_element_get_name(sinkElement);
    g_print("Pipeline::%s: Dv1394: linking %s src pad to %s's %s sinkpad.", __FUNCTION__, "video", sink_element_name, sink_pad_name);
    bool is_linked = gst_pad_link(srcPad, sinkPad);
    if (! is_linked) 
    {
        g_print("Could not link %s to %s.\n", "dv_decoder0", sink_element_name); 
        exit(1);
    }
    g_free(sink_pad_name);
    g_free(sink_element_name);
    g_print("Success!\n");
    gst_object_unref(sinkPad);
}

/**
 * Constructor which create the GStreamer pipeline.
 * This pipeline grabs the video and render the OpenGL.
 */
Pipeline::Pipeline(Application* owner) :
        owner_(owner), 
        record_all_frames_enabled_(false)
{
    Configuration *config = owner_->get_configuration();
    bool verbose = config->get_verbose();
    set_intervalometer_is_on(false);
    pipeline_ = NULL;
    pipeline_ = GST_PIPELINE(gst_pipeline_new("pipeline"));

    GstElement* dv_decoder0 = NULL;
    GstElement* hdv_decoder0 = NULL;
    GstElement* dv_videoscale0 = NULL;
    GstElement* dv_ffmpegcolorspace = NULL;
    GstElement* dvdec = NULL;
    //GstElement* dv_queue0 = NULL;
    //GstElement* dv_queue1 = NULL;
    // capsfilter0, for the capture FPS and size
    GstElement* capsfilter0 = gst_element_factory_make ("capsfilter", NULL);

    bool is_dv_enabled = config->videoSource() == "dv";
    bool is_hdv_enabled = config->videoSource() == "hdv";
    // Video source element
    // TODO: add more input types like in Ekiga
    if (verbose)
        std::cout << "Video source: " << config->videoSource() << std::endl;
    if (config->videoSource() == "test")
    {
        videosrc_  = gst_element_factory_make("videotestsrc", "videosrc0");
    } 
    else if (config->videoSource() == "x") 
    {
        videosrc_  = gst_element_factory_make("ximagesrc", "videosrc0");
    } 
    else if (config->videoSource() == "dv") 
    {
        if (! Raw1394::cameraIsReady())
            g_error("There is no DV camera that is ready.");
        videosrc_  = gst_element_factory_make("dv1394src", "videosrc0");
        //dv_queue0  = gst_element_factory_make("queue", "dv_queue0");
        dv_decoder0 = gst_element_factory_make("dvdemux", "dv_decoder0");
        dvdec = gst_element_factory_make("dvdec", "dvdec");
        dv_videoscale0 = gst_element_factory_make("videoscale", "dv_videoscale0");
        dv_ffmpegcolorspace = gst_element_factory_make("ffmpegcolorspace", "dv_ffmpegcolorspace");
        //dv_queue1  = gst_element_factory_make("queue", "dv_queue1");
        // register connection callback for the dvdemux element.
        // Note that the demuxer will be linked to whatever after it dynamically.
        // The reason is that the DV may contain various streams (for example
        // audio and video). The source pad(s) will be created at run time,
        // by the demuxer when it detects the amount and nature of streams.
        // Therefore we connect a callback function which will be executed
        // when the "pad-added" is emitted.
        g_signal_connect(dv_decoder0, "pad-added",
            G_CALLBACK(cb_new_dvdemux_src_pad),
            static_cast<gpointer>(dvdec));
        g_assert(dv_decoder0);
    } 
    else if (config->videoSource() == "hdv") 
    {
        videosrc_  = gst_element_factory_make("hdv1394src", "videosrc0");
        hdv_decoder0 = gst_element_factory_make("decodebin", "hdv_decoder0");
        g_assert(hdv_decoder0);
    } 
    else  // v4l2src
    {
        // TODO:2010-08-06:aalex:We could rely on gstreamer-properties to configure the video source.
        // Add -d gconf (gconfvideosrc)
        std::string device_name(config->videoSource());
        if (verbose)
        {
            std::cout << "Video source: v4l2src with camera " << device_name << std::endl;
        }
        videosrc_  = gst_element_factory_make("v4l2src", "videosrc0"); 
        g_object_set(videosrc_, "device", device_name.c_str(), NULL); 
    }
    // TODO: use something else than g_assert to see if we could create the elements.
    g_assert(videosrc_); 

    // ffmpegcolorspace0 element
    GstElement* ffmpegcolorspace0 = gst_element_factory_make("ffmpegcolorspace", "ffmpegcolorspace0");
    g_assert(ffmpegcolorspace0);
    GstElement* tee0 = gst_element_factory_make("tee", "tee0");
    g_assert(tee0);
    GstElement* queue0 = gst_element_factory_make("queue", "queue0");
    g_assert(queue0);

    videosink_ = clutter_gst_video_sink_new(CLUTTER_TEXTURE(owner_->get_gui()->get_live_input_texture()));
    // TODO: Make sure the rendering FPS is constant, and not subordinate to
    // the FPS of the camera.

    // GdkPixbuf sink:
    GstElement* queue1 = gst_element_factory_make("queue", "queue1");
    g_assert(queue1);
    gdkpixbufsink_ = gst_element_factory_make("gdkpixbufsink", "gdkpixbufsink0");
    g_assert(gdkpixbufsink_);

    bool is_preview_enabled = config->get_preview_window_enabled();
    GstElement *queue2 = NULL;
    GstElement *ffmpegcolorspace1 = NULL;
    GstElement *xvimagesink = NULL;
    if (is_preview_enabled)
    {
        queue2 = gst_element_factory_make("queue", "queue2");
        g_assert(queue2);
        ffmpegcolorspace1 = gst_element_factory_make("ffmpegcolorspace", "ffmpegcolorspace1");
        g_assert(ffmpegcolorspace1);
        xvimagesink = gst_element_factory_make("xvimagesink", "xvimagesink0");
        g_assert(xvimagesink);
        g_object_set(xvimagesink, "force-aspect-ratio", TRUE, NULL);
    }

    // add elements
    gst_bin_add(GST_BIN(pipeline_), videosrc_); // capture
    gst_bin_add(GST_BIN(pipeline_), capsfilter0);
    if (is_dv_enabled)
    {
        //gst_bin_add(GST_BIN(pipeline_), dv_queue0);
        gst_bin_add(GST_BIN(pipeline_), dv_decoder0);
        gst_bin_add(GST_BIN(pipeline_), dvdec);
        gst_bin_add(GST_BIN(pipeline_), dv_videoscale0);
        gst_bin_add(GST_BIN(pipeline_), dv_ffmpegcolorspace);
        //gst_bin_add(GST_BIN(pipeline_), dv_queue1);
        // Set the capsfilter caps for DV:
    } 
    else if (is_hdv_enabled)
    {
        gst_bin_add(GST_BIN(pipeline_), hdv_decoder0);
    }
    gst_bin_add(GST_BIN(pipeline_), ffmpegcolorspace0);
    gst_bin_add(GST_BIN(pipeline_), tee0);
    gst_bin_add(GST_BIN(pipeline_), queue0); // branch #0: videosink
    //gst_bin_add(GST_BIN(pipeline_), capsfilter1);
    gst_bin_add(GST_BIN(pipeline_), videosink_);
    gst_bin_add(GST_BIN(pipeline_), queue1); // branch #1: gdkpixbufsink
    gst_bin_add(GST_BIN(pipeline_), gdkpixbufsink_);
    if (is_preview_enabled)
    {
        gst_bin_add(GST_BIN(pipeline_), queue2); // branch #2: xvimagesink
        gst_bin_add(GST_BIN(pipeline_), ffmpegcolorspace1);
        gst_bin_add(GST_BIN(pipeline_), xvimagesink);
    }

    // link pads:
    gboolean is_linked = FALSE; 
    if (config->videoSource() == std::string("test") || config->videoSource() == std::string("x")) 
    {
        if (config->videoSource() == std::string("x")) 
        {
            g_object_set(G_OBJECT(videosrc_), "endx", config->get_capture_width(), NULL);
            g_object_set(G_OBJECT(videosrc_), "endy", config->get_capture_height(), NULL);
            GstCaps *the_caps = gst_caps_from_string("video/x-raw-rgb, framerate=30/1");
            g_object_set(capsfilter0, "caps", the_caps, NULL);
            gst_caps_unref(the_caps);
            
        }
        else
        {   // it's a videotestsrc
            //TODO:2010-10-04:aalex:Use config.get_capture_* for test source as well
            // TODO: make videotestsrc size configurable as well!
            GstCaps *the_caps = gst_caps_from_string("video/x-raw-yuv, width=640, height=480, framerate=30/1");
            g_object_set(capsfilter0, "caps", the_caps, NULL);
            gst_caps_unref(the_caps);
        }

        link_or_die(videosrc_, capsfilter0);
    } 
    else if (is_dv_enabled || is_hdv_enabled) 
    {
        if (is_dv_enabled)
        {
            link_or_die(videosrc_, dv_decoder0);
            // dv_decoder0 is linked to dvdec when its src pads appear
            link_or_die(dvdec, dv_videoscale0); // FIXME: rename dv_decoder0 to dvdemux0
            link_or_die(dv_videoscale0, dv_ffmpegcolorspace);
            link_or_die(dv_ffmpegcolorspace, capsfilter0);
        } 
        else // hdv
        {
            g_error("HDV is not yet implemented."); // quits
        }
    } 
    else // it's a v4l2src
    {     
        bool source_is_linked = false;
        int frame_rate_index = 0;
        // Guess the right FPS to use with the video capture device
        while (not source_is_linked)
        {
            GstCaps *videocaps = gst_caps_from_string(guess_source_caps(frame_rate_index).c_str());
            g_object_set(capsfilter0, "caps", videocaps, NULL);
            gst_caps_unref(videocaps);
            is_linked = gst_element_link(videosrc_, capsfilter0);
            if (!is_linked) 
            { 
                std::cout << "Failed to link video source. Trying another framerate." << std::endl;
                ++frame_rate_index;
            }
            else 
            {
                if (verbose)
                    std::cout << "Success." << std::endl;
                source_is_linked = true;
            }
        }
    }
    //Will now link capfilter0--ffmpegcolorspace0--tee.
    link_or_die(capsfilter0, ffmpegcolorspace0);
    link_or_die(ffmpegcolorspace0, tee0);
    //Will now link tee--queue--videosink.
    is_linked = gst_element_link_pads(tee0, "src0", queue0, "sink");
    if (!is_linked) 
    {
        g_print("Could not link %s to %s.\n", "tee0", "sink"); 
        exit(1);
    }
    // output 0: the OpenGL uploader.
    link_or_die(queue0, videosink_);

    // output 1: the GdkPixbuf sink
    //Will now link tee--queue--pixbufsink.
    is_linked = gst_element_link_pads(tee0, "src1", queue1, "sink");
    if (!is_linked) 
    { 
        g_print("Could not link %s to %s.\n", "tee0", "queue1"); 
        exit(1); 
    }
    link_or_die(queue1, gdkpixbufsink_);

    if (is_preview_enabled)
    {
        
        is_linked = gst_element_link_pads(tee0, "src2", queue2, "sink");
        if (!is_linked) 
        { 
            g_print("Could not link %s to %s.\n", "tee0", "queue2"); 
            exit(1); 
        }
       
        is_linked = gst_element_link(queue2, ffmpegcolorspace1);
        if (!is_linked) 
        { 
            g_print("Could not link %s to %s.\n", "queue2", "ffmpegcolorspace1"); 
            exit(1); 
        }
        is_linked = gst_element_link(ffmpegcolorspace1, xvimagesink);
        if (!is_linked) 
        { 
            g_print("Could not link %s to %s.\n", "ffmpegcolorspace1", "xvimagesink0"); 
            exit(1); 
        }
    }

    if (verbose)
        std::cout << "Will now setup the pipeline bus." << std::endl;
    /* setup bus */
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message", G_CALLBACK(bus_message_cb), this);
    gst_object_unref(bus);

    /* run */
    GstStateChangeReturn ret;
    if (verbose)
        std::cout << "Set pipeline to READY" << std::endl;
    ret = gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_READY);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_print("Failed to make the video pipeline ready!\n");
    }
    if (verbose)
        std::cout << "Set pipeline to PLAYING" << std::endl;
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
        //FIXME: causes a segfault: context->owner_->quit();
    }
    if (verbose)
        std::cout << "Successfully started the pipeline." << std::endl;
}

// Desctructor. TODO: do we need to free anything?
Pipeline::~Pipeline() {}
/**
 * Tries to guess the frame rate for a V4L2 source.
 */
std::string Pipeline::guess_source_caps(unsigned int framerateIndex) const
{
    bool is_verbose = owner_->get_configuration()->get_verbose();
    if (is_verbose)
        LOG_DEBUG("Trying to guess source FPS " << framerateIndex);

    std::ostringstream capsStr;
    GstStateChangeReturn ret = gst_element_set_state(videosrc_, GST_STATE_READY);
    if (ret not_eq GST_STATE_CHANGE_SUCCESS)
        THROW_ERROR("Could not change v4l2src state to READY");
    GstPad *srcPad = gst_element_get_static_pad(videosrc_, "src");
    GstCaps *caps = gst_pad_get_caps(srcPad);
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    const GValue *val = gst_structure_get_value(structure, "framerate");
    if (is_verbose)
        LOG_DEBUG("Caps structure from v4l2src srcpad: " << gst_structure_to_string(structure));
    gint framerate_numerator = 1;
    gint framerate_denominator = 1; 
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

    // TODO: handle interlaced video capture stream
    
    if (v4l2util::isInterlaced(owner_->get_configuration()->videoSource()))
    {
        capsSuffix +=", interlaced=true";
    }
    // TODO: handle aspect ratio
    // capsSuffix += ", pixel-aspect-ratio=1/1";
    //capsSuffix += config_.pixelAspectRatio();
    //capsSuffix += "4:3";
    
    Configuration *config = owner_->get_configuration();
    capsStr << "video/x-raw-yuv, width=" 
        << config->get_capture_width() 
        << ", height="
        << config->get_capture_height()
        << ", framerate="
        << capsSuffix;
    if (is_verbose)
        LOG_DEBUG("Video source caps are " << capsStr.str());
    ret = gst_element_set_state(videosrc_, GST_STATE_NULL);
    if (ret not_eq GST_STATE_CHANGE_SUCCESS)
        THROW_ERROR("Could not change v4l2src state to NULL");
    return capsStr.str();
}

void Pipeline::set_record_all_frames(bool enable)
{
    record_all_frames_enabled_ = enable;
}

void Pipeline::set_intervalometer_is_on(bool enable)
{
    intervalometer_is_on_ = enable;
}

