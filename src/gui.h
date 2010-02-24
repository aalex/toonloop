#ifndef __GUI_H__
#define __GUI_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <GL/glx.h>

class Gui
{
    public:
        GtkWidget* get_drawing_area();
        Gui(); 
        ~Gui() {};
        void toggleFullscreen() { toggleFullscreen(window_); } // no argument version of the same method below.

    private:
        GtkWidget *drawing_area_;
        GtkWidget *window_;
        GLXContext glx_context_;
        static void on_delete_event(GtkWidget* widget, GdkEvent* event, gpointer data);
        static gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);

        static int onWindowStateEvent(_GtkWidget *widget, _GdkEventWindowState *event, void *data);
        void toggleFullscreen(GtkWidget* widget);
        void makeFullscreen(GtkWidget* widget);
        void makeUnfullscreen(GtkWidget* widget);
        void hideCursor();
        void showCursor();
        bool isFullscreen_;
};

#endif // __GUI_H__
