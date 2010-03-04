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
 *
 * This file is heavily inspired from an example coming with 
 * the gst-plugins-gl, published under the LGPL:
 *
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

// Try to launch with:
//  --gst-debug=v4l2src:4

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#ifndef WIN32
#include <GL/glx.h>
// #include <X11/Xlib.h>
#include "SDL/SDL_syswm.h"
#endif

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <iostream>
#include <cstdlib> // for getenv

/* This is our SDL surface */
SDL_Surface *surface;
int videoFlags;;
int x11_screen_width = 0;
int x11_screen_height = 0;
GstElement* gdkpixbufsink0 = NULL; // FIXME

/* hack */
typedef struct _GstGLBuffer GstGLBuffer;
struct _GstGLBuffer
{
  GstBuffer buffer;

  GObject *obj;

  gint width;
  gint height;
  GLuint texture;
};

/* rotation angle */
float zrot = 0.0f;

/**
 * Draws a line between given points.
 */
void draw_line(float from_x, float from_y, float to_x, float to_y)
{
    glBegin(GL_LINES);
    glVertex2f(from_x, from_y);
    glVertex2f(to_x, to_y);
    glEnd();
}
/**
 * Draws a texture square of 2 x 2 size centered at 0, 0
 * 
 * Make sure to call glEnable(GL_TEXTURE_RECTANGLE_ARB) first.
 * 
 * @param width: width of the image in pixels
 * @param height: height of the image in pixels
 */
void draw_vertically_flipped_textured_square(float width, float height)
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, height);
    glVertex2f(-1.0, -1.0); // Bottom Left
    glTexCoord2f(width, height);
    glVertex2f(1.0, -1.0); // Bottom Right
    glTexCoord2f(width, 0.0);
    glVertex2f(1.0, 1.0); // Top Right
    glTexCoord2f(0.0, 0.0);
    glVertex2f(-1.0, 1.0); // Top Left
    glEnd();
}

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void init_opengl_scene()   // We call this right after our OpenGL window is created.
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // This Will Clear The Background Color To Black
    glShadeModel(GL_SMOOTH);     // Enables Smooth Color Shading
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}

void resize_rendering_area(int width, int height)
{
    // gl coords are 1.0 of height
    float w = float(width) / float(height);
    float h = 1.0;
    g_print("Resize to %d %d\n", width, height);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w, w, -h, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

/* The main drawing function. */
void DrawGLScene (GstGLBuffer * gst_gl_buf)
{
    GLuint texture = gst_gl_buf->texture;
    GLfloat width = (GLfloat) gst_gl_buf->width;
    GLfloat height = (GLfloat) gst_gl_buf->height;

    glClear(GL_COLOR_BUFFER_BIT);  // Clear The Screen (and no need to clean the depth buffer)
    glLoadIdentity();

    static GLfloat  zrot = 0;
    static GTimeVal current_time;
    static glong last_sec = current_time.tv_sec;
    static gint nbFrames = 0;

    g_get_current_time(&current_time);
    nbFrames++ ;

    if ((current_time.tv_sec - last_sec) >= 1)
    {
        std::cout << "GRAPHIC FPS = " << nbFrames << std::endl;
        nbFrames = 0;
        last_sec = current_time.tv_sec;
    }
    // draw the video on a rectangle
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    glPushMatrix();
    glTranslatef(0.66666666f,0.0f,0.0f);
    glRotatef(zrot,0.0f,0.0f,1.0f);
    glScalef(0.66666666666, 0.5, 1.0);
    draw_vertically_flipped_textured_square(width, height);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.6666666f,0.0f,0.0f);
    glRotatef(zrot,0.0f,0.0f,1.0f);
    glScalef(0.66666666666, 0.5, 1.0);
    draw_vertically_flipped_textured_square(width, height);
    glPopMatrix();
    
    zrot += 0.1f;

    // DRAW LINES
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glColor4f(0.2, 0.2, 0.2, 0.2);
    int num = 64;
    float x;
    
    for (int i = 0; i < num; i++)
    {
        x = (i / float(num)) * 4 - 2;
        draw_line(float(x), -2.0, float(x), 2.0);
        draw_line(-2.0, float(x), 2.0, float(x));
    }

  // swap buffers to display, since we're double buffered.
  SDL_GL_SwapBuffers();
}


