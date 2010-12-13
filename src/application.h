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
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <tr1/memory>
#include <tr1/unordered_map>

class Clip;
class Configuration;
class Controller;
class Gui;
class Message;
class MidiInput;
class MovieSaver;
class OscInterface;
class Pipeline;

// FIXME:2010-08-17:aalex:We should allow more than 10 clips
static const unsigned int MAX_CLIPS = 40;
static const int DEFAULT_CAPTURE_WIDTH = 640;
static const int DEFAULT_CAPTURE_HEIGHT = 480;
// TODO:2010-10-15:aalex:Internationalize the help text.
static const std::string INTERACTIVE_HELP(
    "Toonloop interactive controls:"
    "\n  Space: Grab a single image."
    "\n  Escape: Switch fullscreen mode."
    "\n  Delete: Erase the last captured frame."
    "\n  Ctrl-q: Quit."
    "\n  Page-down: Switch to the next clip."
    "\n  Page-up: Switch to the previous clip."
    "\n  Number from 0 to 9: Switch to a specific clip."
    "\n  Ctrl-number: Switch to a specific layout."
    "\n  s: Save the current clip as a movie file."
    "\n  period (.): Toggle the layout."
    "\n  Tab: Change the playback direction."
    "\n  r: Clear the current clip."
    "\n  Caps lock: Toggle video grabbing."
    "\n  a: Toggle on/off the intervalometer."
    "\n  k: Increase the intervalometer interval by 1 second."
    "\n  j: Decrease the intervalometer interval by 1 second."
    "\n  Right: Move writehead to the next image."
    "\n  Left: Move writehead to the previous image."
    "\n  Return: Move writehead to the last image."
    "\n  semicolon (;): Move writehead to the first image."
    "\n  o: Enable/disable onion skinning."
    "\n  (): Decrease/increase frame blending in playback layout."
    "\n  []: Increase/decrease opacity of the live input image in the overlay layout."
    "\n  F1: Show help."
    "\n  x: Black out the whole window!"
    );

/** The Application class: starts Toonloop.
 */
class Application 
{
    public:
        Application();
        ~Application();
        void run(int argc, char *argv[]);
        void quit();
        Gui *get_gui();
        /** Returns the video Pipeline */
        Pipeline *get_pipeline();
        /** Returns the MIDI input manager */
        MidiInput *get_midi_input();
        /** Returns the Controller for actions and events */
        Controller *get_controller();
        /** Returns the Configuration for the application */
        Configuration *get_configuration();
        /** Returns the movie saving - using mencoder */
        MovieSaver *get_movie_saver();
        /** Returns the currently selected clip */
        Clip* get_current_clip();
        /** Returns a given clip or 0 if there is not such a clip. */
        Clip* get_clip(unsigned int clip_number);
        /** Returns the currently selected clip number */
        unsigned int get_current_clip_number();
        /** Should be only called directly by Controller::choose_clip */
        void set_current_clip_number(unsigned int clipnumber);
        /** Checks for asynchronous messages and treat them */
        void check_for_messages();
        void handle_message(Message &message);
        OscInterface* get_osc_interface();

    private:
        void update_project_home_for_each_clip();
        bool setup_project_home(const std::string& project_home);
        boost::scoped_ptr<Controller> controller_;
        boost::scoped_ptr<Gui> gui_;
        boost::scoped_ptr<MidiInput> midi_input_;
        boost::scoped_ptr<OscInterface> osc_;
        boost::scoped_ptr<Pipeline> pipeline_;
        boost::scoped_ptr<Configuration> config_;
        boost::scoped_ptr<MovieSaver> movie_saver_;
        unsigned int selected_clip_;
        // Aug 25 2010:tmatth:TODO:use shared_ptr, not raw pointers for clips_
        std::tr1::unordered_map<int, std::tr1::shared_ptr<Clip> > clips_;
};

#endif // __APPLICATION_H__
