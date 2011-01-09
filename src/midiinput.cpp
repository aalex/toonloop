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
#include <algorithm> // min, max
#include <boost/lexical_cast.hpp>
#include <cstdlib>
#include <iostream>
#include <stk/RtMidi.h>
#include <tr1/memory> // shared_ptr
#include "application.h"
#include "configuration.h"
#include "command.h"
#include "controller.h"
#include "midiinput.h"
#include "midibinder.h"

/**
 * The MIDI bindings are now set up with a XML file in the MidiBinder class.
 *
 * Historically:
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
 * Convenience function to map a variable from one coordinate space
 * to another.
 * The result is clipped in the range [ostart, ostop]
 * Make sure ostop is bigger than ostart.
 *
 * To map a MIDI control value into the [0,1] range:
 * map(value, 0.0, 1.0, 0. 127.);
 *
 * Depends on: #include <algorithm>
 */
float map_float(float value, float istart, float istop, float ostart, float ostop)
{
    float ret = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    // In Processing, they don't do the following: (clipping)
    return std::max(std::min(ret, ostop), ostart);
}

int map_int(int value, int istart, int istop, int ostart, int ostop)
{
    float ret = ostart + (ostop - ostart) * ((value - istart) / float(istop - istart));
    //g_print("%f = %d + (%d-%d) * ((%d-%d) / (%d-%d))", ret, ostart, ostop, ostart, value, istart, istop, istart);
    // In Processing, they don't do the following: (clipping)
    return std::max(std::min(int(ret), ostop), ostart);
}

/**
 * Casts the args_ member of a MidiRule to any type.
 */
template <typename T>
static bool cast_arg(const MidiRule &rule, T *ret)
{
    try
    {
        *ret = boost::lexical_cast<T>(rule.args_);
    }
    catch (boost::bad_lexical_cast &e)
    {
        g_critical("Bad argument type for MIDI rule %s: %s\n", rule.action_.c_str(), e.what());
        return false;
    }
    return true;
}

/**
 * Factory for commands with no argument.
 */
MidiInput::CommandPtr MidiInput::make_command(const MidiRule *rule)
{
    // Try to sort them by popularity order
    if (rule->action_ == "add_image")
        return CommandPtr(new AddImageCommand);
    else if (rule->action_ == "remove_image")
        return CommandPtr(new RemoveImageCommand);
    else if (rule->action_ == "video_record_on")
        return CommandPtr(new VideoRecordOnCommand);
    else if (rule->action_ == "video_record_off")
        return CommandPtr(new VideoRecordOffCommand);
    else if (rule->action_ == "quit")
        return CommandPtr(new QuitCommand);
    else if (rule->action_ == "select_clip")
    {
        unsigned int clip_number;
        if (cast_arg<unsigned int>(*rule, &clip_number))
            return CommandPtr(new SelectClipCommand(clip_number));
        else
            return CommandPtr();
    }
    else
        return CommandPtr();
}

/**
 * Returns true if it finds a rule.
 */
bool MidiInput::find_rule_for_note_off(int note_pitch)
{
    const MidiRule *rule = midi_binder_.find_rule(NOTE_OFF_RULE, note_pitch);
    if (rule != 0)
    {
        CommandPtr c = make_command(rule);
        push_command(c);
        return true;
    }
    else
        return false;
}

/**
 * Returns true if it finds a rule.
 */
bool MidiInput::find_rule_for_note_on(int note_pitch)
{
    const MidiRule *rule = midi_binder_.find_rule(NOTE_ON_RULE, note_pitch);
    if (rule != 0)
    {
        CommandPtr c = make_command(rule);
        push_command(c);
        return true;
    }
    else
        return false;
}

/**
 * Returns true if it finds a rule.
 */
bool MidiInput::find_rule_for_control_off(int controller_number)
{
    const MidiRule *rule = midi_binder_.find_rule(CONTROL_OFF_RULE, controller_number);
    if (rule != 0)
    {
        CommandPtr c = make_command(rule);
        push_command(c);
        return true;
    }
    else
        return false;
}

/**
 * Returns true if it finds a rule.
 */
bool MidiInput::find_rule_for_control_on(int controller_number)
{
    const MidiRule *rule = midi_binder_.find_rule(CONTROL_ON_RULE, controller_number);
    if (rule != 0)
    {
        CommandPtr c = make_command(rule);
        push_command(c);
        return true;
    }
    else
        return false;
}

/**
 * Returns true if it finds a rule.
 */
bool MidiInput::find_rule_for_control_map(int controller_number, int control_value)
{
    const MidiRule *rule = midi_binder_.find_rule(CONTROL_MAP_RULE, controller_number);
    if (rule != 0)
    {
        if (rule->action_ == "set_float")
        {
            float f_val = map_float((float) control_value , 0.0f, 127.0f, rule->from_, rule->to_);
            push_command(CommandPtr(new SetFloatCommand(rule->args_, f_val)));
        }
        else if (rule->action_ == "set_int")
        {
            int i_val = map_int((int) control_value , 0, 127, (int) rule->from_, (int) rule->to_);
            push_command(CommandPtr(new SetIntCommand(rule->args_, i_val)));
        }
        else
        {
            g_critical("Control map MIDI rules only support set_float and set_int commands. Found %s", rule->action_.c_str());
        }
        return true; // it found a rule, even if invalid
    }
    else
        return false;
}

/**
 * Returns true if it finds a rule.
 */
bool MidiInput::find_rule_for_program_change(int program_number)
{
    const MidiRule *rule = midi_binder_.find_program_change_rule();
    if (rule != 0)
    {
        if (rule->action_ == "select_clip")
        {
            unsigned int clip_number = (unsigned int) program_number;
            CommandPtr c = CommandPtr(new SelectClipCommand(clip_number));
            push_command(c);
        }
        else
            g_critical("Program change MIDI event supports only support select_clip command. Found %s", rule->action_.c_str());
        return true;
    }
    else
        return false;
}