// FIXME: use a class and encapsulate global variables !
void add_frame() 
{
    g_print("add_frame\n");
    GdkPixbuf* pixbuf; 
    gpointer data;
    g_object_get(G_OBJECT(gdkpixbufsink0), "last-pixbuf", &pixbuf, NULL);
    g_print("got the pibuf!\n");
    int w = gdk_pixbuf_get_width(pixbuf);
    int h = gdk_pixbuf_get_height(pixbuf);

    g_print("grabbing size: %dx%d\n", w, h);
    //char* file_name = ;
    //TODO:GError* error = NULL;
    //TODO: pass &error
    if (!gdk_pixbuf_save(pixbuf, "/var/tmp/gdkpixbuf-saved.jpg", "jpeg", NULL, "quality", "100", NULL))
    {
        g_print("Image could not be saved. Error\n");
        // TODO : print error message.
    }
    //TODO:g_object_unref(error);
    g_object_unref(pixbuf);
}

gboolean update_sdl_scene(void *fk)
{
  GstElement *fakesink = (GstElement *) fk;
  GMainLoop *loop =
      (GMainLoop *) g_object_get_data (G_OBJECT (fakesink), "loop");
  GAsyncQueue *queue_input_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_input_buf");
  GAsyncQueue *queue_output_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_output_buf");
  GstGLBuffer *gst_gl_buf = (GstGLBuffer *) g_async_queue_pop (queue_input_buf);

  SDL_Event event;
  while (SDL_PollEvent (&event)) {
    if (event.type == SDL_QUIT) {
      g_main_loop_quit (loop);
    }
    else if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_q) {
        if ((event.key.keysym.mod & KMOD_LCTRL) or (event.key.keysym.mod & KMOD_RCTRL)) 
        {
          g_main_loop_quit (loop);
        }
      }
      else if (event.key.keysym.sym == SDLK_ESCAPE) {
        /* F1 key was pressed
         * this toggles fullscreen mode
         */
        //SDL_WM_ToggleFullScreen(surface);
        videoFlags ^= SDL_FULLSCREEN; // toggles it
        if ((videoFlags & SDL_FULLSCREEN) != 0)
        {
            g_print("Fullscreen ON \n");
            int w = 0;
            int h = 0;
#ifndef WIN32
            w = x11_screen_width;
            h = x11_screen_height;
#endif
            g_print("using a size of %d x %d\n", w, h);
            surface = SDL_SetVideoMode(w, h, 0, videoFlags);
            if(surface != NULL) {
                resize_rendering_area(w, h);
                SDL_ShowCursor(SDL_DISABLE);
            }
        } else{
            g_print("Fullscreen OFF \n");
            SDL_ShowCursor(SDL_ENABLE);
            surface = SDL_SetVideoMode(640, 480, 0, videoFlags);
            if(surface != NULL) {
                resize_rendering_area(640, 480);
            }
        }
        if(surface == NULL) {
            g_print("Surface is NULL. Back to what it was.\n");
            videoFlags ^= SDL_FULLSCREEN; // toggles it
            surface = SDL_SetVideoMode(0, 0, 0, videoFlags); /* If toggle FullScreen failed, then switch back */
            if(surface == NULL) { 
                g_print("Could not recreate a surface full screen.\n");
                exit(1);
            }
        }
      }
      else if (event.key.keysym.sym == SDLK_SPACE) {
          add_frame();
      }
    }
    else if (event.type == SDL_VIDEORESIZE) {
      /* handle resize event */
      g_print("SDL_VIDEORESIZE\n");
      surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 0, videoFlags );
      if (!surface) {
        fprintf(stderr, "Could not get a surface after resize: %s\n", SDL_GetError());
        g_main_loop_quit (loop);
      }
      resize_rendering_area( event.resize.w, event.resize.h );
    }
  }

  DrawGLScene (gst_gl_buf);

  /* push buffer so it can be unref later */
  g_async_queue_push (queue_output_buf, gst_gl_buf);

  return FALSE;
}

