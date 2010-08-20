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
#include <iostream>
#include <cstdlib>
#include <stk/RtMidi.h>
#include "midi.h"

void MidiInput::input_message_cb(double delta_time, std::vector< unsigned char > *message, void *user_data )
{
    MidiInput* context = static_cast<MidiInput*>(user_data);
    unsigned int nBytes = message->size();
    for ( unsigned int i=0; i<nBytes; i++ )
        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    if ( nBytes > 0 )
        std::cout << "stamp = " << delta_time << std::endl;
    if (message->size() >= 3)
    {
        if ((int)message->at(1) == 64)
            if ((int)message->at(2) == 127)
                std::cout << "Sustain pedal is down."  << std::endl;
    }
}

const void MidiInput::enumerate_devices()
{
    // List inputs.
    std::cout << "\nThere are " << ports_count_ << " MIDI input sources available.\n";
    std::string portName;
    for (unsigned int i=0; i < ports_count_; i++) 
    {
        try {
            portName = midi_in_->getPortName(i);
        }
        catch (RtError &name_error) {
            name_error.printMessage();
            //goto cleanup; // FIXME no goto
        }
        std::cout << "  Input Port #" << i << ": " << portName << '\n';
    }
}

const bool MidiInput::is_open()
{
    return opened_;
}

MidiInput::MidiInput()
{
    midi_in_ = 0;
    // RtMidiIn constructor
    try {
        midi_in_ = new RtMidiIn(); // FIXME
    }
    catch (RtError &error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
  
    // Check available ports vs. specified.
    port_ = 0;
    ports_count_ = midi_in_->getPortCount();
    opened_ = false;
}

bool MidiInput::open(unsigned int port)
{
    ports_count_ = midi_in_->getPortCount();
    if (port >= ports_count_) 
    {
        //TODO: raise excepion !
        // delete midi_in_;
        std::cout << "Invalid port specifier!\n";
        // usage();
        opened_ = false;
        return false;
    }
    try 
    {
        midi_in_->openPort(port);
    }
    catch (RtError &error) 
    {
        std::cout << "Error opening MIDI port " << port << std::endl;
        error.printMessage();
        //goto cleanup; // FIXME: no goto
        opened_ = false;
        return false;
    }
  
    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue instead of sent to the callback function.
    midi_in_->setCallback(&input_message_cb, (void *) this);
    // Don't ignore sysex, timing, or active sensing messages.
    //midi_in_->ignoreTypes(false, false, false);
    // ignore sysex, timing, or active sensing messages.
    midi_in_->ignoreTypes(true, true, true);
    opened_ = true;
    return true;
}

MidiInput::~MidiInput()
{}

