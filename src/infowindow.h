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

#ifndef __INFOWINDOW_H__
#define __INFOWINDOW_H__
#include <clutter/clutter.h>
#include <string>
#include <iostream>
#include <vector>
#include <tr1/memory>

class Application;
using std::tr1::shared_ptr;
// TODO: change for a struct since all its data is public.
class ClipInfoBox
{
    public:
        gdouble position_;
        ClutterActor *group_;
        ClutterActor *image_;
        ClutterActor *label_;
        ClipInfoBox() :
            position_(0.0),
            group_(NULL),
            image_(NULL),
            label_(NULL)
        {};
};

class InfoWindow
{
    public:
        InfoWindow(Application *app);
        void create();
        void update_info_window();
    private:
        Application *app_;
        ClutterActor *stage_;
        ClutterActor *text_;
        ClutterActor *clipping_group_;
        ClutterActor *scrollable_box_;
        std::vector<shared_ptr<ClipInfoBox> > clips_;
        static void on_window_destroyed(ClutterActor &stage, gpointer data);
        void on_choose_clip(unsigned int clip_number);
        unsigned int previously_selected_;
        void update_num_frames(unsigned int clip_number);
        void on_add_frame(unsigned int clip_number, unsigned int frame_number);
        void on_remove_frame(unsigned int clip_number, unsigned int frame_number);
        void on_clip_cleared(unsigned int clip_number);
};
#endif
