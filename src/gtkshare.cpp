/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
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

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gtk/gtkgl.h>
#include <gst/gst.h>

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

/* rotation angle for the triangle. */
float rtri = 0.0f;

/* rotation angle for the quadrilateral. */
float rquad = 0.0f;

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL (int Width, int Height)  // We call this right after our OpenGL window is created.
{
  glViewport (0, 0, Width, Height);
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);        // This Will Clear The Background Color To Black
  glClearDepth (1.0);           // Enables Clearing Of The Depth Buffer
  glDepthFunc (GL_LESS);        // The Type Of Depth Test To Do
  glEnable (GL_DEPTH_TEST);     // Enables Depth Testing
  glShadeModel (GL_SMOOTH);     // Enables Smooth Color Shading

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();            // Reset The Projection Matrix

  gluPerspective (45.0f, (GLfloat) Width / (GLfloat) Height, 0.1f, 100.0f);     // Calculate The Aspect Ratio Of The Window

  glMatrixMode (GL_MODELVIEW);
}

/* The main drawing function. */
void DrawGLScene (GstGLBuffer * gst_gl_buf)
{
  GLuint texture = gst_gl_buf->texture;
  GLfloat width = (GLfloat) gst_gl_buf->width;
  GLfloat height = (GLfloat) gst_gl_buf->height;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear The Screen And The Depth Buffer
  glLoadIdentity ();            // Reset The View

  glTranslatef (-1.5f, 0.0f, -6.0f);    // Move Left 1.5 Units And Into The Screen 6.0

  glRotatef (rtri, 0.0f, 1.0f, 0.0f);   // Rotate The Triangle On The Y axis 
  // draw a triangle (in smooth coloring mode)
  glBegin (GL_POLYGON);         // start drawing a polygon
  glColor3f (1.0f, 0.0f, 0.0f); // Set The Color To Red
  glVertex3f (0.0f, 1.0f, 0.0f);        // Top
  glColor3f (0.0f, 1.0f, 0.0f); // Set The Color To Green
  glVertex3f (1.0f, -1.0f, 0.0f);       // Bottom Right
  glColor3f (0.0f, 0.0f, 1.0f); // Set The Color To Blue
  glVertex3f (-1.0f, -1.0f, 0.0f);      // Bottom Left  
  glEnd ();                     // we're done with the polygon (smooth color interpolation)

  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,
      GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
      GL_CLAMP_TO_EDGE);
  glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glLoadIdentity ();            // make sure we're no longer rotated.
  glTranslatef (1.5f, 0.0f, -6.0f);     // Move Right 3 Units, and back into the screen 6.0

  glRotatef (rquad, 1.0f, 0.0f, 0.0f);  // Rotate The Quad On The X axis 
  // draw a square (quadrilateral)
  glColor3f (0.5f, 0.5f, 1.0f); // set color to a blue shade.
  glBegin (GL_QUADS);           // start drawing a polygon (4 sided)
  glTexCoord3f (0.0f, height, 0.0f);
  glVertex3f (-1.0f, 1.0f, 0.0f);       // Top Left
  glTexCoord3f (width, height, 0.0f);
  glVertex3f (1.0f, 1.0f, 0.0f);        // Top Right
  glTexCoord3f (width, 0.0f, 0.0f);
  glVertex3f (1.0f, -1.0f, 0.0f);       // Bottom Right
  glTexCoord3f (0.0f, 0.0f, 0.0f);
  glVertex3f (-1.0f, -1.0f, 0.0f);      // Bottom Left  
  glEnd ();                     // done with the polygon

  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);

  rtri += 1.0f;                 // Increase The Rotation Variable For The Triangle
  rquad -= 1.0f;                // Decrease The Rotation Variable For The Quad 

  // swap buffers to display, since we're double buffered.
  SDL_GL_SwapBuffers ();
}

