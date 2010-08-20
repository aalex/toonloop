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
#include <stk/RtMidi.h>

class MidiInput
{
    public:
        MidiInput();
        bool open(unsigned int port);
        ~MidiInput();
        const void enumerate_devices();
        static void input_message_cb(double delta_time, std::vector<unsigned char> *message, void *user_data);
        const bool is_open();
    private:
        unsigned int port_;
        unsigned int ports_count_;
        RtMidiIn *midi_in_;
        bool opened_;
};

