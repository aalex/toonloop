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

#include "concurrentqueue.h"
#include "message.h"
#include "midibinder.h"

class Application;
class RtMidiIn;

/** MIDI input using RtMidi allowing a pedal to control frame grabbing.
 */
class MidiInput
{
    public:
        /** Contructor. */
        MidiInput(Application *owner, bool verbose);
        /** Opens a MIDI source device. */
        bool open(unsigned int port);
        ~MidiInput();
        /** Enumerates the list of MIDI source devices. */
        void enumerate_devices() const;
        static void input_message_cb(double delta_time, std::vector<unsigned char> *message, void *user_data);
        bool is_open() const;
        bool verbose_;
        /** Sets it verbose or not. */
        void set_verbose(bool verbose);
        /** Flushes the messages from the asynchronous messaging queue. */
        void consume_messages();
    private:
        Application *owner_;
        ConcurrentQueue<Message> messaging_queue_;
        unsigned int port_;
        unsigned int ports_count_;
        bool opened_;
        MidiBinder midi_binder_;
        //void find_and_apply_matching_event(std::string tag_name, std::string attr_name, int attr_value);
        void push_message(Message message);
        // TODO: get rid of all push_action_* methods
        void push_action_with_string(std::string action, std::string args);
        void push_action_with_int(std::string action, int arg);
        void push_action_with_string_and_int(std::string action, std::string args, int int_arg);
        void push_action_with_float(std::string action, std::string args, float float_arg);
        RtMidiIn *midi_in_;
};

#endif // _MIDI_H_

