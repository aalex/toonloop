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
#include <boost/lexical_cast.hpp>
#include <cstdlib>
#include <iostream>
#include <stk/RtMidi.h>
#include "application.h"
#include "configuration.h"
#include "controller.h"
#include "message.h"
#include "midi.h"
#include "presets.h"

static const unsigned char MIDINOTEOFF =       0x80; // channel, pitch, velocity
static const unsigned char MIDINOTEON =        0x90; // channel, pitch, velocity
static const unsigned char MIDICONTROLCHANGE = 0xb0; // channel, controller, value
static const unsigned char MIDIPROGRAMCHANGE = 0xc0; // channel, value
static const unsigned char MIDIPITCHBEND =     0xe0; // channel, value
// TODO:2010-11-07:aalex:Support other MIDI event types.
//static const unsigned char MIDIPOLYTOUCH =     0xa0; // channel, pitch, velocity
//static const unsigned char MIDICHANNELTOUCH=   0xd0; /* 1 */
//static const unsigned char MIDISTARTSYSEX =    0xf0; /* (until F7) */
//static const unsigned char MIDITIMECODE =      0xf1; /* 1 */
//static const unsigned char MIDISONGPOS =       0xf2; /* 2 */
//static const unsigned char MIDISONGSELECT =    0xf3; /* 1 */
//static const unsigned char MIDIRESERVED1 =     0xf4; /* ? */
//static const unsigned char MIDIRESERVED2 =     0xf5; /* ? */
//static const unsigned char MIDITUNEREQUEST =   0xf6; /* 0 */
//static const unsigned char MIDIENDSYSEX =      0xf7; /* 0 */
//static const unsigned char MIDICLOCK =         0xf8; /* 0 */
//static const unsigned char MIDITICK =          0xf9; /* 0 */
//static const unsigned char MIDISTART =         0xfa; /* 0 */
//static const unsigned char MIDICONT =          0xfb; /* 0 */
//static const unsigned char MIDISTOP =          0xfc; /* 0 */
//static const unsigned char MIDIACTIVESENSE =   0xfe; /* 0 */
//static const unsigned char MIDIRESET =         0xff; /* 0 */
static const unsigned char MIDI_NOT_SUPPORTED = 0x00;

unsigned char get_midi_event_type(const unsigned char first_byte)
{
    unsigned char type_code;
    if (first_byte >= 0xf0)
        type_code = first_byte;
    else
        type_code = first_byte & 0xf0;
    unsigned char ret;
    switch (type_code)
    {
        case MIDINOTEOFF:
        case MIDINOTEON:
        case MIDICONTROLCHANGE:
        case MIDIPROGRAMCHANGE:
        case MIDIPITCHBEND:
            ret = type_code;
            break;
        default:
            ret = MIDI_NOT_SUPPORTED;
            break;
    }
    return ret;
}

/**
 * Callback for incoming MIDI messages.  Called in its thread.
 * 
 * - Pressing the sustain pedal down grabs a frame.
 *   MIDI controller 64 is the sustain pedal controller. It looks like this:
 *   <channel and status> <controller> <value>
 *   Where the controller number is 64 and the value is either 0 or 127.
 *
 * - The MIDI controller 80 is also a pedal on the Roland GFC-50.
 *   It controls video grabbing. (on / off)
 *
 * - The program change should allow the user to choose another instrument. 
 *   This way, the Roland GFC-50 allows to select any of ten clips.
 *   The MIDI spec allows for 128 programs, numbered 0-127.
 *
 * - Main volume is control 7. It controls the playback speed.
 *   Volume is from 0 to 127.
 */
