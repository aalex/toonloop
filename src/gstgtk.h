
#ifndef __GST_GTK_H__
#define __GST_GTK_H__

#include <gst/interfaces/xoverlay.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

void gst_x_overlay_set_gtk_window (GstXOverlay *xoverlay, GtkWidget *window);

G_END_DECLS

#endif

