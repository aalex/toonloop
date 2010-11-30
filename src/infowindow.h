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
class Application;

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
};
#endif
