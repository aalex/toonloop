#include <GL/glew.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtk/gtkgl.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gdk/gdkkeysyms.h>

#include "gltools.h"
#include "draw.h"

class Gui
{
    public:
        Gui(); // TODO: add pipeline argument
        ~Gui() {};
        void toggleFullscreen() { toggleFullscreen(window_); } // no argument version of the same method below.

    private:
        GtkWidget *window_;
        GtkWidget *drawing_area_;
        static void on_realize(GtkWidget *widget, gpointer data);
        static gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
        static gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
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

gboolean Gui::onWindowStateEvent(GtkWidget* widget, GdkEventWindowState *event, gpointer data)
{
    Gui *context = static_cast<Gui*>(data);
    context->isFullscreen_ = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    if (context->isFullscreen_)
        context->hideCursor();
    else
        context->showCursor();
    return TRUE;
}

void Gui::hideCursor()
{
    // FIXME: this is because gtk doesn't support GDK_BLANK_CURSOR before gtk-2.16
    char invisible_cursor_bits[] = { 0x0 };
    static GdkCursor* cursor = 0;
    if (cursor == 0)
    {
        static GdkBitmap *empty_bitmap;
        const static GdkColor color = {0, 0, 0, 0};
        empty_bitmap = gdk_bitmap_create_from_data(GDK_WINDOW(drawing_area_->window), invisible_cursor_bits, 1, 1);
        cursor = gdk_cursor_new_from_pixmap(empty_bitmap, empty_bitmap, &color, &color, 0, 0);
    }
    gdk_window_set_cursor(GDK_WINDOW(drawing_area_->window), cursor);
}

void Gui::showCursor()
{
    /// sets to default
    gdk_window_set_cursor(GDK_WINDOW(drawing_area_->window), NULL);
}


/**
 * Sets up the orthographic projection.
 * 
 * Makes sure height is always 1.0 in GL modelview coordinates.
 * 
 * Coordinates should give a rendering area height of 1
 * and a width of 1.33, when in 4:3 ratio.
*/
void _set_view(float ratio)
{
    float w = ratio;
    float h = 1.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w, w, -h, h, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Gui::on_realize(GtkWidget *widget, gpointer data)
{

    Gui *context = static_cast<Gui*>(data);

    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  
    /*** OpenGL BEGIN ***/
    if (! gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
        return;
    }
    glDisable(GL_DEPTH_TEST);
  
    glClearColor(0.0, 0.0, 0.0, 1.0); // black background
    glViewport(0, 0, widget->allocation.width, widget->allocation.height);
    _set_view(widget->allocation.width / float(widget->allocation.height));
    gdk_gl_drawable_gl_end(gldrawable);
    /*** OpenGL END ***/
}

gboolean Gui::on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    Gui *context = static_cast<Gui*>(data);
    GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
    /*** OpenGL BEGIN ***/
    if (! gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
        return FALSE;
    }
    glViewport(0, 0, widget->allocation.width, widget->allocation.height);
    _set_view(widget->allocation.width / float(widget->allocation.height));
    gdk_gl_drawable_gl_end(gldrawable);
    /*** OpenGL END ***/
    return TRUE;
}

// draws the stuff
void _draw()
{
        // DRAW STUFF HERE
        glDisable(GL_TEXTURE_RECTANGLE_ARB);
        glColor4f(1.0, 0.8, 0.2, 1.0);
        glPushMatrix();
        glScalef(0.5, 0.5, 1.0);
        draw::draw_square();
        glPopMatrix();

        glColor4f(1.0, 1.0, 0.0, 0.8);
        int num = 64;
        float x;
        for (int i = 0; i < num; i++)
        {
            x = (i / float(num)) * 4 - 2;
            draw::draw_line(float(x), -2.0, float(x), 2.0);
            draw::draw_line(-2.0, float(x), 2.0, float(x));
        }
}

gboolean Gui::on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    /*** OpenGL BEGIN ***/
    if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
      return FALSE;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _draw();

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

gboolean Gui::key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    Gui *context = static_cast<Gui*>(data);
    switch (event->keyval)
    {
        case GDK_Escape:
            context->toggleFullscreen(widget);
            break;
        case GDK_q:
            // Quit application on ctrl-q, this quits the main loop
            // (if there is one)
            if (event->state & GDK_CONTROL_MASK)
            {
                g_print("Ctrl-Q key pressed, quitting.");
                //context->app_.quit();
            }
            break;
        default:
            break;
    }
    return TRUE;
}

void Gui::on_delete_event(GtkWidget* widget, GdkEvent* event, gpointer data)
{
    Gui *context = static_cast<Gui*>(data);
    g_print("Close\n");
    gtk_main_quit();
}

void Gui::toggleFullscreen(GtkWidget *widget)
{
    // toggle fullscreen state
    isFullscreen_ ? makeUnfullscreen(widget) : makeFullscreen(widget);
}

void Gui::makeFullscreen(GtkWidget *widget)
{
    gtk_window_stick(GTK_WINDOW(widget)); // window is visible on all workspaces
    gtk_window_fullscreen(GTK_WINDOW(widget));
}

void Gui::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unstick(GTK_WINDOW(widget)); // window is not visible on all workspaces
    gtk_window_unfullscreen(GTK_WINDOW(widget));
}

/**
 * Exits the application if OpenGL needs are not met.
 */
Gui::Gui() :
    isFullscreen_(false) 
{
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
    gltools::examine_gl_config_attrib(glconfig);
    // Main GTK window
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window_, 640, 480);
    gtk_window_set_title(GTK_WINDOW (window_), "Toonloop 1.3 experimental");
    GdkGeometry geometry;
    geometry.min_width = 1;
    geometry.min_height = 1;
    geometry.max_width = -1;
    geometry.max_height = -1;
    gtk_window_set_geometry_hints(GTK_WINDOW(window_), window_, &geometry, GDK_HINT_MIN_SIZE);
    g_signal_connect(G_OBJECT(window_), "delete-event", G_CALLBACK(on_delete_event), this);
    g_signal_connect(G_OBJECT(window_), "key-press-event", G_CALLBACK(key_press_event), this);
    // add listener for window-state-event to detect fullscreenness
    g_signal_connect(G_OBJECT(window_), "window-state-event", G_CALLBACK(onWindowStateEvent), this);

    //area where the video is drawn
    drawing_area_ = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window_), drawing_area_);

    //avoid flickering when resizing or obscuring the main window
    //gtk_widget_realize(drawing_area_);
    //gdk_window_set_back_pixmap(drawing_area_->window_, NULL, FALSE);
    //gtk_widget_set_app_paintable(drawing_area_, TRUE);
    //gtk_widget_set_double_buffered(drawing_area_, FALSE);

    /* Set OpenGL-capability to the widget. */
    gtk_widget_set_gl_capability(drawing_area_, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
  
    g_signal_connect_after(G_OBJECT(drawing_area_), "realize", G_CALLBACK(on_realize), this);
    g_signal_connect(G_OBJECT(drawing_area_), "configure_event", G_CALLBACK(on_configure_event), this);
    g_signal_connect(G_OBJECT(drawing_area_), "expose_event", G_CALLBACK(on_expose_event), this);

    gtk_widget_show_all(window_);
}

void run_gui(gint argc, gchar* argv[])
{
    gtk_init(&argc, &argv);
    // Init GTK GL:
    gtk_gl_init(&argc, &argv);
    Gui gui = Gui();
    gtk_main();
}
