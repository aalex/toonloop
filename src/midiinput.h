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

#ifndef _MIDI_H_
#define _MIDI_H_

#include "command.h"
#include "concurrentqueue.h"
#include "midibinder.h"
#include <tr1/memory>

class Application;
class RtMidiIn;

/** MIDI input using RtMidi allowing a pedal to control frame grabbing.
 */
class MidiInput
{
    public:
        // types:
        typedef std::tr1::shared_ptr<Command> CommandPtr;
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
        void consume_commands();
    private:
        Application *owner_;
        ConcurrentQueue<CommandPtr> messaging_queue_;
        //TODO: ConcurrentQueue<std::tr1::shared_ptr<Command> > messaging_queue_;
        unsigned int port_;
        unsigned int ports_count_;
        bool opened_;
        MidiBinder midi_binder_;
        void push_command(CommandPtr command);
        RtMidiIn *midi_in_;
        Message make_message(const std::string &action);
        CommandPtr make_command(const MidiRule *rule);
        bool find_rule_for_note_off(int note_pitch);
        bool find_rule_for_note_on(int note_pitch);
        bool find_rule_for_control_off(int controller_number);
        bool find_rule_for_control_on(int controller_number);
        bool find_rule_for_control_map(int controller_number, int control_value);
        bool find_rule_for_program_change(int program_number);
        bool find_rule_for_pitch_wheel(int pitch_bend);
};

#endif // _MIDI_H_

