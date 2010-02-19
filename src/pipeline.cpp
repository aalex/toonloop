#include <gtk/gtk.h>
#include <GL/glx.h>
#include <gst/gst.h>

/**
 * GST bus signal watch callback 
 */
void Pipeline::end_stream_cb(GstBus* bus, GstMessage * msg, gpointer data)
{
    switch (GST_MESSAGE_TYPE(msg)) 
    {
        case GST_MESSAGE_EOS:
            g_print("End-of-stream\n");
            g_print("For more information, try to run it with GST_DEBUG=gldisplay:2\n");
            break;
        case GST_MESSAGE_ERROR:
        {
            gchar *debug = NULL;
            GError *err = NULL;
            gst_message_parse_error(msg, &err, &debug);
            g_print("Error: %s\n", err->message);
            g_error_free (err);
            if (debug) 
            {
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

Pipeline::Pipeline()
{
    pipeline_ = NULL;
    state_ = 0;
    
    
    pipeline_ = GST_PIPELINE(gst_parse_launch("videotestsrc ! video/x-raw-yuv, width=320, height=240, framerate=(fraction)30/1 ! glupload ! fakesink sync=1", NULL));
    /* setup bus */
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), this);
    g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), this);
    gst_object_unref(bus);
  
}

/**
 * Sets is an external OpenGL context with which gst-plugins-gl want to share textures 
 */
void Pipeline::set_glx_context(GLXContext gl_context)
{
    static bool called = false;
    GstElement* glupload0 = NULL;
    GstBus* bus = NULL;
    
    if (! called)
    {
        called = true;
    
        glupload0 = gst_bin_get_by_name(GST_BIN(pipeline_), "glupload0");
        g_object_set(G_OBJECT(glupload0), "external-opengl-context", gl_context, NULL);
        g_object_unref(glupload0);
        
        /* NULL to PAUSED state pipeline to make sure the gst opengl context is created and
         * shared with the other one 
         */
        gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_PAUSED);
        state_ = GST_STATE_PAUSED;
        if (gst_element_get_state(GST_ELEMENT(pipeline_), &state_, NULL, GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_SUCCESS) 
        {
            g_debug("failed to pause pipeline\n");
            exit(1);
        }
        // set a callback to retrieve the gst gl textures
        fakesink = gst_bin_get_by_name(GST_BIN(pipeline_), "fakesink0");
        g_object_set(G_OBJECT(fakesink), "signal-handoffs", TRUE, NULL);
        g_signal_connect(fakesink, "handoff", G_CALLBACK(on_gst_buffer), this);
        g_object_unref(fakesink);

        /* play gst */
        gst_element_set_state (GST_ELEMENT (pipeline_), GST_STATE_PLAYING);
    }
}

/**
 * Called everytime there is a new video image ready to texture with.
 * fakesink handoff callback.
 *
 * @param element: The fakesink0 element.
 * @param buf: The pixels.
 * @param data: The this pointer.
 */
static void on_gst_buffer(GstElement* element, GstBuffer* buf, GstPad * pad, gpointer data)
{
    GAsyncQueue *queue_input_buf = NULL;
    GAsyncQueue *queue_output_buf = NULL;
    Pipeline *context = static_cast<Pipeline*>(data);
  
    // ref then push buffer to use it in clutter
    gst_buffer_ref(buf);
    queue_input_buf = g_object_get_data(G_OBJECT(texture_actor), "queue_input_buf");
    g_async_queue_push(queue_input_buf, buf);
    if (g_async_queue_length(queue_input_buf) > 2) 
    {
        clutter_threads_add_idle_full(G_PRIORITY_HIGH, update_texture_with_data, texture_actor, NULL);
    }
  
    /* pop then unref buffer we have finished to use in clutter */
    queue_output_buf = g_object_get_data(G_OBJECT(texture_actor), "queue_output_buf");
    if (g_async_queue_length(queue_output_buf) > 2) 
    {
      GstBuffer *buf_old = g_async_queue_pop(queue_output_buf);
      gst_buffer_unref(buf_old);
    }
}

/* 
 * Updates an OpenGL texture with the data from the GST gl buffer
 */
gboolean update_texture_with_data(gpointer data)
{
    ClutterTexture *texture_actor = (ClutterTexture *) data;
    GAsyncQueue *queue_input_buf = g_object_get_data(G_OBJECT (texture_actor), "queue_input_buf");
    GAsyncQueue *queue_output_buf = g_object_get_data(G_OBJECT (texture_actor), "queue_output_buf");
    GstGLBuffer *gst_gl_buf = g_async_queue_pop(queue_input_buf);
    ClutterActor *stage = g_object_get_data(G_OBJECT (texture_actor), "stage");
    CoglHandle cogl_texture = 0;
  
    /* Create a cogl texture from the gst gl texture */
    glEnable (GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, gst_gl_buf->texture);
    if (glGetError () != GL_NO_ERROR) 
    {
        g_debug ("failed to bind texture that comes from gst-gl\n");
    }  
    cogl_texture = cogl_texture_new_from_foreign (gst_gl_buf->texture,
        CGL_TEXTURE_RECTANGLE_ARB, gst_gl_buf->width, gst_gl_buf->height, 0, 0,
        COGL_PIXEL_FORMAT_RGBA_8888);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
  
    /* Previous cogl texture is replaced and so its ref counter discreases to 0.
     * According to the source code, glDeleteTexture is not called when the previous
     * ref counter of the previous cogl texture is reaching 0 because is_foreign is TRUE */
    clutter_texture_set_cogl_texture (CLUTTER_TEXTURE (texture_actor), cogl_texture);
    cogl_texture_unref(cogl_texture);
  
    /* we can now show the clutter scene if not yet visible */
    if (!CLUTTER_ACTOR_IS_VISIBLE(stage))
    {
        clutter_actor_show_all(stage);
    }
  
    /* push buffer so it can be unref later */
    g_async_queue_push (queue_output_buf, gst_gl_buf);
    return FALSE;
}

Pipeline::~Pipeline() {}

