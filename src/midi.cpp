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
#include "application.h"
#include "configuration.h"
#include "controller.h"
#include "midi.h"
/**
 * Callback for incoming MIDI messages. 
 * 
 * MIDI controller 64 is the sustain pedal controller. It looks like this:
 *   <channel and status> <controller> <value>
 * Where the controller number is 64 and the value is either 0 or 127.
 *
 * The MIDI controller 80 is also a pedal on the Roland GFC-50.
 * It controls video grabbing.
 *
 * The program change should allow the user to choose another instrument. 
 * This way, the Roland GFC-50 allows to select any of ten clips.
 * The MIDI spec allows for 128 programs, numbered 0-127.
 */
void MidiInput::input_message_cb(double delta_time, std::vector< unsigned char > *message, void *user_data )
{
    MidiInput* context = static_cast<MidiInput*>(user_data);
    unsigned int nBytes = message->size();
    if (context->verbose_) 
    {
        std::cout << "(";
        for (unsigned int i = 0; i < nBytes; i++)
            std::cout << (int)message->at(i) << " ";
        std::cout << ")";
        if (nBytes > 0)
            std::cout << " stamp = " << delta_time << std::endl;
    }
    if (message->size() >= 3)
    {
        if ((int)message->at(1) == 64) // 64: sustain pedal
        {
            if ((int)message->at(2) == 127) 
                context->on_pedal_down();
        }
        else if ((int)message->at(1) == 80) // 80: general-purpose control. The Roland GFC-50 provides an on/off pedal input with that number
        {
            if ((int)message->at(2) == 127) 
                context->on_ctrl_80_changed(true);
            else
                context->on_ctrl_80_changed(false);
        } 
    }
    else if (message->size() >= 2)
    {
        if (((int)message->at(0) & 192) == 192)
        {
            context->on_program_change((int)message->at(1));
        }
    }
}

void MidiInput::enumerate_devices() const
{
    // List inputs.
    std::cout << std::endl << "MIDI input devices: " << ports_count_ << " found." << std::endl;
    std::string portName;
    for (unsigned int i=0; i < ports_count_; i++) 
    {
        try {
            portName = midi_in_->getPortName(i);
        }
        catch (RtError &name_error) {
            name_error.printMessage();
        }
        std::cout << " * " << i << " : " << portName << '\n';
    }
}

bool MidiInput::is_open() const
{
    return opened_;
}

/** Called when a sustain MIDI pedal goes down.
 */
void MidiInput::on_pedal_down()
{
    if (owner_->get_configuration()->get_verbose())
        std::cout << "on_pedal_down" << std::endl;
    owner_->get_controller()->add_frame();
}
/** Called when a control #80's value changes.
 */
void MidiInput::on_ctrl_80_changed(bool is_on)
{
    if (owner_->get_configuration()->get_verbose())
        std::cout << "control #80's value changed:" << is_on << std::endl;
    owner_->get_controller()->enable_video_grabbing(is_on);
}
/** Called when the user sends a program change message
 */
void MidiInput::on_program_change(int number)
{
    if (number >= 10)
        std::cout << "Cannot choose a clip greater or equal to 10." << std::endl; 
    else
    {
        owner_->get_controller()->choose_clip(number);
    }
}

MidiInput::MidiInput(Application* owner) : 
        owner_(owner)
{
    verbose_ = false;
    midi_in_ = 0;
    // RtMidiIn constructor
    try {
        midi_in_ = new RtMidiIn; // FIXME: should be use a pointer?
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
        std::cout << "Invalid input MIDI port!" << std::endl;
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

void MidiInput::set_verbose(bool verbose)
{
    verbose_ = verbose;
}

MidiInput::~MidiInput()
{}