void MidiInput::input_message_cb(double /* delta_time */, std::vector< unsigned char > *message, void *user_data )
{
    MidiInput* context = static_cast<MidiInput*>(user_data);
    //MidiEventType event_type = NOTE_ON; // default...
    //unsigned char value = 0;
    // Print debug info:
    std::cout << __FUNCTION__ << std::endl;
    //if (context->verbose_) 
    if (true)
    {
        std::cout << "MIDI message: (";
        for (unsigned int i = 0; i < message->size(); i++)
            std::cout << (int)message->at(i) << " ";
        std::cout << ")";
    }
    if (message->size() <= 1)
        return; // Don't support messages with only one byte or less.
    unsigned char message_type = get_midi_event_type(message->at(0));
    const MidiRule *rule;
    switch (message_type)
    {
        case MIDINOTEON:
            std::cout << "NOTE ON" << std::endl;
            if (message->size() < 3)
            {
                std::cout << "Note on message should have 4 params" << std::endl;
                return;
            }
            if (message->at(2) == 0x00) // if velocity is 0, it's actually a note off message
            {
                rule = context->midi_binder_.find_rule(NOTE_OFF_RULE, int(message->at(1)));
                if (rule != 0) {
                    context->push_action(rule->action_, rule->args_);
                    return;
                }
            } else {
                //context->find_and_apply_matching_event("note_on", "note", ));
                rule = context->midi_binder_.find_rule(NOTE_ON_RULE, int(message->at(1)));
                if (rule != 0)
                {
                    context->push_action(rule->action_, rule->args_);
                    return;
                }
            }
            break;
        case MIDINOTEOFF:
            std::cout << "NOTE OFF" << std::endl;
            rule = context->midi_binder_.find_rule(NOTE_OFF_RULE, int(message->at(1)));
            if (rule != 0)
            {
                context->push_action(rule->action_, rule->args_);
                return;
            }
            break;
        case MIDICONTROLCHANGE:
        { // we declare some scope variables:
            std::cout << "CONTROL" << std::endl;
            int controller_number = int(message->at(1));
            int control_value = int(message->at(2));
            std::cout << "control_value" << control_value << std::endl;
            if (control_value == 0)
            {
                rule = context->midi_binder_.find_rule(CONTROL_OFF_RULE, controller_number);
                if (rule != 0)
                {
                    context->push_action(rule->action_, rule->args_);
                    return;
                }
            } else {
                rule = context->midi_binder_.find_rule(CONTROL_ON_RULE, controller_number);
                if (rule != 0)
                {
                    context->push_action(rule->action_, rule->args_);
                    return;
                }
            } // and if not of those found:
            rule = context->midi_binder_.find_rule(CONTROL_MAP_RULE, controller_number);
            if (rule != 0)
            {
                //TODO:2010-11-07:aalex:Map the value from [0,127] to the desired range:
                context->push_action(rule->action_, boost::lexical_cast<std::string>(control_value)); // we pass the value
                return;
            }
            break;
        }
        //    context->find_and_apply_matching_event("note_on", "note", int(message->at(2)));
        //case MIDIPROGRAMCHANGE:
        //    context->find_and_apply_matching_event("note_on", "note", int(message->at(2)));
        //    break;
        //case MIDIPITCHBEND:
        //    context->find_and_apply_matching_event("note_on", "note", int(message->at(2)));
        //    break;
        default:
            return;
            break;
    }

//    // Check if it's a note or control:
//    if (message->size() >= 3)
//    {
//        value = message->at(1);
//        if ((int)message->at(2) != 0) 
//            event_type = PEDAL_ON;
//        else
//            event_type = PEDAL_OFF;
//        //else if ((int)message->at(1) == 7) // 7: volume expression pedal
//        //    context->on_volume_control((int)message->at(2));
//    }
//    else if (message->size() >= 2)
//    {
//        if (((int)message->at(0) & 192) == 192)
//        {
//            // Program change control (chooses a clip)
//            unsigned int clip_number = (unsigned int)message->at(1);
//            //context->on_program_change();
//            if (clip_number >= 10)
//                std::cout << "Cannot choose a clip greater or equal to 10." << std::endl; 
//            else
//                context->push_message(Message(Message::SELECT_CLIP, clip_number));
//        }
//    }
//    // TODO: change those by lookups in the map of MidiRules
//    if (event_type == PEDAL_ON && value == 64)
//        context->push_message(Message(Message::ADD_IMAGE));
//    else if (value == 80)
//    {
//        if (event_type == PEDAL_ON)
//            context->push_message(Message(Message::VIDEO_RECORD_ON));
//        else
//            context->push_message(Message(Message::VIDEO_RECORD_OFF));
//    }
}

