#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <gst/gst.h>
#include <GL/glx.h>

class Pipeline
{
    private:
        GstPipeline* pipeline_;
        GstState state_;
        static void end_stream_cb(GstBug* bus, GstMessage* msg, gpointer data);
        static void on_gst_buffer(GstElement* element, GstBuffer* buf, GstPad * pad, gpointer context)
    public:
        Pipeline();
        ~Pipeline();
        void set_glx_context(GLXContext context);
};

#endif // __PIPELINE_H__
