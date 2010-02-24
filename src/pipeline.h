#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <GL/glx.h>

class Pipeline
{
    public:
        void stop();
        void set_drawing_area(GtkWidget *drawing_area);
        Pipeline();
        ~Pipeline();
    private:
        GstElement* videosrc_;
        GstElement* videosink_;
        GstPipeline* pipeline_;
        GstState state_;
        static void end_stream_cb(GstBus* bus, GstMessage* msg, GstElement* pipeline);
};

static GstBusSyncReply create_window(GstBus* bus, GstMessage* message, GtkWidget* widget);
static gboolean on_expose_event(GtkWidget* widget, GdkEventExpose* event, GstElement* videosink);
void reshapeCallback(GLuint width, GLuint height, gpointer data);
gboolean drawCallback(GLuint texture, GLuint width, GLuint height, gpointer data);

#endif // __PIPELINE_H__