/* fakesink handoff callback */
void on_gst_buffer (GstElement * fakesink, GstBuffer * buf, GstPad * pad, gpointer data)
{
  GAsyncQueue *queue_input_buf = NULL;
  GAsyncQueue *queue_output_buf = NULL;

  /* ref then push buffer to use it in sdl */
  gst_buffer_ref (buf);
  queue_input_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_input_buf");
  g_async_queue_push (queue_input_buf, buf);
  if (g_async_queue_length (queue_input_buf) > 3)
    g_idle_add (update_sdl_scene, (gpointer) fakesink);

  /* pop then unref buffer we have finished to use in sdl */
  queue_output_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_output_buf");
  if (g_async_queue_length (queue_output_buf) > 3) {
    GstBuffer *buf_old = (GstBuffer *) g_async_queue_pop (queue_output_buf);
    gst_buffer_unref (buf_old);
  }
}

/* gst bus signal watch callback */
void end_stream_cb (GstBus * bus, GstMessage * msg, GMainLoop * loop)
{
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print("End-of-stream\n");
      g_print("For more information, try to run: GST_DEBUG=gldisplay:2 ./sdlshare\n");
      break;
    case GST_MESSAGE_ERROR:
    {
      gchar *debug = NULL;
      GError *err = NULL;
      gst_message_parse_error(msg, &err, &debug);
      g_print("Error: %s\n", err->message);
      g_error_free(err);
      if (debug) 
      {
        g_print("Debug deails: %s\n", debug);
        g_free(debug);
      }
      break;
    }
    default:
      break;
  }
  g_main_loop_quit (loop);
}


int main (int argc, char **argv)
{
  std::cout << "DISPLAY=" << std::getenv("DISPLAY") << std::endl;

#ifdef WIN32
  HGLRC sdl_gl_context = 0;
  HDC sdl_dc = 0;
#else
  SDL_SysWMinfo info;
  Display *sdl_display = NULL;
  Window sdl_win = 0;
  GLXContext sdl_gl_context = NULL;
#endif

  GMainLoop *loop = NULL;
  GstPipeline *pipeline = NULL;
  GstBus *bus = NULL;
  //GstElement *glfilter = NULL;
  GstElement *fakesink = NULL;
  GstState state;
  GAsyncQueue *queue_input_buf = NULL;
  GAsyncQueue *queue_output_buf = NULL;

  /* Initialize SDL for video output */
  if (SDL_Init (SDL_INIT_VIDEO) < 0) {
    fprintf (stderr, "Unable to initialize SDL: %s\n", SDL_GetError ());
    return -1;
  }
  videoFlags = SDL_OPENGL | SDL_RESIZABLE;
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  /* Create a 640x480 OpenGL screen */
  surface = SDL_SetVideoMode(640, 480, 0, videoFlags);
  if (surface == NULL) {
    fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError ());
    SDL_Quit();
    return -1;
  }

  // Get the current video hardware information
  //const SDL_VideoInfo* sdl_video_info = SDL_GetVideoInfo();
  //std::cout << "Current video resolution is " << sdl_video_info->current_w << "x" << sdl_video_info->current_h << " pixels" << std::endl;

  /* Set the title bar in environments that support it */
  SDL_WM_SetCaption("Toonloop 1.3 SDL Prototype", NULL);

  /* Loop, drawing and checking events */
  init_opengl_scene();
  resize_rendering_area(640, 480);

  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* retrieve and turn off sdl opengl context */
#ifdef WIN32
  sdl_gl_context = wglGetCurrentContext ();
  sdl_dc = wglGetCurrentDC ();
  wglMakeCurrent (0, 0);
