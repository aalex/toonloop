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
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <tr1/unordered_map>

class Clip;
class Configuration;
class Controller;
class Gui;
class MidiInput;
class MovieSaver;
class OscInterface;
class Pipeline;

// FIXME:2010-08-17:aalex:We should allow more than 10 clips
static const unsigned int MAX_CLIPS = 10;
static const int DEFAULT_CAPTURE_WIDTH = 640;
static const int DEFAULT_CAPTURE_HEIGHT = 480;

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
        /** Returns the currently selected clip number */
        unsigned int get_current_clip_number();
        /** Should be only called directly by Controller::choose_clip */
        void set_current_clip_number(unsigned int clipnumber);

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
        std::tr1::unordered_map<int, Clip*> clips_;
};

#endif // __APPLICATION_H__
