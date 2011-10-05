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

/**
 * The Controller contains most actions and events for Toonloop.
 */

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <string>
#include "clip.h" // for clip_direction enum
#include "properties.h"
#include "property.h"
#include "timer.h"

typedef Property<int> IntProperty;
typedef Property<float> FloatProperty;

// Forward declaration
class Application;

/** The Controller contains most actions and events for Toonloop.
 *
 * The Controller contains the methods that any class should call
 * in order to create Toonloop animations. 
 *
 * It also contains the signals to which they should connect their 
 * slots in order to subscribe to the events notifications. 
 */
class Controller
{
    public:
        /**
         * Constructor.
         */
        Controller(Application* owner);
        /**
         * Hash table of int properties. 
         * Should be used more in future.
         */
        Properties<int> int_properties_;
        /**
         * Hash table of float properties. 
         * Should be used more in future.
         */
        Properties<float> float_properties_;
        /**
         * Creates a new named int property.
         * Returns a pointer to it, so that clients can query its value and register slots for their signals.
         */
        Property<int> *add_int_property(const std::string &name, int value);
        /**
         * Creates a new named float property.
         * Returns a pointer to it, so that clients can query its value and register slots for their signals.
         */
        Property<float> *add_float_property(const std::string &name, float value);
        /** 
         * Sets the value of a int property, given its name. 
         * Returns whether is succeeded or not.
         */
        bool set_int_value(const std::string &name, int value);
        /** 
         * Sets the value of a float property, given its name. 
         * Returns whether is succeeded or not.
         */
        bool set_float_value(const std::string &name, float value);
        /**
         * Returns the value of a named int property.
         * Returns 0 if it doesn't exist.
         */
        int get_int_value(const std::string &name);
        /**
         * Returns the value of a named float property.
         * Returns 0.0 if it doesn't exist.
         */
        float get_float_value(const std::string &name);

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
         * Called when a clip is cleared of all its images.
         * Argument: clip number
         */
        boost::signals2::signal<void (unsigned int)> clip_cleared_signal_;
        /**
         * Called when the auto video grabbing of every frame is toggled.
         * Arguments: clip number, autograb is enabled
         */
        boost::signals2::signal<void (unsigned int, bool)>clip_videograb_changed_signal_;
        /**
         * Called when a clip's intervalometer rate is changed. 
         *
         * Arguments: clip number, rate in seconds.
         */
        boost::signals2::signal<void (unsigned int, float)> intervalometer_rate_changed_signal_;
        /**
         * Called when a clip's intervalometer is enabled or not.
         *
         * Arguments: clip number, intervalometer is enabled.
         */
        boost::signals2::signal<void (unsigned int, bool)> intervalometer_toggled_signal_;
        // TODO: the writehead_moved_signal_ should be triggered when we add or remove an image.
        /**
         * Called when the writehead position changes
         *
         * Arguments: clip number, writehead position.
         */
        boost::signals2::signal<void (unsigned int, unsigned int)> writehead_moved_signal_;
        /**
         * Called when the whole project has been saved to an XML file.
         */
        boost::signals2::signal<void (std::string)> save_project_signal_;

        /** 
         * Called when the playback is toggled.
         * Arguments: enabled (true if not paused)
         */
        boost::signals2::signal<void (bool)> playback_toggled_signal_;

        // ----------------------------------- methods ---------------------
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
        /**
         * Sets the playback direction of the current clip's playhead.
         *
         * Triggers the clip_direction_changed_signal_
         */
        void set_current_clip_direction(const std::string &direction);
        /**
         * Clears the current clip of all its images.
         *
         * Triggers the clip_cleared_signal_
         */
        void clear_current_clip();
        /**
         * Toggles on/off the grabbing of every consecutive frame.
         *
         * Triggers the clip_videograb_changed_signal_
         */
        void toggle_video_grabbing();
        /**
         * Enables or not the grabbing of every consecutive frame.
         *
         * Triggers the clip_videograb_changed_signal_
         */
        void enable_video_grabbing(bool enable);
        /**
         * Toggles on/off the intervalometer
         *
         * Triggers the intervalometer_toggled_signal_
         */
        void toggle_intervalometer();
        /**
         * Enables or not the intervalometer
         *
         * Triggers the intervalometer_toggled_signal_
         */
        void enable_intervalometer(bool enable);

        /**
         * Sets the intervalometer rate for the current clip.
         *
         * Triggers the intervalometer_rate_changed_signal_
         */
        void set_intervalometer_rate(float rate);

        /**
         * Increases the intervalometer rate for the current clip.
         *
         * Triggers the intervalometer_rate_changed_signal_
         */
        void increase_intervalometer_rate();
        /**
         * Decreases the intervalometer rate for the current clip.
         *
         * Triggers the intervalometer_rate_changed_signal_
         */
        void decrease_intervalometer_rate();
        /**
         * Moves the writehead of the current clip to the next image.
         * 
         * Triggers the writehead_moved_signal_
         */
        void move_writehead_to_next();
        /**
         * Moves the writehead of the current clip to the previous image.
         * 
         * Triggers the writehead_moved_signal_
         */
        void move_writehead_to_previous();
        /**
         * Moves the writehead of the current clip to the last image.
         * 
         * Triggers the writehead_moved_signal_
         */
        void move_writehead_to_last();
        /**
         * Moves the writehead of the current clip to its first image.
         * 
         * Triggers the writehead_moved_signal_
         */
        void move_writehead_to_first();
        /**
         * Moves the writehead of the current clip to a given image.
         * 
         * Triggers the writehead_moved_signal_
         */
        void move_writehead_to(unsigned int position);

        /**
         * Quits the application.
         */
        void quit();

        /**
         * Prints all the Toonloop properties.
         */
        void print_properties();

        /** 
         * Saves the whole project to an XML file.
         */
        void save_project();

        /**
         * Imports an image to the current clip.
         */
        void import_image(const std::string &file_name);

        /**
         * Wrapper to choose a clip and add a frame in a single call.
         * See Controller::choose_clip and Controller::add_frame.
         */
        void choose_clip_and_add_frame(unsigned int clip_number);

        bool get_playback_enabled() const
        {
            return playback_enabled_;
        }

        void playback_toggle(bool enabled);

        void move_playhead_to(unsigned int position);
    private:
        Application* owner_;
        Timer playback_timer_;
        unsigned int prev_clip_id_;
        void advertise_current_image();
        std::string prev_image_name_;
        bool playback_enabled_; // false if paused
};
// TODO: 
// /** 
// * Saves a clip by number
// */
// void save_clip(int clip_number);
#endif 
