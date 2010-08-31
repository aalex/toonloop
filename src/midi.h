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


#ifndef _MIDI_H_
#define _MIDI_H_

//#include <boost/signals2.hpp>
#include <stk/RtMidi.h>

class Application;

/** MIDI input using RtMidi allowing a pedal to control frame grabbing.
 */
class MidiInput
{
    public:
        MidiInput(Application *owner);
        bool open(unsigned int port);
        ~MidiInput();
        void enumerate_devices() const;
        static void input_message_cb(double delta_time, std::vector<unsigned char> *message, void *user_data);
        bool is_open() const;
        //boost::signals2::signal<void ()> pedal_down_signal_;
        bool verbose_;
        void set_verbose(bool verbose);
    private:
        Application *owner_;
        void on_pedal_down();
        void on_ctrl_80_changed(bool is_on);
        unsigned int port_;
        unsigned int ports_count_;
        RtMidiIn *midi_in_;
        bool opened_;
};

#endif // _MIDI_H_