/**
 * Returns true if it finds a rule.
 */
bool MidiInput::find_rule_for_pitch_wheel(int pitch_bend)
{
    const MidiRule *rule = midi_binder_.find_pitch_wheel_rule();
    if (rule != 0)
    {
        if (rule->action_ == "set_float")
        {
            float f_val = map_float((float) pitch_bend , 0.0f, 127.0f, rule->from_, rule->to_);
            push_command(CommandPtr(new SetFloatCommand(rule->args_, f_val)));
        }
        else
        {
            g_critical("Pitch bend MIDI rules only support set_float command. Found %s", rule->action_.c_str());
        }
        return true; // it found a rule, even if invalid
    }
    else
        return false;
}

/**
 * Callback for incoming MIDI messages.  Called in its thread.
 *
 * We try to get the first MidiRule found for the given MIDI event.
 * (given its pitch, and if on or off)
 *  - NOTE_OFF_RULE: [s:action s:args]
 *  - NOTE_ON_RULE: [s:action s:args]
 *  - CONTROL_MAP_RULE [s:action i:value]
 *  - CONTROL_ON_RULE [s:action s:args]
 *  - CONTROL_OFF_RULE [s:action s:args]
 *  - MIDIPROGRAMCHANGE [s:action i:number]
 */
void MidiInput::input_message_cb(double /* delta_time */, std::vector< unsigned char > *message, void *user_data )
{
    MidiInput* context = static_cast<MidiInput*>(user_data);
    if (context->verbose_)
    {
        std::cout << "MIDI message: (";
        for (unsigned int i = 0; i < message->size(); i++)
            std::cout << (int)message->at(i) << " ";
        std::cout << ")" << std::endl;
    }
    if (message->size() <= 1)
        return; // Don't support messages with only one byte or less.
    unsigned char message_type = get_midi_event_type(message->at(0));
    switch (message_type)
    {
        case MIDINOTEON:
        {
            if (context->verbose_)
                std::cout << "MIDINOTEON" << std::endl;
            if (message->size() < 3)
            {
                g_critical("Note on message should have 4 params");
                return;
            }
            int note_pitch = int(message->at(1));
            if (message->at(2) == 0x00) // if velocity is 0, it's actually a note off message
            {
                if (context->find_rule_for_note_off(note_pitch))
                    return;
            } else {
                if (context->find_rule_for_note_on(note_pitch))
                    return;
            }
            break;
        }
        case MIDINOTEOFF:
        {
            if (context->verbose_)
                std::cout << "MIDINOTEOFF" << std::endl;
            int note_pitch = int(message->at(1));
            if (context->find_rule_for_note_off(note_pitch))
                return;
            break;
        }
        case MIDICONTROLCHANGE:
        {
            int controller_number = int(message->at(1));
            int control_value = int(message->at(2));
            if (control_value == 0)
            {
                if (context->find_rule_for_control_off(controller_number))
                    return;
            } else {
                if (context->find_rule_for_control_on(controller_number))
                    return;
                if (context->find_rule_for_control_map(controller_number, control_value))
                    return;
            }
            break;
        }
        case MIDIPROGRAMCHANGE:
        {
            int program_number = int(message->at(0) & 0x0f);
            if (context->find_rule_for_program_change(program_number))
                return;
            break;
        }
        case MIDIPITCHBEND:
        {
            int val(message->at(2));
            if (context->find_rule_for_pitch_wheel(val))
                return;
            break;
        }
        default:
            return;
            break;
    }
}
#if 0
/**
 * Maps action name to a message enum.
 */
Message MidiInput::make_message(const std::string &action)
{
    // TODO: use some map lookup, not else if
    if (action == "add_image")
        return Message(Message::ADD_IMAGE);
    else if (action == "set_float")
        return Message(Message::SET_FLOAT);
    else if (action == "set_int")
        return Message(Message::SET_INT);
    else if (action == "remove_image")
        return Message(Message::REMOVE_IMAGE);
    else if (action == "video_record_on")
        return Message(Message::VIDEO_RECORD_ON);
    else if (action == "video_record_off")
        return Message(Message::VIDEO_RECORD_OFF);
    else if (action == "select_clip")
        return Message(Message::SELECT_CLIP);
    else if (action == "quit")
        return Message(Message::QUIT);
    else
    {
        g_critical("%s: Unknown action name: %s\n", __FUNCTION__, action.c_str());
        return Message(Message::NOP);
    }
}
#endif

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

void MidiInput::push_command(CommandPtr command)
{
    if (verbose_)
        std::cout << __FUNCTION__ << "(" << command->get_name() << ")" << std::endl;
    messaging_queue_.push(command);
}

/**
 * Takes action!
 *
 * Should be called when it's time to take action, before rendering a frame, for example.
 */
void MidiInput::consume_commands()
{
    // TODO:2010-10-03:aalex:Move this handling to Application.
    bool success = true;
    while (success)
    {
        CommandPtr command;
        success = messaging_queue_.try_pop(command);
        if (success)
        {
            if (verbose_)
                std::cout << __FUNCTION__ << ": apply " << command->get_name() << std::endl;
            command->apply(*(owner_->get_controller()));
        }
    }
}

MidiInput::MidiInput(Application* owner, bool verbose) :
        verbose_(verbose),
        owner_(owner),
        messaging_queue_(),
        port_(0),
        ports_count_(0),
        opened_(false),
        midi_binder_(verbose)
{
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
    midi_binder_.set_verbose(verbose);
}

MidiInput::~MidiInput()
{
    delete midi_in_;
}

