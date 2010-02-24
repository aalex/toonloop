#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gtk/gtkgl.h>
#include "application.h"
#include <iostream>
// gint gint gchar
int main(int argc, char* argv[])
{
    // Init GTK:
    gtk_init(&argc, &argv);
    // Init GTK GL:
    gtk_gl_init(&argc, &argv);
    // Init GST:
    gst_init(&argc, &argv);

    try 
    {
        Application::get_instance().run();
    }
    catch(const std::exception& e) 
    {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch (...) 
    {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }
    return 0;
}
