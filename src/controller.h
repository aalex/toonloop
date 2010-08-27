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

/**
 * The Controller contains most actions and events for Toonloop.
 */

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <string>

// Forward declaration
class Application;

/** The Controller contains most actions and events for Toonloop.
 *
 * The Controller contains the methods that any class should call
 * in order to create Toonloop animations. It also contains the signals 
 * to which they should connect their slots in order to subscribe to 
 * the events notifications. 
 */
class Controller
{
    public:
        Controller(Application* owner);
        /** 
         * Called when a frame is added to a clip.
         * Arguments: clip number, new frame number.
         */
        boost::signals2::signal<void (unsigned int, unsigned int)> add_frame_signal_;
        /** 
         * Called when a frame is removed from a clip.
         * Arguments: clip number, deleted frame number.
         */
        boost::signals2::signal<void (unsigned int, unsigned int)> remove_frame_signal_;
        /** 
         * Called when a clip is chosen.
         * Arguments: clip number.
         */
        boost::signals2::signal<void (unsigned int)> choose_clip_signal_;
        /** 
         * Called when the FPS of a clip changes.
         * Arguments: clip number, FPS.
         */
        boost::signals2::signal<void (unsigned int, unsigned int)> clip_fps_changed_signal_;
        /** 
         * Called when a clip is saved.
         * Arguments: clip number, file name.
         */
        //TODO: make the string &const
        boost::signals2::signal<void (unsigned int, std::string)> save_clip_signal_;


        /**
         * Called when it's time to play the next image.
         *
         * Arguments: clip number, image number, file name.
         */
        // TODO:2010-08-26:aalex:Make the file name string &const?
        boost::signals2::signal<void (unsigned int, unsigned int, std::string)> next_image_to_play_signal_;
        /**
         * Called when there is no image to play
         */
        boost::signals2::signal<void ()> no_image_to_play_signal_;
        /** 
         * Called when the direction of a clip changes
         * Arguments: clip number, string direction. (FORWARD, BACKWARD, YOYO)
         */
        boost::signals2::signal<void (unsigned int, std::string)> clip_direction_changed_signal_;
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
         *
         * Triggers the choose_clip_signal_ 
         */
        void choose_clip(unsigned int clip_number);
        /** 
         * Chooses the next clip
         *
         * Calls choose_clip
         */
        void choose_next_clip();
        /** 
         * Chooses the previous clip
         *
         * Calls choose_clip
         */
        void choose_previous_clip();
        /** 
         * Saves the currently selected clip
         *
         * Triggers the save_clip_signal_
         */
        void save_current_clip();
        
        /**
         * Checks if it's time to update the playback image
         * and iterate the playhead.
         *
         * Times the playhead and iterate it if it's time to.
         */
        void update_playback_image();

        /**
         * Increases the FPS of the current clip's playhead.
         */
        void increase_playhead_fps();

        /**
         * Decreases the FPS of the current clip's playhead.
         */
        void decrease_playhead_fps();
        /**
         * Sets the FPS of the current clip's playhead.
         */
        void set_playhead_fps(unsigned int fps);
        /**
         * Changes the playback direction of the current clip's playhead.
         *
         * Will navigate through FORWARD, BACKWARD and YOYO directions.
         * Triggers the clip_direction_changed_signal_
         */
        void change_current_clip_direction();

    private:
        Application* owner_;
};
// TODO: 
// /** 
// * Saves a clip
// */
// void save_clip(int clip_number);
#endif 
