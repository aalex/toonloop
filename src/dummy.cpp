/*
 * GStreamer
 * Copyright (C) 2008-2009 Julien Isorce <julien.isorce@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <GL/glew.h>

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <iostream>
#include "./gstgtk.h"

//client reshape callback
void reshapeCallback (GLuint width, GLuint height, gpointer data)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (gfloat)width/(gfloat)height, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
}

//client draw callback
gboolean drawCallback (GLuint texture, GLuint width, GLuint height, gpointer data)
{
    static GLfloat  xrot = 0;
    static GLfloat  yrot = 0;
    static GLfloat  zrot = 0;
    static GTimeVal current_time;
    static glong last_sec = current_time.tv_sec;
    static gint nbFrames = 0;

    g_get_current_time (&current_time);
    nbFrames++ ;

    if ((current_time.tv_sec - last_sec) >= 1)
    {
        std::cout << "GRPHIC FPS = " << nbFrames << std::endl;
        nbFrames = 0;
        last_sec = current_time.tv_sec;
    }

    glEnable(GL_DEPTH_TEST);

    glEnable (GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f,0.0f,-5.0f);

    glRotatef(xrot,1.0f,0.0f,0.0f);
    glRotatef(yrot,0.0f,1.0f,0.0f);
    glRotatef(zrot,0.0f,0.0f,1.0f);

    glBegin(GL_QUADS);
          // Front Face
          glTexCoord2f((gfloat)width, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
          glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
          glTexCoord2f(0.0f, (gfloat)height); glVertex3f( 1.0f,  1.0f,  1.0f);
          glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f(-1.0f,  1.0f,  1.0f);
          // Back Face
          glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
          glTexCoord2f(0.0f, (gfloat)height); glVertex3f(-1.0f,  1.0f, -1.0f);
          glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f( 1.0f,  1.0f, -1.0f);
          glTexCoord2f((gfloat)width, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
          // Top Face
          glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f(-1.0f,  1.0f, -1.0f);
          glTexCoord2f((gfloat)width, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
          glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
          glTexCoord2f(0.0f, (gfloat)height); glVertex3f( 1.0f,  1.0f, -1.0f);
          // Bottom Face
          glTexCoord2f((gfloat)width, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
          glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
          glTexCoord2f(0.0f, (gfloat)height); glVertex3f( 1.0f, -1.0f,  1.0f);
          glTexCoord2f((gfloat)width,(gfloat)height); glVertex3f(-1.0f, -1.0f,  1.0f);
          // Right face
          glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
          glTexCoord2f(0.0f, (gfloat)height); glVertex3f( 1.0f,  1.0f, -1.0f);
          glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f( 1.0f,  1.0f,  1.0f);
          glTexCoord2f((gfloat)width, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
          // Left Face
          glTexCoord2f((gfloat)width, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
          glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
          glTexCoord2f(0.0f, (gfloat)height); glVertex3f(-1.0f,  1.0f,  1.0f);
          glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();

    xrot+=0.3f;
    yrot+=0.2f;
    zrot+=0.4f;

    //return TRUE causes a postRedisplay
    return TRUE;
}


static GstBusSyncReply create_window (GstBus* bus, GstMessage* message, GtkWidget* widget)
{
    // ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;

    if (!gst_structure_has_name (message->structure, "prepare-xwindow-id"))
        return GST_BUS_PASS;

    g_print ("setting xwindow id\n");

    gst_x_overlay_set_gtk_window (GST_X_OVERLAY (GST_MESSAGE_SRC (message)), widget);

    gst_message_unref (message);

    return GST_BUS_DROP;
}


static void end_stream_cb(GstBus* bus, GstMessage* message, GstElement* pipeline)
{
    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR)
    {
        std::cout << "ERROR" << std::endl;
    } else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
        std::cout << "End of stream" << std::endl;
    } else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_WARNING) {
        std::cout << "WARNING" << std::endl;
        GError *gerror;
        gchar *debug;
        gst_message_parse_warning(message, &gerror, &debug);
        std::cout << gerror << std::endl;
        std::cout << debug << std::endl;
    }
    g_print("Stopping the stream\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gtk_main_quit();
}

static gboolean expose_cb(GtkWidget* widget, GdkEventExpose* event, GstElement* videosink)
{
    gst_x_overlay_expose (GST_X_OVERLAY (videosink));
    return FALSE;
}


static void destroy_cb(GtkWidget* widget, GdkEvent* event, GstElement* pipeline)
{
    g_print("Close\n");

    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    gtk_main_quit();
}


static void button_state_null_cb(GtkWidget* widget, GstElement* pipeline)
{
    gst_element_set_state (pipeline, GST_STATE_NULL);
    g_print ("GST_STATE_NULL\n");
}


static void button_state_ready_cb(GtkWidget* widget, GstElement* pipeline)
{
    gst_element_set_state (pipeline, GST_STATE_READY);
    g_print ("GST_STATE_READY\n");
}


static void button_state_paused_cb(GtkWidget* widget, GstElement* pipeline)
{
    gst_element_set_state (pipeline, GST_STATE_PAUSED);
    g_print ("GST_STATE_PAUSED\n");
}


static void button_state_playing_cb(GtkWidget* widget, GstElement* pipeline)
{
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_print ("GST_STATE_PLAYING\n");
}


static gchar* slider_fps_cb (GtkScale* scale, gdouble value, GstElement* pipeline)
{
    //change the video frame rate dynamically
    return g_strdup_printf ("video framerate: %0.*g", gtk_scale_get_digits (scale), value);
}



gint main (gint argc, gchar *argv[])
{
    gtk_init (&argc, &argv);
    gst_init (&argc, &argv);

    GstElement* pipeline = gst_pipeline_new ("pipeline");

    //window that contains an area where the video is drawn
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window, 640, 480);
    gtk_window_move (GTK_WINDOW (window), 300, 10);
    gtk_window_set_title (GTK_WINDOW (window), "glimagesink implement the gstxoverlay interface");
    GdkGeometry geometry;
    geometry.min_width = 1;
    geometry.min_height = 1;
    geometry.max_width = -1;
    geometry.max_height = -1;
    gtk_window_set_geometry_hints (GTK_WINDOW (window), window, &geometry, GDK_HINT_MIN_SIZE);

    //window to control the states
    GtkWidget* window_control = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    geometry.min_width = 1;
    geometry.min_height = 1;
    geometry.max_width = -1;
    geometry.max_height = -1;
    gtk_window_set_geometry_hints (GTK_WINDOW (window_control), window_control, &geometry, GDK_HINT_MIN_SIZE);
    gtk_window_set_resizable (GTK_WINDOW (window_control), FALSE);
    gtk_window_move (GTK_WINDOW (window_control), 10, 10);
    GtkWidget* table = gtk_table_new (2, 1, TRUE);
    gtk_container_add (GTK_CONTAINER (window_control), table);

    //control state null
    GtkWidget* button_state_null = gtk_button_new_with_label ("GST_STATE_NULL");
    g_signal_connect (G_OBJECT (button_state_null), "clicked",
        G_CALLBACK (button_state_null_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), button_state_null, 0, 1, 0, 1);
    gtk_widget_show (button_state_null);

    //control state ready
    GtkWidget* button_state_ready = gtk_button_new_with_label ("GST_STATE_READY");
    g_signal_connect (G_OBJECT (button_state_ready), "clicked",
        G_CALLBACK (button_state_ready_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), button_state_ready, 0, 1, 1, 2);
    gtk_widget_show (button_state_ready);

    //control state paused
    GtkWidget* button_state_paused = gtk_button_new_with_label ("GST_STATE_PAUSED");
    g_signal_connect (G_OBJECT (button_state_paused), "clicked",
        G_CALLBACK (button_state_paused_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), button_state_paused, 0, 1, 2, 3);
    gtk_widget_show (button_state_paused);

    //control state playing
    GtkWidget* button_state_playing = gtk_button_new_with_label ("GST_STATE_PLAYING");
    g_signal_connect (G_OBJECT (button_state_playing), "clicked",
        G_CALLBACK (button_state_playing_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), button_state_playing, 0, 1, 3, 4);
    gtk_widget_show (button_state_playing);

    //change framerate
    GtkWidget* slider_fps = gtk_vscale_new_with_range (1, 30, 2);
    g_signal_connect (G_OBJECT (slider_fps), "format-value",
        G_CALLBACK (slider_fps_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), slider_fps, 1, 2, 0, 4);
    gtk_widget_show (slider_fps);

    gtk_widget_show (table);
    gtk_widget_show (window_control);

    //configure the pipeline
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(destroy_cb), pipeline);

    GstElement* videosrc  = gst_element_factory_make ("videotestsrc", "videotestsrc0");
    GstElement* glupload  = gst_element_factory_make ("glupload", "gloupload0");
    GstElement* videosink = gst_element_factory_make ("glimagesink", "glimagesink0");

    if (!videosrc || !glupload || !videosink)
    {
        g_print ("one element could not be found \n");
        return -1;
    }

    //GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
    //                                    "width", G_TYPE_INT, 640,
    //                                    "height", G_TYPE_INT, 480,
    //                                    "framerate", GST_TYPE_FRACTION, 25, 1,
    //                                    "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'),
    //                                    NULL) ;
    /* change video source caps */
    GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb",
                                        "width", G_TYPE_INT, 320,
                                        "height", G_TYPE_INT, 240,
                                        "framerate", GST_TYPE_FRACTION, 25, 1,
                                        NULL) ;


    GstCaps *outcaps = gst_caps_new_simple("video/x-raw-gl",
                                        "width", G_TYPE_INT, 800,
                                        "height", G_TYPE_INT, 600,
                                        NULL) ;

    // configure elements
    g_object_set(G_OBJECT(videosrc), "num-buffers", 400, NULL);
    g_object_set(G_OBJECT(videosink), "client-reshape-callback", reshapeCallback, NULL);
    g_object_set(G_OBJECT(videosink), "client-draw-callback", drawCallback, NULL);
    //g_object_set(G_OBJECT(videosink), "client-data", NULL, NULL);

    // add elements
    gst_bin_add_many (GST_BIN (pipeline), videosrc, glupload, videosink, NULL);

    // link elements
    gboolean link_ok = gst_element_link_filtered(videosrc, glupload, caps) ;
    gst_caps_unref(caps) ;
    if(!link_ok)
    {
        g_warning("Failed to link videosrc to glupload!\n") ;
        return -1;
    }
    link_ok = gst_element_link_filtered(glupload, videosink, outcaps) ;
    gst_caps_unref(outcaps) ;
    if(!link_ok)
    {
        g_warning("Failed to link glupload to videosink!\n") ;
        return -1 ;
    }

    //area where the video is drawn
    GtkWidget* area = gtk_drawing_area_new();
    gtk_container_add (GTK_CONTAINER (window), area);

    //avoid flickering when resizing or obscuring the main window
    gtk_widget_realize(area);
    gdk_window_set_back_pixmap(area->window, NULL, FALSE);
    gtk_widget_set_app_paintable(area,TRUE);
    gtk_widget_set_double_buffered(area, FALSE);

    //set window id on this event
    GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window, area);
    gst_bus_add_signal_watch (bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), pipeline);
    g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), pipeline);
    g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), pipeline);
    gst_object_unref (bus);

    //needed when being in GST_STATE_READY, GST_STATE_PAUSED
    //or resizing/obscuring the window
    g_signal_connect(area, "expose-event", G_CALLBACK(expose_cb), videosink);

    //start
    GstStateChangeReturn ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_print ("Failed to start up pipeline!\n");
        return -1;
    }

    gtk_widget_show_all (window);

    gtk_main();

    return 0;
}