#if 0
/**
 * Checks for a tag in our MIDI rules whose attribute match a given value.
 * If one is found, pushes a message to the messaging queue.
 *
 * Example of arguments to this method:: "note_on", "note", 64
 *
 * Give "" as attr_name and it won't validate any attr. (always match)
 */
void MidiInput::find_and_apply_matching_event(std::string tag_name, std::string attr_name, int attr_value)
{
    std::vector<MidiRule> rules = midi_binder_.get_rules();
    for (MidiRuleIterator iter = rules.begin(); iter != rules.end(); ++iter)
    {
        if (iter->name_ == tag_name)
        {
            std::cout << "Found a rule with " << tag_name << std::endl;
            bool matches(false);
            if (attr_name == "")
                matches = true;
            // TODO: replace "note" to attr
            MidiRuleAttributeIter note_found = iter->attributes_.find(attr_name);
            if (note_found != iter->attributes_.end()) // if found it
            {
                int note = boost::lexical_cast<int>((*note_found).second);
                if (note == attr_value)
                {
                    matches = true;
                }
            }
            if (matches)
            {
                MidiRuleAttributeIter args_found = iter->attributes_.find("args");
                std::string args("");
                if (args_found != iter->attributes_.end())
                    args = (*args_found).second;
                MidiRuleAttributeIter action_found = iter->attributes_.find("action");
                if (action_found == iter->attributes_.end())
                    g_critical("Could not find any action in that XML MIDI rule.");
                else
                {
                    std::cout << "push_action " << (*action_found).second << " " << args << std::endl;
                    push_action((*action_found).second, args);
                }
                break; // leave for loop
            }
        }
    }
}
#endif
/**
 * Here we map the string for actions to their Message class const 
 */
void MidiInput::push_action(std::string action, std::string args)
{
    std::cout << __FUNCTION__ << " " << action << " " << args << std::endl;
    if (action == "add_image") 
        push_message(Message(Message::ADD_IMAGE));
    else if (action == "remove_image") 
        push_message(Message(Message::REMOVE_IMAGE));
    else if (action == "select_clip") 
        push_message(Message(Message::SELECT_CLIP, boost::lexical_cast<int>(args)));
    else if (action == "video_record_on") 
        push_message(Message(Message::VIDEO_RECORD_ON));
    else if (action == "video_record_off") 
        push_message(Message(Message::VIDEO_RECORD_OFF));
    //else if (action == "quit")
    //    push_message(Message(Message::QUIT));
    else
        g_critical("Unknown action %s", action.c_str());
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

void MidiInput::push_message(Message message)
{
    // TODO: pass this message argument by reference?
    messaging_queue_.push(message);
}

/**
 * Takes action!
 *
 * Should be called when it's time to take action, before rendering a frame, for example.
 */
void MidiInput::consume_messages()
{
    // TODO:2010-10-03:aalex:Move this handling to Application.
    bool success = true;
    while (success)
    {
        Message message;
        success = messaging_queue_.try_pop(message);
        if (success)
        {
            owner_->handle_message(message);
        }
    }
}

MidiInput::MidiInput(Application* owner) : 
        owner_(owner),
        messaging_queue_(),
        port_(0),
        ports_count_(0),
        opened_(false),
        midi_binder_()
{
    verbose_ = false;
    //verbose_ = true;
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
    ports_count_ = midi_in_->getPortCount();
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
    // TODO:2010-10-03:aalex:Pass only a pointer to the concurrent queue, no the whole MidiInput instance, 
    // which is not thread safe
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
{
    delete midi_in_;
}