gboolean update_gl_scene (void *fk)
{
  GstElement *fakesink = (GstElement *) fk;
  GAsyncQueue *queue_input_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_input_buf");
  GAsyncQueue *queue_output_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_output_buf");
  GstGLBuffer *gst_gl_buf = (GstGLBuffer *) g_async_queue_pop (queue_input_buf);

  DrawGLScene (gst_gl_buf);

  /* push buffer so it can be unref later */
  g_async_queue_push (queue_output_buf, gst_gl_buf);

  return FALSE;
}


gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    switch (event->keyval)
    {
        case GDK_q:
            // Quit application on ctrl-q, this quits the main loop
            if (event->state & GDK_CONTROL_MASK)
            {
                g_print("Ctrl-Q key pressed, quitting.\n");
                gtk_main_quit();
            }
            break;
        default:
            break;
    }
    return TRUE;
}

/* fakesink handoff callback */
void on_gst_buffer (GstElement * fakesink, GstBuffer * buf, GstPad * pad,
    gpointer data)
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
    g_idle_add (update_gl_scene, (gpointer) fakesink);

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
void end_stream_cb (GstBus * bus, GstMessage * msg, gpointer data)
{
  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End-of-stream\n");
      g_print
          ("For more information, try to run: GST_DEBUG=gldisplay:2 ./sdlshare\n");
      break;

    case GST_MESSAGE_ERROR:
    {
      gchar *debug = NULL;
      GError *err = NULL;

      gst_message_parse_error (msg, &err, &debug);

      g_print ("Error: %s\n", err->message);
      g_error_free (err);

      if (debug) {
        g_print ("Debug deails: %s\n", debug);
        g_free (debug);
      }

      break;
    }

    default:
      break;
  }
  gtk_main_quit();
}

static void on_realize(GtkWidget *widget, gpointer data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  
    /*** OpenGL BEGIN ***/
    if (! gdk_gl_drawable_gl_begin(gldrawable, glcontext))
        return;
    
    InitGL (widget->allocation.width, widget->allocation.height);  
  
    gdk_gl_drawable_gl_end (gldrawable);
    /*** OpenGL END ***/
}

static gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
    /*** OpenGL BEGIN ***/
    if (! gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
        return FALSE;
    }
    InitGL (widget->allocation.width, widget->allocation.height);  
    
    gdk_gl_drawable_gl_end (gldrawable);
    /*** OpenGL END ***/
    return TRUE;
}


static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    /*** OpenGL BEGIN ***/
    if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
      return FALSE;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // FIXME: draw stuff here.
    glCallList(1);

    if (gdk_gl_drawable_is_double_buffered(gldrawable))
    {
        gdk_gl_drawable_swap_buffers(gldrawable);
    } else {
        glFlush();
    }
    gdk_gl_drawable_gl_end(gldrawable);
    /*** OpenGL END ***/
    return TRUE;
}

static void print_gl_config_attrib(GdkGLConfig *glconfig, const gchar *attrib_str, int attrib, gboolean is_boolean)
{
    int value;
    g_print ("%s = ", attrib_str);
    if (gdk_gl_config_get_attrib(glconfig, attrib, &value))
    {
        if (is_boolean)
        {
            g_print("%s\n", value == TRUE ? "TRUE" : "FALSE");
        } else {
            g_print("%d\n", value);
        } 
    } else {
        g_print("*** Cannot get %s attribute value\n", attrib_str);
    }
}

