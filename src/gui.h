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
 */
#ifndef __GUI_H__
#define __GUI_H__

#include <GL/glx.h>
#include <clutter-gtk/clutter-gtk.h>
#include <clutter/clutter.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "application.h"

const int WINWIDTH = 640;
const int WINHEIGHT = 480;

class Gui
{
    public:
        ClutterActor* get_live_input_texture();
        Gui(Application* owner); 
        ~Gui() {};
        void toggleFullscreen() { toggleFullscreen(window_); } // no argument version of the same method below.
        ClutterActor *stage_;
        //FIXME:2010-08-06:aalex:live_input_texture_ should not be public?
        ClutterActor *live_input_texture_;
        ClutterActor *playback_texture_;
        ClutterTimeline *timeline_;
        void resize_actors();
        float video_input_width_;
        float video_input_height_;
        void switch_to_clip_number(unsigned int key_val);
    private:
        GtkWidget *window_;
        GtkWidget *clutter_widget_;
        GtkWidget *vbox_;
        bool isFullscreen_;
        Application* owner_;
        static void on_delete_event(GtkWidget* widget, GdkEvent* event, gpointer data);
        static gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);

        static int onWindowStateEvent(_GtkWidget *widget, _GdkEventWindowState *event, void *data);
        void toggleFullscreen(GtkWidget* widget);
        void makeFullscreen(GtkWidget* widget);
        void makeUnfullscreen(GtkWidget* widget);
        void hideCursor();
        void showCursor();

};

#endif // __GUI_H__
