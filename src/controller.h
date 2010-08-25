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
#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <boost/bind.hpp>
#include <boost/signals2.hpp>

namespace s2 = boost::signals2;

// Forward declaration
class Application;

/**
 * The Controller contains the methods that any class should call
 * in order to create Toonloop animations. It also contains the signals 
 * to which they should connect their slots in order to subscribe to 
 * the events notifications. 
 */
class Controller
{
    public:
        /**
         * Constructor
         */
        Controller(Application* owner);
        /** 
         * Called when a frame is added to a clip.
         * Arguments: clip number, new frame number.
         */
        s2::signal<void (unsigned int, unsigned int)> add_frame_signal_;
        /** 
         * Called when a frame is removed from a clip.
         * Arguments: clip number, deleted frame number.
         */
        s2::signal<void (unsigned int, unsigned int)> remove_frame_signal_;
        /** 
         * Called when a clip is chosen.
         * Arguments: clip number.
         */
        s2::signal<void (unsigned int)> choose_clip_signal_;
        /** 
         * Called when the FPS of a clip changes.
         * Arguments: clip number, FPS.
         */
        s2::signal<void (unsigned int, unsigned int)> clip_fps_changed_signal_;
        /** 
         * Called when a clip is saved.
         * Arguments: clip number.
         */
        s2::signal<void (unsigned int)> save_clip_signal_;
        /**
         * Adds a frame to the current clip.
         */
        void add_frame();
        /**
         * Removes a frame from the current clip.
         */
        void remove_frame();
        /** 
         * Chooses a clip
         */
        void choose_clip(unsigned int clip_number);
        /** 
         * Chooses the next clip
         */
        void choose_next_clip();
        /** 
         * Chooses the previous clip
         */
        void choose_previous_clip();
        /** 
         * Saves the currently selected clip
         */
        void save_current_clip();

    private:
        Application* owner_;
};
// TODO: 
// /** 
// * Saves a clip
// */
// void save_clip(int clip_number);
#endif 