static void examine_gl_config_attrib(GdkGLConfig *glconfig)
{
  g_print("\nOpenGL visual configurations :\n\n");
  g_print("gdk_gl_config_is_rgba (glconfig) = %s\n", gdk_gl_config_is_rgba (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_is_double_buffered (glconfig) = %s\n", gdk_gl_config_is_double_buffered (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_is_stereo (glconfig) = %s\n", gdk_gl_config_is_stereo (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_has_alpha (glconfig) = %s\n", gdk_gl_config_has_alpha (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_has_depth_buffer (glconfig) = %s\n", gdk_gl_config_has_depth_buffer (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_has_stencil_buffer (glconfig) = %s\n", gdk_gl_config_has_stencil_buffer (glconfig) ? "TRUE" : "FALSE");
  g_print("gdk_gl_config_has_accum_buffer (glconfig) = %s\n",
  gdk_gl_config_has_accum_buffer (glconfig) ? "TRUE" : "FALSE");
  g_print ("\n");
  print_gl_config_attrib(glconfig, "GDK_GL_USE_GL", GDK_GL_USE_GL, TRUE);
  print_gl_config_attrib(glconfig, "GDK_GL_BUFFER_SIZE", GDK_GL_BUFFER_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_LEVEL", GDK_GL_LEVEL, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_RGBA", GDK_GL_RGBA, TRUE);
  print_gl_config_attrib(glconfig, "GDK_GL_DOUBLEBUFFER", GDK_GL_DOUBLEBUFFER, TRUE);
  print_gl_config_attrib(glconfig, "GDK_GL_STEREO", GDK_GL_STEREO, TRUE);
  print_gl_config_attrib(glconfig, "GDK_GL_AUX_BUFFERS", GDK_GL_AUX_BUFFERS, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_RED_SIZE", GDK_GL_RED_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_GREEN_SIZE", GDK_GL_GREEN_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_BLUE_SIZE", GDK_GL_BLUE_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ALPHA_SIZE", GDK_GL_ALPHA_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_DEPTH_SIZE", GDK_GL_DEPTH_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_STENCIL_SIZE", GDK_GL_STENCIL_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ACCUM_RED_SIZE", GDK_GL_ACCUM_RED_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ACCUM_GREEN_SIZE", GDK_GL_ACCUM_GREEN_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ACCUM_BLUE_SIZE", GDK_GL_ACCUM_BLUE_SIZE, FALSE);
  print_gl_config_attrib(glconfig, "GDK_GL_ACCUM_ALPHA_SIZE", GDK_GL_ACCUM_ALPHA_SIZE, FALSE);
  g_print ("\n");
}

static void on_delete_event(GtkWidget* widget, GdkEvent* event, void* data)
{
    g_print("Close\n");
    gtk_main_quit();
}

int main (int argc, char **argv)
{
  SDL_SysWMinfo info;
  Display *gtkglext_display = NULL;
  Window gtkglext_window = 0;
  GLXContext gtkglext_gl_context = NULL;
  
  GstPipeline *pipeline = NULL;
  GstBus *bus = NULL;
  GstElement *glfilter = NULL;
  GstElement *fakesink = NULL;
  GstState state;
  GAsyncQueue *queue_input_buf = NULL;
  GAsyncQueue *queue_output_buf = NULL;

  gst_init (&argc, &argv);
  gtk_init (&argc, &argv);
  gtk_gl_init(&argc, &argv);
    
  gint major; 
  gint minor;
  gdk_gl_query_version(&major, &minor);
  g_print("\nOpenGL extension version - %d.%d\n", major, minor);
  /* Try double-buffered visual */

  GdkGLConfig* glconfig;
  // the line above does not work in C++ if the cast is not there.
  glconfig = gdk_gl_config_new_by_mode(static_cast<GdkGLConfigMode>(GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE));
  if (glconfig == NULL)
  {
      g_print("*** Cannot find the double-buffered visual.\n");
      g_print("*** Trying single-buffered visual.\n");
      /* Try single-buffered visual */
      glconfig = gdk_gl_config_new_by_mode(static_cast<GdkGLConfigMode>(GDK_GL_MODE_RGB));
      if (glconfig == NULL)
      {
          g_print ("*** No appropriate OpenGL-capable visual found.\n");
          exit(1);
      }
  }
  examine_gl_config_attrib(glconfig);

  // Main GTK window
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request(window, 640, 480);
  gtk_window_set_title(GTK_WINDOW (window), "Toonloop 1.3 experimental");
  GdkGeometry geometry;
  geometry.min_width = 1;
  geometry.min_height = 1;
  geometry.max_width = -1;
  geometry.max_height = -1;
  gtk_window_set_geometry_hints(GTK_WINDOW(window), window, &geometry, GDK_HINT_MIN_SIZE);
  g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(on_delete_event), NULL);

  //area where the video is drawn
  GtkWidget* drawing_area = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), drawing_area);

  /* Set OpenGL-capability to the widget. */
  gtk_widget_set_gl_capability(drawing_area, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);

  /* Loop, drawing and checking events */
  g_signal_connect_after(G_OBJECT(drawing_area), "realize", G_CALLBACK(on_realize), NULL);
  g_signal_connect(G_OBJECT(drawing_area), "configure_event", G_CALLBACK(on_configure_event), NULL);
  g_signal_connect(G_OBJECT(drawing_area), "expose_event", G_CALLBACK(on_expose_event), NULL);

  gtk_widget_show_all(window);

 // ------------------ done with the GTK GUI


  /* retrieve and turn off gtkglext opengl context */
  SDL_VERSION (&info.version);
  SDL_GetWMInfo (&info);
  gtkglext_display = info.info.x11.display;
  gtkglext_window = info.info.x11.window;
  gtkglext_gl_context = glXGetCurrentContext ();
  glXMakeCurrent (gtkglext_display, None, 0);

  pipeline =
      GST_PIPELINE (gst_parse_launch
      ("videotestsrc ! video/x-raw-yuv, width=320, height=240, framerate=(fraction)30/1 ! "
          "glupload ! gleffects effect=5 ! fakesink sync=1", NULL));

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message::error", G_CALLBACK (end_stream_cb), NULL);
  g_signal_connect (bus, "message::warning", G_CALLBACK (end_stream_cb), NULL);
  g_signal_connect (bus, "message::eos", G_CALLBACK (end_stream_cb), NULL);
  gst_object_unref (bus);

  /* gtkglext_gl_context is an external OpenGL context with which gst-plugins-gl want to share textures */
  glfilter = gst_bin_get_by_name (GST_BIN (pipeline), "gleffects0");
  g_object_set (G_OBJECT (glfilter), "external-opengl-context",
      gtkglext_gl_context, NULL);
  g_object_unref (glfilter);

  /* NULL to PAUSED state pipeline to make sure the gst opengl context is created and
   * shared with the gtkglext one */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
  state = GST_STATE_PAUSED;
  if (gst_element_get_state (GST_ELEMENT (pipeline), &state, NULL,
          GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_SUCCESS) {
    g_debug ("failed to pause pipeline\n");
    return -1;
  }

  /* turn on back gtk opengl context */
  glXMakeCurrent (gtkglext_display, gtkglext_window, gtkglext_gl_context);

  /* append a gst-gl texture to this queue when you do not need it no more */
  fakesink = gst_bin_get_by_name (GST_BIN (pipeline), "fakesink0");
  g_object_set (G_OBJECT (fakesink), "signal-handoffs", TRUE, NULL);
  g_signal_connect (fakesink, "handoff", G_CALLBACK (on_gst_buffer), NULL);
  queue_input_buf = g_async_queue_new ();
  queue_output_buf = g_async_queue_new ();
  g_object_set_data (G_OBJECT (fakesink), "queue_input_buf", queue_input_buf);
  g_object_set_data (G_OBJECT (fakesink), "queue_output_buf", queue_output_buf);
  g_object_unref (fakesink);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
  
  gtk_main();

  /* before to deinitialize the gst-gl-opengl context,
   * no shared context (here the gtkglext one) must be current
   */
  glXMakeCurrent (gtkglext_display, gtkglext_window, gtkglext_gl_context);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
  g_object_unref (pipeline);

  /* turn on back gtkglext opengl context */
  glXMakeCurrent (gtkglext_display, None, 0);

  /* make sure there is no pending gst gl buffer in the communication queues 
   * between gtkglext and gst-gl
   */
  while (g_async_queue_length (queue_input_buf) > 0) {
    GstBuffer *buf = (GstBuffer *) g_async_queue_pop (queue_input_buf);
    gst_buffer_unref (buf);
  }

  while (g_async_queue_length (queue_output_buf) > 0) {
    GstBuffer *buf = (GstBuffer *) g_async_queue_pop (queue_output_buf);
    gst_buffer_unref (buf);
  }

  return 0;
}
