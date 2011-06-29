/*
 * Toonloop
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
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
 */

#include <clutter/clutter.h>
#include <string.h> // memcpy
#include <clutter/x11/clutter-x11.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/cursorfont.h>

gboolean toon_clutter_stage_set_window_icon(ClutterStage *stage, const gchar *path, GError ** /*error*/ )
{
    Display *dpy = clutter_x11_get_default_display();
    Window win = clutter_x11_get_stage_window(stage);
    static Atom net_wm_icon = None;

    if (! net_wm_icon)
        net_wm_icon = XInternAtom(dpy, "_NET_WM_ICON", False);

    GdkPixbuf *pixbuf = NULL;
    GError *pixbuf_error = NULL;

    gint pixels_w = 32;
    gint pixels_h = 32;
    //g_info("Loading image at size of %dx%d\n", pixels_w, pixels_h);
    pixbuf = gdk_pixbuf_new_from_file_at_scale(path, pixels_w, pixels_h, FALSE, &pixbuf_error);
    if (! pixbuf)
    {
        g_error("Error loading image %s: %s", path, pixbuf_error->message);
        g_error_free(pixbuf_error);
        pixbuf_error = NULL;
        g_object_unref(pixbuf);
        return FALSE;
    }
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    if (n_channels != 4)
    {
        g_critical("Image has no alpha channel and we require one.");
        g_object_unref(pixbuf);
        return FALSE;
    }
    g_assert(gdk_pixbuf_get_colorspace(pixbuf) == GDK_COLORSPACE_RGB);
    g_assert(gdk_pixbuf_get_bits_per_sample(pixbuf) == 8);
    g_assert(gdk_pixbuf_get_has_alpha(pixbuf));
    g_assert(n_channels == 4);

    gulong bufsize = pixels_w * pixels_h * sizeof(char) * 4;
    guchar *data = (guchar *) g_malloc(bufsize + (sizeof(gulong) * 2));
    ((gulong *)data)[0] = pixels_w;
    ((gulong *)data)[1] = pixels_h;
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    memcpy((void *) &(((gulong *)data)[2]), (void *) pixels, bufsize);

    ///* For some inexplicable reason XChangeProperty always takes
    // * an array of longs when the format == 32 even on 64-bit
    // * architectures where sizeof(long) != 32. Therefore we need
    // * to pointlessly pad each 32-bit value with an extra 4
    // * bytes so that libX11 can remove them again to send the
    // * request. We can do this in-place if we start from the
    // * end 
    // */
    //if (sizeof(gulong) != 4)
    //{
    //    const guint32 *src = (guint32 *) (data + 2) + pixels_w * pixels_h;
    //    gulong *dst = data + 2 + pixels_w * pixels_h;
 
    //    while (dst > data + 2)
    //    {
    //        *(--dst) = *(--src);
    //    }
    //}

    XChangeProperty(
        dpy, // X11 display
        win, // X11 window
        net_wm_icon, // name of property
        XA_CARDINAL, // type of property
        32, // format
        PropModeReplace, // mode
        (unsigned char *) data, // data (we must cast it to unsigned char* if format is 32)
        pixels_w * pixels_h + 2); // nelements

    g_free(data);
    g_object_unref(pixbuf);
    return TRUE;
}