#else
  SDL_VERSION (&info.version);
  SDL_GetWMInfo (&info);
  sdl_display = info.info.x11.display;
  sdl_win = info.info.x11.window;
  sdl_gl_context = glXGetCurrentContext ();
  glXMakeCurrent (sdl_display, None, 0);
#endif

#ifndef WIN32
  // XXX Linux only:
  std::cout << "display infos:" << std::endl;
  int scr = 0;
  for (int i = 0; i < ScreenCount(sdl_display); i++) 
  {
    x11_screen_width = XDisplayWidth(sdl_display, i);
    x11_screen_height = XDisplayHeight(sdl_display, i);
    g_print("  screen #%d  dimensions:    %dx%d pixels \n", i, x11_screen_width, x11_screen_height);
  }
  int default_screen_num = DefaultScreen(sdl_display);
  x11_screen_width = XDisplayWidth(sdl_display, default_screen_num);
  x11_screen_height = XDisplayHeight(sdl_display, default_screen_num);
  g_print("Using  screen #%d  dimensions:    %dx%d pixels \n", default_screen_num, x11_screen_width, x11_screen_height);

#endif

  // Gstreamer Pipeline:
  // gst-launch v4l2src device=/dev/video0 ! video/x-raw-yuv,format=\(fourcc\)UYVY,width=640,height=480 ! ffmpegcolorspace ! xvimagesink
  // video source:
  // See also gtkmedia.c in pidgin for a nice auto-detected video source.
  // See also cheese.
  pipeline = GST_PIPELINE(gst_pipeline_new("pipeline0"));
  g_assert(pipeline);
  GstElement* videosrc0  = gst_element_factory_make("v4l2src", "videosrc0");
  g_assert(videosrc0);
  // caps filter element:
  GstElement* capsfilter0 = gst_element_factory_make("capsfilter", "capsfilter0");
  g_assert(capsfilter0);
  GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
      "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC('U','Y','V','Y'),
      "width", G_TYPE_INT, 640,
      "height", G_TYPE_INT, 480,
      //"framerate", GST_TYPE_FRACTION, 30, 1,
      NULL); 
  g_object_set(capsfilter0, "caps", caps, NULL);
  gst_caps_unref(caps);
  // ffmpegcolorspace0 element:
  GstElement* ffmpegcolorspace0 = gst_element_factory_make("ffmpegcolorspace", "ffmpegcolorspace0");
  g_assert(ffmpegcolorspace0);
  GstElement* tee0 = gst_element_factory_make("tee", "tee0");
  g_assert(tee0);
  GstElement* queue0 = gst_element_factory_make("queue", "queue0");
  g_assert(queue0);

  // glupload0 element:
  GstElement* glupload0 = gst_element_factory_make("glupload", "glupload0");
  g_assert(glupload0);
  GstElement* fakesink0 = gst_element_factory_make("fakesink", "fakesink0");
  g_assert(fakesink0);
  // GdkPixbuf sink:
  GstElement* queue1 = gst_element_factory_make("queue", "queue1");
  g_assert(queue1);
  //FIXME: GstElement* 
  gdkpixbufsink0 = gst_element_factory_make("gdkpixbufsink", "gdkpixbufsink0");
  g_assert(gdkpixbufsink0);

  // add elements
  gst_bin_add(GST_BIN(pipeline), videosrc0);
  gst_bin_add(GST_BIN(pipeline), capsfilter0);
  gst_bin_add(GST_BIN(pipeline), ffmpegcolorspace0);
  gst_bin_add(GST_BIN(pipeline), tee0);
  gst_bin_add(GST_BIN(pipeline), queue0);
  gst_bin_add(GST_BIN(pipeline), glupload0);
  gst_bin_add(GST_BIN(pipeline), fakesink0);
  gst_bin_add(GST_BIN(pipeline), queue1);
  gst_bin_add(GST_BIN(pipeline), gdkpixbufsink0);
  // link pads:
  gboolean is_linked = NULL;
  is_linked = gst_element_link_pads(videosrc0, "src", capsfilter0, "sink");
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
  is_linked = gst_element_link_pads(glupload0, "src", fakesink0, "sink");
  if (!is_linked) { g_print("Could not link %s to %s.\n", "glupload0", "fakesink0"); exit(1); }

  // output 1: the GdkPixbuf sink
  is_linked = gst_element_link_pads(tee0, "src1", queue1, "sink");
  if (!is_linked) { g_print("Could not link %s to %s.\n", "tee0", "queue1"); exit(1); }
  is_linked = gst_element_link_pads(queue1, "src", gdkpixbufsink0, "sink");
  if (!is_linked) { g_print("Could not link %s to %s.\n", "queue1", "gdkpixbufsink0"); exit(1); }

  g_object_set(fakesink0, "sync", FALSE, NULL); 
  //TODO: 
  char* device_name = "/dev/video0";
  g_print("Using camera %s.\n", device_name);
  g_object_set(videosrc0, "device", device_name, NULL); 
  /* sdl_gl_context is an external OpenGL context with which gst-plugins-gl want to share textures */
  g_object_set(G_OBJECT(glupload0), "external-opengl-context", sdl_gl_context, NULL);

  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_add_signal_watch(bus);
  g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), loop);
  g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), loop);
  g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), loop);
  gst_object_unref(bus);

  /* NULL to PAUSED state pipeline to make sure the gst opengl context is created and
   * shared with the sdl one */
  gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);
  // FIXME: cannot set it to pause right now.
  state = GST_STATE_PAUSED;
  if (gst_element_get_state(GST_ELEMENT(pipeline), &state, NULL, GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_SUCCESS) 
  {
    g_debug("failed to pause pipeline\n");
    //return -1;
  }

  /* turn on back sdl opengl context */
#ifdef WIN32
  wglMakeCurrent(sdl_dc, sdl_gl_context);
#else
  glXMakeCurrent(sdl_display, sdl_win, sdl_gl_context);
#endif

  /* append a gst-gl texture to this queue when you do not need it no more */
  fakesink = gst_bin_get_by_name(GST_BIN(pipeline), "fakesink0");
  g_object_set(G_OBJECT(fakesink), "signal-handoffs", TRUE, NULL);
  g_signal_connect(fakesink, "handoff", G_CALLBACK(on_gst_buffer), NULL);
  queue_input_buf = g_async_queue_new();
  queue_output_buf = g_async_queue_new();
  g_object_set_data(G_OBJECT(fakesink), "queue_input_buf", queue_input_buf);
  g_object_set_data(G_OBJECT(fakesink), "queue_output_buf", queue_output_buf);
  g_object_set_data(G_OBJECT(fakesink), "loop", loop);
  g_object_unref(fakesink);

  gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

  g_main_loop_run(loop);

  /* before to deinitialize the gst-gl-opengl context,
   * no shared context (here the sdl one) must be current
   */
#ifdef WIN32
  wglMakeCurrent(0, 0);
#else
  glXMakeCurrent(sdl_display, sdl_win, sdl_gl_context);
#endif

  gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
  g_object_unref(pipeline);

  /* turn on back sdl opengl context */
#ifdef WIN32
  wglMakeCurrent(sdl_dc, sdl_gl_context);
#else
  glXMakeCurrent(sdl_display, None, 0);
#endif

  SDL_Quit ();

  /* make sure there is no pending gst gl buffer in the communication queues 
   * between sdl and gst-gl
   */
  while (g_async_queue_length(queue_input_buf) > 0) {
    GstBuffer *buf = (GstBuffer *) g_async_queue_pop(queue_input_buf);
    gst_buffer_unref(buf);
  }

  while (g_async_queue_length(queue_output_buf) > 0) {
    GstBuffer *buf = (GstBuffer *) g_async_queue_pop(queue_output_buf);
    gst_buffer_unref(buf);
  }

  return 0;
}
