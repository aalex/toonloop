#include <gtk/gtk.h>
#include <GL/glx.h>
#include <gst/gst.h>
#include <gdk/gdk.h>
#include <iostream>
#include "gstgtk.h"
#include "pipeline.h"
#include "draw.h"
#include "application.h"

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
    GstElement* glupload;
    
    // videotestsrc element
    videosrc_  = gst_element_factory_make("videotestsrc", "videotestsrc0");
    // capsfilter element #0
        // Possible values for the capture FPS:
        // 25/1 
        // 30000/1001
        // 30/1
    GstElement* capsfilter0 = gst_element_factory_make ("capsfilter", NULL);
    GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb",
                                        "width", G_TYPE_INT, 640,
                                        "height", G_TYPE_INT, 480,
                                        "framerate", GST_TYPE_FRACTION, 30000, 1001,
                                        NULL); 
    g_object_set(capsfilter0, "caps", caps, NULL);
    gst_caps_unref(caps);
    // glupload element
    glupload  = gst_element_factory_make ("glupload", "glupload0");
    // glimagesink
    videosink_ = gst_element_factory_make("glimagesink", "glimagesink0");
    g_object_set(videosink_, "sync", FALSE, NULL);
    g_object_set(G_OBJECT(videosink_), "client-reshape-callback", reshapeCallback, NULL);
    g_object_set(G_OBJECT(videosink_), "client-draw-callback", drawCallback, NULL);
    // capsfilter element #1
    GstElement* capsfilter1 = gst_element_factory_make ("capsfilter", NULL);
    GstCaps *outcaps = gst_caps_new_simple("video/x-raw-gl",
                                        "width", G_TYPE_INT, 800,
                                        "height", G_TYPE_INT, 600,
                                        NULL) ;
    g_object_set(capsfilter1, "caps", outcaps, NULL);
    gst_caps_unref(outcaps);

    // add elements
    if (!videosrc_ or !capsfilter0 or !glupload or !capsfilter1 or !videosink_)
    {
        g_print("one element could not be found \n");
        exit(1);
    }
    gst_bin_add_many(GST_BIN(pipeline_), videosrc_, capsfilter0, glupload, capsfilter1, videosink_, NULL);
    gboolean linked_ok = gst_element_link_many(videosrc_, capsfilter0, glupload, capsfilter1, videosink_, NULL);
    if (!linked_ok)
    {
        g_print("Could not link the elements\n.");
        exit(1);
    }
    /* setup bus */
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), this);
    gst_object_unref(bus);

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

    glBegin(GL_QUADS);
    // Front Face
    glTexCoord2f((gfloat)width, 0.0f); 
    glVertex3f(-0.666f, -0.5f,  0.0f);
    glTexCoord2f(0.0f, 0.0f); 
    glVertex3f( 0.666f, -0.5f,  0.0f);
    glTexCoord2f(0.0f, (gfloat)height); 
    glVertex3f( 0.666f,  0.5f,  0.0f);
    glTexCoord2f((gfloat)width, (gfloat)height); 
    glVertex3f(-0.666f,  0.5f,  0.0f);
    glEnd();

    zrot+=0.001f;
    glPopMatrix();

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
    // ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;
    if (!gst_structure_has_name(message->structure, "prepare-xwindow-id"))
        return GST_BUS_PASS;
    g_print("setting xwindow id\n");
    gst_x_overlay_set_gtk_window(GST_X_OVERLAY(GST_MESSAGE_SRC(message)), widget);
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

