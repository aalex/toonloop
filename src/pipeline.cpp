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
#include <GL/glew.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <gst/gst.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <iostream>
#include <cstring> // for memcopy
#include "gstgtk.h"
#include "pipeline.h"
#include "draw.h"
#include "application.h"
#include "gui.h"
#include <stdlib.h> // for itoa()
#include "configuration.h"
#include "log.h" // TODO: make it async and implement THROW_ERROR
#include <boost/filesystem.hpp>
#include "timing.h"

//const bool USE_SHADER = false;
namespace fs = boost::filesystem;

/**
 * You need to create an OpenGL contect prior to use this.
 */
bool check_if_shaders_are_supported()
{
    bool shaders_are_supported = false;
    // check for shaders support
    GLenum err = glewInit(); 
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        std::cerr << "Error calling glewInit: " << glewGetErrorString(err) << std::endl;
        shaders_are_supported = false;
    } else {
        std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    }
    if (glewGetExtension("GL_ARB_fragment_program"))
    {
        std::cout << "Status: Looks like ARB_fragment_program is supported." << std::endl;
        shaders_are_supported = true;
    } else {
        shaders_are_supported = false;
    }
    return shaders_are_supported;
}

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

// TODO: make sure the number of consecutive slashes in the path is ok
std::string Pipeline::get_image_full_path(Image* image)
{
    std::string project_path = Application::get_instance().get_configuration().get_project_home();
    std::string image_name = image->get_name() + ".jpg";
    //std::string file_name = fs::path(project_path) / image_name; 
    return project_path + "/" + image_name; 
}

/** 
 * Adds an image to the current clip.
 * TODO: should be moved out of here, to application. 
 */
void Pipeline::remove_frame()
{
    Clip *thisclip = Application::get_instance().get_current_clip();
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
    Clip *thisclip = Application::get_instance().get_current_clip();
    int current_clip_id = thisclip->get_id();
    g_object_get(G_OBJECT(gdkpixbufsink_), "last-pixbuf", &pixbuf, NULL);

    int w = gdk_pixbuf_get_width(pixbuf);
    int h = gdk_pixbuf_get_height(pixbuf);
    int nchannels = gdk_pixbuf_get_n_channels(pixbuf);

    /* if this is the first frame grabbed, set frame properties in clip */
    if (! has_recorded_a_frame_) 
    {
        // TODO: each image should be a different size, if that's what it is.
        thisclip->set_width(w);
        thisclip->set_height(h);
        has_recorded_a_frame_ = true;
    }

    int image_number = thisclip->frame_add();
    Image *thisimage = &thisclip->get_image(image_number);
    std::cout << "Current clip: " << current_clip_id << ". Image number: " << image_number << std::endl;
    std::string file_name = get_image_full_path(thisimage);
    // We can store the pixel data in RAM or not.
    // We need 3 textures: 
    //  * the onionskin of the last frame grabbed. (or at the writehead position)
    //  * the frame at the playhead position
    //  * the current live input. (GST gives us this one)

    if (!gdk_pixbuf_save(pixbuf, file_name.c_str(), "jpeg", NULL, "quality", "100", NULL))
    {
        g_print("Image %s could not be saved. Error\n", file_name.c_str());
    }
    else
        g_print("Image %s saved\n", file_name.c_str());
    if (Application::get_instance().get_configuration().get_images_in_ram())
    {
        size_t buf_size = w * h * nchannels;
        thisimage->allocate_image(w * h * nchannels);
        char *buf = thisimage->get_rawdata();
        /* copy gdkpixbuf raw data to Image's buffer. Will be used for the texture of the grabbed frames */
        memcpy(buf, gdk_pixbuf_get_pixels(pixbuf), w * h * nchannels);
    }
    thisimage->set_ready(true);
    g_object_unref(pixbuf);
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

    onionskin_texture_ = Texture();
    playback_texture_ = Texture();
    
    pipeline_ = NULL;
    
    pipeline_ = GST_PIPELINE(gst_pipeline_new("pipeline"));
    //state_ = 0;
    GstElement* glupload0;
    
    // Video source element
    if (config.videoSource() == std::string("test")) 
    {
        videosrc_  = gst_element_factory_make("videotestsrc", "videosrc0");
    } else {
        videosrc_  = gst_element_factory_make("v4l2src", "videosrc0"); // TODO: add more like in Ekiga
    }
    g_assert(videosrc_); // TODO: use something else than g_assert to see if we could create the elements.
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

    // glupload element
    glupload0  = gst_element_factory_make ("glupload", "glupload0");
    // glimagesink
    videosink_ = gst_element_factory_make("glimagesink", "glimagesink0");
    g_object_set(videosink_, "sync", FALSE, NULL);
    g_object_set(G_OBJECT(videosink_), "client-reshape-callback", G_CALLBACK(reshapeCallback), NULL); // gpointer(this)
    g_object_set(G_OBJECT(videosink_), "client-draw-callback", drawCallback, NULL);
    //g_object_set(G_OBJECT(videosink_), "client-draw-callback", G_CALLBACK(drawCallback), gpointer(this));
    // capsfilter element #1, for the OpenGL FPS and size
    // TODO: Make sure the rendering FPS is constant, and not subordinate to
    // the FPS of the camera.
    GstElement* capsfilter1 = gst_element_factory_make("capsfilter", NULL);
    GstCaps *outcaps = gst_caps_new_simple("video/x-raw-gl",
                                        "width", G_TYPE_INT, 800,
                                        "height", G_TYPE_INT, 600,
                                        //"framerate", GST_TYPE_FRACTION, config.get_rendering_fps() * 1000, 1001,
                                        NULL) ;
    g_object_set(capsfilter1, "caps", outcaps, NULL);

    // GdkPixbuf sink:
    GstElement* queue1 = gst_element_factory_make("queue", "queue1");
    g_assert(queue1);
    gdkpixbufsink_ = gst_element_factory_make("gdkpixbufsink", "gdkpixbufsink0");
    g_assert(gdkpixbufsink_);
    gst_caps_unref(outcaps);

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
    bool source_is_linked = false;
    int frame_rate_index = 0;
    if (config.videoSource() == std::string("test")) 
    {
        g_object_set(capsfilter0, "caps", gst_caps_from_string(std::string("video/x-raw-yuv, width=640, height=480, framerate=30/1").c_str()));
        is_linked = gst_element_link_pads(videosrc_, "src", capsfilter0, "sink");
        if (!is_linked) { g_print("Could not link %s to %s.\n", "videotestsrc", "capfilter0"); exit(1); }
        source_is_linked = true;
    }

    while (not source_is_linked)
    {
        g_object_set(capsfilter0, "caps", gst_caps_from_string(guess_source_caps(frame_rate_index).c_str()));
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
    if (!is_linked) { g_print("Could not link %s to %s.\n", "capsfilter1", "videosink0"); exit(1); }

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
    if (config.videoSource() != std::string("test"))
    {
        std::string device_name = config.videoSource(); // "/dev/video0";
        g_print("Using camera %s.\n", device_name.c_str());
        g_object_set(videosrc_, "device", device_name.c_str(), NULL); 
    }
    has_recorded_a_frame_ = false;
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

Shader* Pipeline::get_shader()
{
    return shader_;
}

/**
 * Client reshape callback
 *
 * The OpenGL coordinates of the rendering are in the range [-1, 1] vertically 
 * and something like [-1.333, 1.333] horizontally.
 * It depends on the actual aspect ratio of the window.
*/
void reshapeCallback(GLuint width, GLuint height, gpointer data)
{
    // XXX: Pipeline* context = static_cast<Pipeline*>(data);
    // XXX: std::cout << "Hello: " << context->get_numframes() << std::endl;
    glViewport(0, 0, width, height);
    // if >= 4/3:
    float w = float(width) / float(height);
    float h = 1.0; // constant height

    // make sure the aspect ratio of the window is not less wide than 4:3
    if (w < 4.0f/3.0f)
    {
        h = (float(height) / float(width)) * 1.333333f;
        w = 1.3333333333f; // constant width
    }
    //std::cout << "Resized window to " << width << "x" << height << ". Ratio is " << w << "x" << h << std::endl; 

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w, w, -h, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //glEnable (GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //
    // enables transparency blending:
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
}

void Pipeline::set_shader(Shader* shader)
{
    shader_ = shader;
}

/**
 * This is where the OpenGL drawing is done.
 * Client draw callback
 */
gboolean drawCallback(GLuint texture, GLuint width, GLuint height, gpointer data)
{
    //Pipeline* context = static_cast<Pipeline*>(data);
    static int number_of_frames_in_last_second = 0; // counting FPS
    static bool first_draw = true;
    static Timer fps_calculation_timer = Timer();
    static Timer playback_timer = Timer(); // TODO: move to Clip
    GLuint frametexture;
    GLint texturelocation;
    bool move_playhead = false;
    Clip *thisclip = Application::get_instance().get_current_clip();

    ++ number_of_frames_in_last_second;
    playback_timer.tick();
    fps_calculation_timer.tick();

    // check if it is time to move the playhead
    if ((playback_timer.get_elapsed()) >=  (1.0f / thisclip->get_playhead_fps() * 1.0) || playback_timer.get_elapsed() < 0.0f)
    {
        move_playhead = true;
        playback_timer.reset();
    }
    // calculate rendering FPS
    if (fps_calculation_timer.get_elapsed() >= 1.0f)
    {
        std::cout << "Rendering FPS: " << number_of_frames_in_last_second << std::endl;
        number_of_frames_in_last_second = 0;
        fps_calculation_timer.reset();
    }
    // Use or load shader if enabled and supported
    Shader* myshader;
    bool USE_SHADER = Application::get_instance().get_configuration().get_effects_enabled();
    if (USE_SHADER)
    {
        if(first_draw == true)
        {
            std::cout << "Checking for GLSL shaders support..." << std::endl;
            if (check_if_shaders_are_supported())
            {
                std::cout << "GLSL shaders are supported!" << std::endl;
                myshader = new Shader(); // FIXME: should we use a scoped_ptr ?
                Application::get_instance().get_pipeline().set_shader(myshader);
            }
            // TODO: else disable effects in config
            std::cout << "Compiling the shader" << std::endl;
            myshader->compile_link();
            first_draw = false;
            texturelocation = glGetUniformLocation(myshader->get_program_object(), "image");
        } else {    
            myshader = Application::get_instance().get_pipeline().get_shader();
        }
    }
    // Enable shader
    if (USE_SHADER)
    {
        glUseProgram(myshader->get_program_object());
    }
    // Load the texture
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
    if (USE_SHADER)
    {
        glUniform1i(texturelocation, 0);
        glActiveTexture(GL_TEXTURE0);
    }
    // TODO: simplify those parameters
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    // Left image
    glPushMatrix();
    glTranslatef(-0.6666666f, 0.0f, 0.0f);
    glScalef(0.6666666f, 0.5f, 1.0f);
    draw::draw_vertically_flipped_textured_square(width, height);
    glPopMatrix();
    
    if(thisclip->size() > 0) 
    {     
        // FIXME: we don't need to create a texture on every frame!!
        double spf = (1 / thisclip->get_playhead_fps());
        if (move_playhead) // if it's time to move the playhead
            thisclip->iterate_playhead(); // updates the clip's playhead number
        int image_number = thisclip->get_playhead();
        bool pixels_are_loaded = false;
        width = thisclip->get_width(); // FIXME: do not override data given by gst-plugins-gl!
        height = thisclip->get_height();// FIXME: do not override data given by gst-plugins-gl!
        char *buf;
        GdkPixbuf *pixbuf;
        bool loaded_pixbuf = false;
        Image* thisimage = &thisclip->get_image(image_number);
        if (thisimage == NULL)
        {
            std::cout << "No image at index" << image_number << "." << std::endl;
        } else {
            /*FIXME: we may not need this dimension update in general. But I get a weirdly cropped frame, for the right side rendering grabbed frame. It seems
            the grabbed frame dimensions don't match with the width, height passed from glimasesink to the draw callback*/
            // XXX: yes, I think we should always check the size of the images. (especially when we will read them from the disk)

            if (thisimage->is_ready())
            {
                if (Application::get_instance().get_configuration().get_images_in_ram())
                {
                    buf = thisimage->get_rawdata();
                    pixels_are_loaded = true;
                } else {
                    std::string image_full_path = Application::get_instance().get_pipeline().get_image_full_path(thisimage);
                    // TODO: validate this path
                    GError *error = NULL;
                    
                    pixbuf = gdk_pixbuf_new_from_file(image_full_path.c_str(), &error);
                    if (!pixbuf)
                    {
                        std::cerr << "Failed to load pixbuf file: " << image_full_path << " " << error->message << std::endl;
                        g_error_free(error);
                    } else {
                        buf = (char*) gdk_pixbuf_get_pixels(pixbuf);
                        pixels_are_loaded = true;
                        loaded_pixbuf = true;
                    }
                }
            }
        }
        if (pixels_are_loaded)
        {
            // Storing image data in RAM is nice when we don't have too many images, but it doesn't scale very well.
            // Let's read them from the disk.
            
            glEnable(GL_TEXTURE_RECTANGLE_ARB);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, frametexture);
            glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
            // TODO: simplify those parameters
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            // Right image
            glPushMatrix();
            glTranslatef(0.6666666f, 0.0f, 0.0f);
            glScalef(0.6666666f, 0.5f, 1.0f);
            draw::draw_vertically_flipped_textured_square(width, height);
            glPopMatrix();

            if (loaded_pixbuf)
            {
                //std::cout << "Pixels loaded. Freeing pixels buffer.\n";
                g_object_unref(pixbuf);
                //std::cout << "Freed pixels buffer.\n";
            }
        }
    }
#if 0
    //disable shader
    glUseProgram(0); 
#endif

#if 0
    // DRAW LINES
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glColor4f(1.0, 1.0, 1.0, 0.2);
    //glColor4f(0.2, 0.2, 0.2, 1.0);
    int num = 64;
    float x;
    
    for (int i = 0; i < num; i++)
    {
        x = (i / float(num)) * 4 - 2;
        draw::draw_line(float(x), -2.0, float(x), 2.0);
        draw::draw_line(-2.0, float(x), 2.0, float(x));
    }
    glColor4f(1.0, 1.0, 1.0, 1.0); // I don't know why, but this is necessary if we want to see the images in the next rendering pass.
#endif
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
// TODO: we don't need this. Make it simpler.
void Pipeline::set_drawing_area(GtkWidget* drawing_area)
{
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline_));
    gst_bus_set_sync_handler(bus, (GstBusSyncHandler)create_window, drawing_area);
    gst_object_unref(bus);
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
    LOG_DEBUG("V4l2src caps are " << capsStr.str());
    ret = gst_element_set_state(videosrc_, GST_STATE_NULL);
    if (ret not_eq GST_STATE_CHANGE_SUCCESS)
        THROW_ERROR("Could not change v4l2src state to NULL");

    return capsStr.str();
}

